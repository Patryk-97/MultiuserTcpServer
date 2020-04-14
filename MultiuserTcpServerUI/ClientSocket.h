#pragma once

#ifndef __CLIENT_SOCKET_H__
#define __CLIENT_SOCKET_H__

#include "Socket.h"

class ClientSocket : public Socket
{
public:
   ClientSocket();
   ClientSocket(SOCKET clientSocket);
   ~ClientSocket();
   void reset(void);
   bool connect(const char* address, const uint16_t port);

   void setLocalPort(uint16_t localPort);
   uint16_t getLocalPort(void) const;
   void setPort(uint16_t port);
   uint16_t getPort(void) const;
   void setLocalAddressIp(const char* localAddressIp);
   std::string getLocalAddressIp(void) const;
   void setServerAddressIp(const char* serverAddressIp);
   std::string getServerAddressIp(void) const;

private:
   uint16_t localPort;
   uint16_t port;
   std::string localAddressIp;
   std::string serverAddressIp;
   std::unique_ptr<sockaddr_in> localSocketAddr;
};

#endif //__CLIENT_SOCKET_H__