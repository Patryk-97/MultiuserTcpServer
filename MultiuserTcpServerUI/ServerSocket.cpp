#include "ServerSocket.h"

ServerSocket::ServerSocket() : Socket()
{
   this->isStopped = false;
   this->winapiMutex = std::make_unique<WinapiMutex>();
   this->socketConnections = 0;
}

ServerSocket::~ServerSocket()
{

}

bool ServerSocket::bind(const char* address, const uint16_t port)
{
   // locals
   bool rV = true;

   this->fillIpProtocolFamily();
   this->fillPort(port);
   rV = this->fillNetworkAddressStructure(address);

   if (true == rV &&
      ::bind(this->socketId, (sockaddr*)this->socketAddr.get(), sizeof(*this->socketAddr)) == SOCKET_ERROR)
   {
      rV = false;
   }

   return rV;
}

bool ServerSocket::listen(const int backlog)
{
   // locals
   bool rV = true;

   if (::listen(this->socketId, backlog) == SOCKET_ERROR)
   {
      rV = false;
   }

   return rV;
}

ClientSocket* ServerSocket::accept(void) const
{
   // locals
   ClientSocket* clientSocket = nullptr;
   std::unique_ptr<sockaddr_in> remoteClientSockAddr = std::make_unique<sockaddr_in>();
   std::unique_ptr<sockaddr_in> localClientSockAddr = std::make_unique<sockaddr_in>();
   int remoteSize = sizeof(*remoteClientSockAddr), sockAddrSize = sizeof(*localClientSockAddr);
   SOCKET clientSocketId = ::accept(this->socketId, (sockaddr*)remoteClientSockAddr.get(), &remoteSize);

   if (clientSocketId != INVALID_SOCKET &&
      ::getsockname(clientSocketId, (sockaddr*)localClientSockAddr.get(), &sockAddrSize) == 0)
   {
      clientSocket = new ClientSocket(clientSocketId);

      clientSocket->setLocalAddressIp(Socket::convertAddressIpToStr(remoteClientSockAddr.get()).c_str());
      clientSocket->setLocalPort(Socket::convertPortFromNetworkEndianness(remoteClientSockAddr.get()));
      clientSocket->setServerAddressIp(Socket::convertAddressIpToStr(localClientSockAddr.get()).c_str());
      clientSocket->setPort(Socket::convertPortFromNetworkEndianness(localClientSockAddr.get()));
   }

   return clientSocket;
}

uint16_t ServerSocket::getLocalPort(void) const
{
   return Socket::convertPortFromNetworkEndianness(this->socketAddr.get());
}

uint32_t ServerSocket::listenThread(void* arg)
{
   std::pair<ServerSocket*, CDialog*>* pair =
      (std::pair<ServerSocket*, CDialog*>*)arg;
   ServerSocket* serverSocket = pair->first;
   CDialog* currentDialog = pair->second;
   serverSocket->listenThreadImpl(currentDialog);
   return 0;
}

void ServerSocket::listenThreadImpl(CDialog* currentDialog)
{
   // locals
   uint32_t messageType;
   CString lparam, clientLocalAddressIp;
   int send;

   while (true)
   {
      messageType = PRINT_LOG;
      lparam.Format(L"Waiting for new client ...");
      currentDialog->SendMessage(WM_MESSAGE, (WPARAM)messageType, (LPARAM)&lparam);

      ClientSocket* clientSocket = this->accept();
      ++this->socketConnections;
      if (clientSocket != nullptr)
      {
         if (this->socketConnections > MAX_SOCKETS_CONNECTION)
         {
            clientSocket->send("Too many sockets connected to server!", send);
            clientSocket->close();
            delete clientSocket;
         }
         else
         {
            currentDialog->SendMessage(WM_MESSAGE, (WPARAM)INCREMENT_CLIENTS_COUNT, NULL);

            clientLocalAddressIp = clientSocket->getLocalAddressIp().c_str();

            messageType = PRINT_LOG;
            lparam.Format(L"New client [%s, %u] connected with Server\r\n",
               clientLocalAddressIp, (unsigned)clientSocket->getLocalPort());
            currentDialog->SendMessage(WM_MESSAGE, (WPARAM)messageType, (LPARAM)&lparam);

            Socket* sockets[] = { this, clientSocket };
            auto pair = std::make_pair((Socket**)sockets, (CDialog*)currentDialog);

            std::unique_ptr<WinapiThreadAdaptor> clientThread =
               std::make_unique<WinapiThreadAdaptor>(ServerSocket::clientThread, &pair);
         }
      }
      else
      {
         this->winapiMutex->lock();
         if (isStopped)
         {
            break;
         }
         this->winapiMutex->unlock();

         messageType = PRINT_LOG;
         lparam.Format(L"Error occurred when new client tried to connect with server");
         currentDialog->SendMessage(WM_MESSAGE, (WPARAM)messageType, (LPARAM)&lparam);
      }

      this->winapiMutex->lock();
      if (isStopped)
      {
         break;
      }
      this->winapiMutex->unlock();
   }
}

