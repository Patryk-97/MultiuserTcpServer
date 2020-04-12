#pragma once

#ifndef __SERVER_SOCKET_H__
#define __SERVER_SOCKET_H__

#include "Socket.h"
#include "ClientSocket.h"

class ServerSocket : public Socket
{
public:
   ServerSocket();
   ~ServerSocket();
   bool bind(const char* address, const uint16_t port);
   bool listen(const int backlog);
   ClientSocket* accept(void) const;
   uint16_t getLocalPort(void) const;
};

#endif //__SERVER_SOCKET_H__