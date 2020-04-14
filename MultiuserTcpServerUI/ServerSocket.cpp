#include "ServerSocket.h"

ServerSocket::ServerSocket() : Socket()
{

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
   CString wparam, lparam, clientLocalAddressIp;

   while (true)
   {
      wparam.Format(L"Waiting for new client ...");
      lparam.Format(L"");
      currentDialog->SendMessage(WM_YOUR_MESSAGE, (WPARAM)&wparam, (LPARAM)&lparam);

      ClientSocket* clientSocket = this->accept();
      if (clientSocket != nullptr)
      {
         clientLocalAddressIp = clientSocket->getLocalAddressIp().c_str();
         wparam.Format(L"New client [%s, ", clientLocalAddressIp);
         lparam.Format(L"%u] connected with Server", (unsigned)clientSocket->getLocalPort());
         currentDialog->SendMessage(WM_YOUR_MESSAGE, (WPARAM)&wparam, (LPARAM)&lparam);

         Socket* sockets[] = { this, clientSocket };
         auto pair = std::make_pair((Socket**)sockets, (CDialog*)currentDialog);

         std::unique_ptr<WinapiThreadAdaptor> clientThread =
            std::make_unique<WinapiThreadAdaptor>(ServerSocket::clientThread, &pair);
      }
      else
      {
         wparam.Format(L"Error occurred when new client tried to connect with server");
         lparam.Format(L"");
         currentDialog->SendMessage(WM_YOUR_MESSAGE, (WPARAM)&wparam, (LPARAM)&lparam);
      }
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
   CString wparam, lparam, clientLocalAddressIp, receiveBuffer, errorMessage;

   do
   {
      if ((bytesReceived = clientSocket->recv(recvBuff, RECV_BUFF_SIZE)) > 0)
      {
         clientLocalAddressIp = clientSocket->getLocalAddressIp().c_str();
         receiveBuffer = recvBuff;
         receiveBuffer.AppendChar('\0');

         wparam.Format(L"Message (%d bytes) from client [%s, ",
            bytesReceived, clientLocalAddressIp);
         lparam.Format(L"%u ]: %s\r\n", (unsigned)clientSocket->getLocalPort(), receiveBuffer);
         currentDialog->SendMessage(WM_YOUR_MESSAGE, (WPARAM)&wparam, (LPARAM)&lparam);

         if (true == clientSocket->send(recvBuff, bytesReceived))
         {
            wparam.Format(L"Reply message to client: %s", receiveBuffer);
            lparam.Format(L"");
            currentDialog->SendMessage(WM_YOUR_MESSAGE, (WPARAM)&wparam, (LPARAM)&lparam);
         }
         else
         {
            errorMessage = WinsockManager::getErrorMessage().c_str();

            wparam.Format(L"Reply message has not sent\r\n");
            lparam.Format(L"Error: %s", errorMessage);
            currentDialog->SendMessage(WM_YOUR_MESSAGE, (WPARAM)&wparam, (LPARAM)&lparam);
            clientSocket->close();
            break;
         }
      }
      else if (bytesReceived == 0)
      {
         clientLocalAddressIp = clientSocket->getLocalAddressIp().c_str();

         wparam.Format(L"Client [%s, ", clientLocalAddressIp);
         lparam.Format(L"%u] disconnected", (unsigned)clientSocket->getLocalPort());
         currentDialog->SendMessage(WM_YOUR_MESSAGE, (WPARAM)&wparam, (LPARAM)&lparam);
         clientSocket->close();
      }
      else
      {
         errorMessage = WinsockManager::getErrorMessage().c_str();

         wparam.Format(L"Error occured while server was waiting for message from client\n");
         lparam.Format(L"Error: %s", errorMessage);
         currentDialog->SendMessage(WM_YOUR_MESSAGE, (WPARAM)&wparam, (LPARAM)&lparam);
         clientSocket->close();
      }

   } while (bytesReceived > 0);
   delete clientSocket;
}