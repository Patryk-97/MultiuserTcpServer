#include "Socket.h"

Socket::Socket()
{
   this->socketId = INVALID_SOCKET;
   this->ipProtocol = IpProtocol::IPV4;
   this->txProtocol = TxProtocol::TCP;
   this->socketAddr = std::make_unique<sockaddr_in>();
}

Socket::~Socket()
{
   this->reset();
}

void Socket::reset(void)
{
   if (this->socketId != INVALID_SOCKET)
   {
      closesocket(this->socketId);
   }
   this->ipProtocol = IpProtocol::IPV4;
   this->txProtocol = TxProtocol::TCP;
   this->socketAddr.reset(new sockaddr_in());
   this->socketId = INVALID_SOCKET;
}

bool Socket::init(IpProtocol ipProtocol, TxProtocol txProtocol)
{
   // locals
   int family = AF_INET;
   int type = SOCK_STREAM;
   int proto = IPPROTO_TCP;
   bool rV = true;

   if (ipProtocol == IpProtocol::IPV6)
   {
      family = AF_INET6;
      this->ipProtocol = IpProtocol::IPV6;
   }
   if (txProtocol == TxProtocol::UDP)
   {
      type = SOCK_DGRAM;
      this->txProtocol = TxProtocol::UDP;
      proto = IPPROTO_UDP;
   }

   this->socketId = ::socket(family, type, proto);
   if (this->socketId == INVALID_SOCKET)
   {
      rV = false;
   }
   return rV;
}

bool Socket::send(const std::string& sendBuff, int& bytesSend)
{
   // locals
   bool rV = true;

   bytesSend = ::send(this->socketId, sendBuff.c_str(), sendBuff.length() + 1, 0);

   if (bytesSend == SOCKET_ERROR)
   {
      rV = false;
   }

   return rV;
}

int Socket::recv(char* recvBuff, int recvBuffSize)
{
   // locals
   int rV = 0;

   memset(recvBuff, 0, recvBuffSize);
   rV = ::recv(this->socketId, recvBuff, recvBuffSize, 0);

   return rV;
}

void Socket::close()
{
   if (this->socketId != INVALID_SOCKET)
   {
      closesocket(this->socketId);
   }
}

std::string Socket::getIpProtocolStr(void) const
{
   // locals
   std::string rV;

   rV = this->ipProtocol == IpProtocol::IPV4 ? "IPV4" : "IPV6";
   return rV;
}

IpProtocol Socket::getIpProtocol(void) const
{
   return this->ipProtocol;
}

std::string Socket::getTxProtocolStr(void) const
{
   // locals
   std::string rV;

   rV = this->txProtocol == TxProtocol::TCP ? "TCP" : "UDP";
   return rV;
}

TxProtocol Socket::getTxProtocol(void) const
{
   return this->txProtocol;
}

void Socket::fillAddrInfoCriteria(addrinfo* hints) const
{
   memset(hints, 0, sizeof(*hints));
   hints->ai_socktype = this->txProtocol == TxProtocol::TCP ? SOCK_STREAM : SOCK_DGRAM;
   hints->ai_family = this->ipProtocol == IpProtocol::IPV4 ? AF_INET : AF_INET6;
}

bool Socket::fillNetworkAddressStructure(const char* address)
{
   // locals
   bool rV = true;
   std::unique_ptr<addrinfo> hints = std::make_unique<addrinfo>();
   addrinfo* res = nullptr;

   if (address == nullptr)
   {
      this->socketAddr->sin_addr.s_addr = INADDR_ANY;
   }
   else if (address[0] >= '0' && address[0] <= '9')
   {
      if (::inet_pton(AF_INET, address, &this->socketAddr->sin_addr) != 1)
      {
         rV = false;
      }
   }
   else
   {
      this->fillAddrInfoCriteria(hints.get());
      if (0 != ::getaddrinfo(address, nullptr, hints.get(), &res))
      {
         rV = false;
      }
      else
      {
         this->socketAddr->sin_addr.S_un = ((sockaddr_in*)(res->ai_addr))->sin_addr.S_un;
         ::freeaddrinfo(res);
      }
   }

   return rV;
}

void Socket::fillPort(uint16_t port)
{
   this->socketAddr->sin_port = htons(port);
}

void Socket::fillIpProtocolFamily(void)
{
   if (this->ipProtocol == IpProtocol::IPV4)
   {
      this->socketAddr->sin_family = AF_INET;
   }
   else
   {
      this->socketAddr->sin_family = AF_INET6;
   }
}

std::string Socket::convertAddressIpToStr(const sockaddr_in* socketAddr)
{
   // locals
   std::string addressIp = "\"";

   addressIp += std::to_string(socketAddr->sin_addr.S_un.S_un_b.s_b1) + ".";
   addressIp += std::to_string(socketAddr->sin_addr.S_un.S_un_b.s_b2) + ".";
   addressIp += std::to_string(socketAddr->sin_addr.S_un.S_un_b.s_b3) + ".";
   addressIp += std::to_string(socketAddr->sin_addr.S_un.S_un_b.s_b4) + "\"";

   return addressIp;
}

uint16_t Socket::convertPortFromNetworkEndianness(const sockaddr_in* socketAddr)
{
   // locals
   uint16_t port = (socketAddr->sin_port & 0xFF00) >> 8 | (socketAddr->sin_port & 0x00FF) << 8;

   return port;
}