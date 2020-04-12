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