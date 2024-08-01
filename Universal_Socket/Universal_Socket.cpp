//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// 
//  ___________ __  .__                       ___________
//  \_   _____//  |_|  |__ _____    ____      \_   _____/
//   |    __)_\   __\  |  \\__  \  /    \      |    __)_ 
//   |        \|  | |   Y  \/ __ \|   |  \     |        \
//  /_______  /|__| |___|  (____  /___|  / /\ /_______  /
//          \/           \/     \/     \/  \/         \/ 
// 
// Universal Socket Implementation File
// 07-05-2024
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")

#include "Universal_Socket.h"

#include <WS2tcpip.h>

//+-+-+-+-+-+-+-+-+-+-+PRIVATE FUNCTIONS+-+-+-+-+-+-+-+-+-+-+-+-+-+

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+    
/// Opens the socket as a TCP Server
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
bool Universal_Socket::TCP_Server_Start()
{
   bool result = true;
   int func_result = 0;
   int opt_val = 0;

   //
   // 1. Create the listen socket for TCP
   _listen_socket = socket(
      AF_INET,     // address family = IPV4
      SOCK_STREAM, // type = TCP
      IPPROTO_TCP  // protocol = TCP
   );
   if (SOCKET_ERROR == _listen_socket)
   {
      printf("ERROR, Server socket() failed with: %u\n", WSAGetLastError());
      result &= false;
      return result;
   }

   //
   // 2. setsockopt for listen_socket with KEEPALIVE
   setsockopt(_listen_socket, SOL_SOCKET, SO_KEEPALIVE, (char*)&opt_val, sizeof(int));

   //
   // 3. Initialize the SOCKADDR_IN
   _address.sin_family = AF_INET;
   _address.sin_addr.s_addr = inet_addr(_ip_address.c_str());
   _address.sin_port = htons(_port);
 
   //
   // 4. Bind the socket to an IP address and port
   func_result = bind(_listen_socket, (SOCKADDR*)&_address, sizeof(_address));
   if (SOCKET_ERROR == func_result)
   {
      printf("ERROR, Server bind() failed with: %u\n", WSAGetLastError());
      printf("Port = %d\n", _port);
      printf("IP Address = %s\n", _ip_address.c_str());
      result &= false;
      return result;
   }

   //
   // 5. Listen for incoming connections
   func_result = listen(_listen_socket, 1);
   if (SOCKET_ERROR == func_result)
   {
      printf("ERROR, Server listen() failed with: %u\n", WSAGetLastError());
      result &= false;
      return result;
   }
   else
   {
      printf("TCP-S awaiting new connections!\n");
   }

   //
   // 6. Accept the incoming connection
   _socket = accept(_listen_socket, NULL, NULL);
   if (INVALID_SOCKET == _socket) 
   {
      printf("ERROR, Server accept() failed with: %u\n", WSAGetLastError());
      result &= false;
      return result;
   }

   //
   // 7. No longer need listen socket
   closesocket(_listen_socket);

   return result;
}  // END TCP_Server_Start()

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+    
/// Opens the socket as a TCP Client
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
bool Universal_Socket::TCP_Client_Start()
{
   bool result = true;
   int func_result = 0;
   //
   // 1. Attempt connection with the server
   _socket = socket(
      AF_INET,       // address family = IPV4
      SOCK_STREAM,   // type = TCP
      IPPROTO_TCP);  // protocol = TCP
   if (INVALID_SOCKET == _socket)
   {
      printf("ERROR, Client socket() failed with: %u\n", WSAGetLastError());
      result &= false;
      return result;
   }

   //
   // 2. Initialize the SOCKADDR_IN
   _address.sin_family = AF_INET;
   _address.sin_addr.s_addr = inet_addr(_ip_address.c_str());
   _address.sin_port = htons(_port);

   //
   // 3. Make a connection with the server
   func_result = connect(_socket, (SOCKADDR*) &_address, sizeof(_address));
   if (0 != func_result)
   {
      printf("ERROR, Client connect() failed with: %u\n", WSAGetLastError());
      result &= false;
      return result;
   }
   else
   {
      printf("Client: Ready for sending and/or receiving messages...\n");
      result &= true;
   }

   return result;
}  // END TCP_Client_Start()

//-+-+-+-+-+-+-+-+-+-+-+PUBLIC FUNCTIONS+-+-+-+-+-+-+-+-+-+-+-+-+-+

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
/// Constructor
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
Universal_Socket::Universal_Socket
(
   Socket_Vars::Protocol protocol_type,
   Socket_Vars::Connection_Type connection_type,
   std::string ip_address,
   uint16_t port
)
{
   //Initialize private variables
   _protocol = protocol_type;
   _connection = connection_type;
   _ip_address = ip_address;
   _port = port;
   _socket = INVALID_SOCKET;
   _listen_socket = INVALID_SOCKET;
   memset(&_address, 0, sizeof(SOCKADDR_IN));

   // function level variables
   int func_result = 0;
   WORD wVersionRequested = MAKEWORD(2, 0);  // Request Winsock v2
   WSADATA wsa_data;

   //
   // 1. Retrieve details of Windows Sockets implementation
   func_result = WSAStartup(wVersionRequested, &wsa_data);
   if (0 != func_result)
   {
      printf("ERROR loading WinSock v2!\n");
   }

}  // End Constructor

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+    
/// Opens the socket ready to receive and or send
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
bool Universal_Socket::Start()
{
   bool result = true;

   //
   // 1. Start the socket for the specified protocol and connection type
   if (Socket_Vars::TCP == _protocol)
   {
      if (Socket_Vars::SERVER == _connection)
      {
         result &= TCP_Server_Start();
      }
      else if (Socket_Vars::CLIENT == _connection)
      {
         result &= TCP_Client_Start();
      }
      else
      {
         printf("Invalid connection type specified for TCP Socket!\n");
         result &= false;
      }
   }
   else if (Socket_Vars::UDP == _protocol)
   {
      if (Socket_Vars::SERVER == _connection)
      {
         //result &= UDP_Server_Start();
      }
      else if (Socket_Vars::CLIENT == _connection)
      {
         //result &= UDP_Client_Start();
      }
      else
      {
         printf("Invalid connection type specified for UDP Socket!\n");
         result &= false;
      }
   }
   else
   {
      printf("Invalid protocol type specified for Socket!\n");
      result &= false;
   }

   return result;
}  // END Start()

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
/// Opens the socket ready to receive and or send
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
bool Receive(unsigned char* buffer)
{
   bool result = true;



   return result;
}

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+    
/// Closes the socket
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
bool Universal_Socket::Stop()
{
   bool result = true;
   int func_result;

   //
   // 1. Shutdown the connection
   func_result = shutdown(_socket, SD_SEND);
   if (SOCKET_ERROR == func_result) {
      printf("shutdown() failed with: %u\n", WSAGetLastError());
      result &= false;
   }

   //
   // 2. Cleanup
   closesocket(_socket);
   WSACleanup();

   return result;
}  // END Stop()