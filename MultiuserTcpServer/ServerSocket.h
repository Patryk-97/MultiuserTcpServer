#pragma once

#ifndef __SERVER_SOCKET_H__
#define __SERVER_SOCKET_H__

#define RECV_BUFF_SIZE 4096

#include "Socket.h"
#include "ClientSocket.h"
#include "WinapiThreadAdaptor.h"
#include "WinsockManager.h"

#include <iostream>

class ServerSocket : public Socket
{
public:
   ServerSocket();
   ~ServerSocket();
   bool bind(const char* address, const uint16_t port);
   bool listen(const int backlog);
   ClientSocket* accept(void) const;
   uint16_t getLocalPort(void) const;
   static uint32_t listenThread(void* arg);
   void listenThreadImpl(void);
   static uint32_t clientThread(void* arg);
   void clientThreadImpl(ClientSocket* clientSocket);
};

#endif //__SERVER_SOCKET_H__