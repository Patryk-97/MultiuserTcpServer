#include "ServerSocket.h"
#include "WinsockManager.h"
#include "WinapiThreadAdaptor.h"

#include <iostream>
#include <string>
#include <memory>
#include <atomic>

#define DLL_WINSOCK_VERSION MAKEWORD(2, 2)
#define MAX_SOCKETS_CONNECTION 1

int main()
{
   std::unique_ptr<WinsockManager> winsockManager = std::make_unique<WinsockManager>();
   std::unique_ptr<ServerSocket> serverSocket = nullptr;
   std::unique_ptr<WinapiThreadAdaptor> listenThread = nullptr;
   uint16_t port = 7;

   if (false == winsockManager->startup(DLL_WINSOCK_VERSION))
   {
      printf("Winsock initialization error\n");
      return -1;
   }

   std::cout << "Enter port: ";
   std::cin >> port;

   if (port == 0)
   {
      std::cout << "You entered wrong port. Closing...\n";
      return -1;
   }

   serverSocket = std::make_unique<ServerSocket>();

   if (true == serverSocket->init(IpProtocol::IPV4, TxProtocol::TCP))
   {
      std::cout << "Server socket initialized\n";
   }
   else
   {
      std::cout << "Cannot initialiaze a socket\n";
      std::cout << "Error: " << WinsockManager::getErrorMessage() << "\n";
      winsockManager->cleanup();
      return -1;
   }

   if (true == serverSocket->bind(nullptr, port))
   {
      std::cout << "Server socket bound. Port: " << port << "\n";
   }
   else
   {
      std::cout << "Cannot bind socket server.\n";
      std::cout << "Error: " << WinsockManager::getErrorMessage() << "\n";
      serverSocket->close();
      std::cout << "Server socket closed" << "\n";
      winsockManager->cleanup();
      return -1;
   }

   if (true == serverSocket->listen(MAX_SOCKETS_CONNECTION))
   {
      std::cout << "Server socket started listening [max " << MAX_SOCKETS_CONNECTION << " connections]\n";
   }
   else
   {
      std::cout << "Cannot start listening socket server\n";
      std::cout << "Error: " << WinsockManager::getErrorMessage() << "\n";
      serverSocket->close();
      std::cout << "Server socket closed" << "\n";
      winsockManager->cleanup();
      return -1;
   }

   listenThread = 
      std::make_unique<WinapiThreadAdaptor>(ServerSocket::listenThread, serverSocket.get());

   listenThread->join();

   serverSocket->close();
   std::cout << "Server socket closed\n";
   winsockManager->cleanup();
   return 0;
}