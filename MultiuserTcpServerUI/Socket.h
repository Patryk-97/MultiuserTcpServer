#pragma once

#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <Ws2tcpip.h>
#include <string>
#include <memory>
#include "IpProtocol.h"
#include "TxProtocol.h"

class Socket
{
public:
   Socket();
   virtual ~Socket();
   void reset(void);
   bool init(IpProtocol ipProtocol, TxProtocol txProtocol);
   bool send(const std::string& sendBuff, int& bytesSend);
   int recv(char* recvBuff, int recvBuffSize);
   void close();

   std::string getIpProtocolStr(void) const;
   IpProtocol getIpProtocol(void) const;
   std::string getTxProtocolStr(void) const;
   TxProtocol getTxProtocol(void) const;

protected:

   // protected methods
   void fillAddrInfoCriteria(addrinfo* hints) const;
   bool fillNetworkAddressStructure(const char* address);
   void fillPort(uint16_t port);
   void fillIpProtocolFamily(void);
   static std::string convertAddressIpToStr(const sockaddr_in* socketAddr);
   static uint16_t convertPortFromNetworkEndianness(const sockaddr_in* socketAddr);

protected:

   // attributes
   SOCKET socketId;
   IpProtocol ipProtocol;
   TxProtocol txProtocol;
   std::unique_ptr<sockaddr_in> socketAddr;
};


#endif // __SOCKET_H__
