#include <iostream>
#include <string>
#include <memory>
#include "ServerSocket.h"
#include "WinsockManager.h"

#define RECV_BUFF_SIZE 4096
#define DLL_WINSOCK_VERSION MAKEWORD(2, 2)
#define MAX_SOCKETS_CONNECTION 1

int main()
{
   std::unique_ptr<WinsockManager> winsockManager = std::make_unique<WinsockManager>();
   std::unique_ptr<ServerSocket> serverSocket = nullptr;
   ClientSocket* clientSocket = nullptr;
   uint16_t port = 7;
   char recvBuff[RECV_BUFF_SIZE];
   int bytesReceived = 0;
   char decision = 'Y';

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
      std::cout << "Error: " << winsockManager->getErrorMessage() << "\n";
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
      std::cout << "Error: " << winsockManager->getErrorMessage() << "\n";
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
      std::cout << "Error: " << winsockManager->getErrorMessage() << "\n";
      serverSocket->close();
      std::cout << "Server socket closed" << "\n";
      winsockManager->cleanup();
      return -1;
   }

   do
   {
      std::cout << "Waiting for new client ...\n";
      clientSocket = serverSocket->accept();

      if (clientSocket != nullptr)
      {
         std::cout << "New client [" << clientSocket->getLocalAddressIp() << ", ";
         std::cout << clientSocket->getPort() << "] connected with Server\n\n";
      }
      else
      {
         std::cout << "Error occurred when new client tried to connect with server\n";
         continue;
      }

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
               std::cout << "Error: " << winsockManager->getErrorMessage() << "\n";
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
            std::cout << "Error: " << winsockManager->getErrorMessage() << "\n";
            clientSocket->close();
         }

      } while (bytesReceived > 0);

      std::cout << "Waiting for another client or stop listening? Y/N: ";
      std::cin >> decision;

   } while (decision == 'y' || decision == 'Y');

   serverSocket->close();
   std::cout << "Server socket closed\n";
   winsockManager->cleanup();
   return 0;
}