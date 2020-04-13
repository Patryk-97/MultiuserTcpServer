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
   ServerSocket* serverSocket = (ServerSocket*)arg;
   serverSocket->listenThreadImpl();
   return 0;
}

void ServerSocket::listenThreadImpl(void)
{
   while (true)
   {
      std::cout << "Waiting for new client ...\n";
      ClientSocket* clientSocket = this->accept();
      if (clientSocket != nullptr)
      {
         std::cout << "New client [" << clientSocket->getLocalAddressIp() << ", ";
         std::cout << clientSocket->getPort() << "] connected with Server\n\n";
         Socket* sockets[] = { this, clientSocket };
         std::unique_ptr<WinapiThreadAdaptor> clientThread =
            std::make_unique<WinapiThreadAdaptor>(ServerSocket::clientThread, sockets);
      }
      else
      {
         std::cout << "Error occurred when new client tried to connect with server\n";
      }
   }
}

uint32_t ServerSocket::clientThread(void* arg)
{
   Socket** sockets = (Socket**)arg;
   ServerSocket* serverSocket = dynamic_cast<ServerSocket*>(sockets[0]);
   ClientSocket* clientSocket = dynamic_cast<ClientSocket*>(sockets[1]);
   serverSocket->clientThreadImpl(clientSocket);
   return 0;
}

void ServerSocket::clientThreadImpl(ClientSocket* clientSocket)
{
   // locals
   int bytesReceived = 0;
   char recvBuff[RECV_BUFF_SIZE];

   do
   {
      if ((bytesReceived = clientSocket->recv(recvBuff, RECV_BUFF_SIZE)) > 0)
      {
         std::cout << "Message (" << bytesReceived << " bytes) from client [";
         std::cout << clientSocket->getLocalAddressIp() << ", " << clientSocket->getLocalPort() << "]: ";
         std::cout << recvBuff << "\n";
         if (true == clientSocket->send(recvBuff, bytesReceived))
         {
            std::cout << "Reply message to client: " << recvBuff << "\n\n";
         }
         else
         {
            std::cout << "Reply message has not sent\n";
            std::cout << "Error: " << WinsockManager::getErrorMessage() << "\n";
            clientSocket->close();
            break;
         }
      }
      else if (bytesReceived == 0)
      {
         std::cout << "Client [" << clientSocket->getLocalAddressIp() << ", ";
         std::cout << clientSocket->getLocalPort() << "] disconnected\n";
         clientSocket->close();
      }
      else
      {
         std::cout << "Error occured while server was waiting for message from client\n" << "\n";
         std::cout << "Error: " << WinsockManager::getErrorMessage() << "\n";
         clientSocket->close();
      }

   } while (bytesReceived > 0);
   delete clientSocket;
}