uint32_t ServerSocket::clientThread(void* arg)
{
   std::pair<Socket**, CDialog*>* pair = (std::pair<Socket**, CDialog*>*)arg;
   Socket** sockets = (Socket**)pair->first;
   ServerSocket* serverSocket = dynamic_cast<ServerSocket*>(sockets[0]);
   ClientSocket* clientSocket = dynamic_cast<ClientSocket*>(sockets[1]);
   CDialog* currentDialog = pair->second;
   serverSocket->clientThreadImpl(clientSocket, currentDialog);
   return 0;
}

void ServerSocket::clientThreadImpl(ClientSocket* clientSocket, CDialog* currentDialog)
{
   // locals
   int bytesReceived = 0;
   char recvBuff[RECV_BUFF_SIZE];
   uint32_t messageType;
   CString lparam, clientLocalAddressIp, receiveBuffer, errorMessage;

   do
   {
      if ((bytesReceived = clientSocket->recv(recvBuff, RECV_BUFF_SIZE)) > 0)
      {
         clientLocalAddressIp = clientSocket->getLocalAddressIp().c_str();
         receiveBuffer = recvBuff;
         receiveBuffer.AppendChar('\0');

         messageType = PRINT_LOG;
         lparam.Format(L"Message (%d bytes) from client [%s, %u ]: %s\r\n",
            bytesReceived, clientLocalAddressIp, (unsigned)clientSocket->getLocalPort(),
            receiveBuffer);
         currentDialog->SendMessage(WM_MESSAGE, (WPARAM)messageType, (LPARAM)&lparam);

         if (true == clientSocket->send(recvBuff, bytesReceived))
         {
            messageType = PRINT_LOG;
            lparam.Format(L"Reply message to client: %s", receiveBuffer);
            currentDialog->SendMessage(WM_MESSAGE, (WPARAM)messageType, (LPARAM)&lparam);
         }
         else
         {
            errorMessage = WinsockManager::getErrorMessage().c_str();

            messageType = PRINT_LOG;
            lparam.Format(L"Reply message has not sent\r\nError: %s", errorMessage);
            currentDialog->SendMessage(WM_MESSAGE, (WPARAM)messageType, (LPARAM)&lparam);
            clientSocket->close();
            break;
         }
      }
      else if (bytesReceived == 0)
      {
         clientLocalAddressIp = clientSocket->getLocalAddressIp().c_str();

         messageType = PRINT_LOG;
         lparam.Format(L"Client [%s, %u] disconnected\r\n", clientLocalAddressIp,
            (unsigned)clientSocket->getLocalPort());
         --this->socketConnections;
         currentDialog->SendMessage(WM_MESSAGE, (WPARAM)DECREMENT_CLIENTS_COUNT, NULL);
         currentDialog->SendMessage(WM_MESSAGE, (WPARAM)messageType, (LPARAM)&lparam);
         clientSocket->close();
      }
      else
      {
         errorMessage = WinsockManager::getErrorMessage().c_str();

         messageType = PRINT_LOG;
         lparam.Format(
            L"Error occured while server was waiting for message from client\r\nError: %s",
            errorMessage);
         currentDialog->SendMessage(WM_MESSAGE, (WPARAM)messageType, (LPARAM)&lparam);
         clientSocket->close();
      }

      this->winapiMutex->lock();
      if (isStopped)
      {
         delete clientSocket; break;
      }
      this->winapiMutex->unlock();

   } while (bytesReceived > 0);
}