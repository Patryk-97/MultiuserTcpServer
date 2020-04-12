#include "ClientSocket.h"

ClientSocket::ClientSocket() : Socket()
{
   this->localSocketAddr = std::make_unique<sockaddr_in>();
   this->localPort = 0;
   this->port = 0;
}

ClientSocket::ClientSocket(SOCKET socketId) : ClientSocket()
{
   this->socketId = socketId;
}

ClientSocket::~ClientSocket()
{

}

void ClientSocket::reset(void)
{
   Socket::reset();
   this->socketAddr.reset(new sockaddr_in());
   this->localPort = 0;
   this->port = 0;
   this->serverAddressIp = "";
   this->localAddressIp = "";
}

bool ClientSocket::connect(const char* address, const uint16_t port)
{
   // locals
   bool rV = true;
   int sockAddrSize = sizeof(*this->localSocketAddr);

   this->fillIpProtocolFamily();
   this->fillPort(port);
   rV = this->fillNetworkAddressStructure(address);

   if (true == rV &&
      ::connect(this->socketId, (sockaddr*)this->socketAddr.get(), sizeof(*this->socketAddr)) == SOCKET_ERROR)
   {
      rV = false;
   }

   if (true == rV &&
      ::getsockname(this->socketId, (sockaddr*)this->localSocketAddr.get(), &sockAddrSize) == SOCKET_ERROR)
   {
      rV = false;
   }

   if (true == rV)
   {
      this->port = port;
      this->serverAddressIp = Socket::convertAddressIpToStr(this->socketAddr.get());
      this->localPort = Socket::convertPortFromNetworkEndianness(this->localSocketAddr.get());
      this->localAddressIp = Socket::convertAddressIpToStr(this->localSocketAddr.get());
   }

   return rV;
}

void ClientSocket::setLocalPort(uint16_t localPort)
{
   this->localPort = localPort;
}

uint16_t ClientSocket::getLocalPort(void) const
{
   return this->localPort;
}

void ClientSocket::setPort(uint16_t port)
{
   this->port = port;
}

uint16_t ClientSocket::getPort(void) const
{
   return this->port;
}

void ClientSocket::setLocalAddressIp(const char* localAddressIp)
{
   this->localAddressIp = localAddressIp;
}

std::string ClientSocket::getLocalAddressIp(void) const
{
   return this->localAddressIp;
}

void ClientSocket::setServerAddressIp(const char* serverAddressIp)
{
   this->serverAddressIp = serverAddressIp;
}

std::string ClientSocket::getServerAddressIp(void) const
{
   return this->serverAddressIp;
}