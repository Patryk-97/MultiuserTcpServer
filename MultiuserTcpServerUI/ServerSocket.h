#pragma once

#ifndef __SERVER_SOCKET_H__
#define __SERVER_SOCKET_H__

#define RECV_BUFF_SIZE 4096

#ifndef WM_USER
#define WM_USER 0x0400
#endif

#define WM_MESSAGE (WM_USER + 10)

#include <afxwin.h>

#include "Socket.h"
#include "ClientSocket.h"
#include "WinapiThreadAdaptor.h"
#include "WinsockManager.h"
#include "WinapiMutex.h"

#include <atomic>
#include <memory>

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
   void listenThreadImpl(CDialog* currentDialog);
   static uint32_t clientThread(void* arg);
   void clientThreadImpl(ClientSocket* clientSocket, CDialog* currentDialog);

   std::atomic<bool> isStopped;
   std::unique_ptr<WinapiMutex> winapiMutex;
};

#endif //__SERVER_SOCKET_H__