//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// 
//  ___________ __  .__                       ___________
//  \_   _____//  |_|  |__ _____    ____      \_   _____/
//   |    __)_\   __\  |  \\__  \  /    \      |    __)_ 
//   |        \|  | |   Y  \/ __ \|   |  \     |        \
//  /_______  /|__| |___|  (____  /___|  / /\ /_______  /
//          \/           \/     \/     \/  \/         \/ 
// 
// Universal Socket Header File
// 07-05-2024
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

#pragma once

#include <iostream>
#include <string>
#include <WinSock2.h>

namespace Socket_Vars
{
   
   enum Protocol : uint8_t
   {
      TCP   = 1,
      UDP   = 2
   };

   enum Connection_Type : uint8_t
   {
      CLIENT  = 1,
      SERVER  = 2
   };

}	// END namespace Socket_Vars

class Universal_Socket
{
public:

   //+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   /// @brief   Constructor
   //+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   Universal_Socket(
      Socket_Vars::Protocol protocol_type,
      Socket_Vars::Connection_Type connection_type,
      std::string ip_address,
      uint16_t port);

   //+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   /// @brief   Destructor
   //+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   ~Universal_Socket();

   //+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   /// @brief   Opens the socket ready to receive and or send
   /// @return  bool       Result of the opening of the socket
   //+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   bool Start();

   //+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   /// @brief   Sends a message over the socket
   /// @param   unsigned char*    Buffer containing the message
   /// @return  bool              Result of the send
   //+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   bool Send(unsigned char* buffer);

   //+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   /// @brief   Opens the socket ready to receive and or send
   /// @param   unsigned char*    Buffer where message will be put in
   /// @return  bool              Result of the receive
   //+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   bool Receive(unsigned char* buffer);

   //+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   /// @brief   Reconnects the socket
   /// @return  bool              Result of the reconnection
   //+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   bool Reconnect();

   //+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   /// @brief   Closes the socket
   /// @return  bool       Result of the closing of the socket
   //+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   bool Stop();

private:

   SOCKET _socket;
   SOCKET _listen_socket;
   SOCKADDR_IN _address;
   Socket_Vars::Protocol _protocol;
   Socket_Vars::Connection_Type _connection;
   std::string _ip_address;
   uint16_t _port;

   //+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   /// @brief   Opens the socket as a TCP Server
   /// @return  bool       Result of the opening of the socket
   //+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   bool TCP_Server_Start();

   //+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   /// @brief   Opens the socket as a TCP Client
   /// @return  bool       Result of the opening of the socket
   //+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   bool TCP_Client_Start();

};	// END class Universal_Socket