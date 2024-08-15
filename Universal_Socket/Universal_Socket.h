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
// 
//  Author   Date         Description
// --------------------------------------------------------
//  ECE      08-11-2024   Initial Implementation
//  ECE      08-14-2024   Added _is_socket_connected flag
//  ECE      08-15-2024   Added getter for connection type
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

#pragma once

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <string>
#include <mutex>
#include <queue>

namespace Socket_Vars
{
   constexpr uint16_t MAX_EVENTS{ 100 };
   constexpr uint16_t MAX_SOCKETS{ 100 };

   // List of all socket event handles
   extern HANDLE socket_events[MAX_EVENTS];

   // Number of events in the socket_events list
   extern uint16_t event_count;

   // Mutexs for send and receive queues
   extern std::mutex receive_mutex[MAX_SOCKETS];
   extern std::mutex send_mutex[MAX_SOCKETS];

   // Receive and send queues for each socket,
   // if you have a message to be sent, place it on the send queue
   // if a message is received, it will be placed on the receive queue
   extern std::queue<std::string> receive_queue[MAX_SOCKETS];
   extern std::queue<std::string> send_queue[MAX_SOCKETS];

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

   //+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   /// @brief   When creating a socket, an event HANDLE must
   /// @brief   also be created and added to the socket_events
   /// @brief   list, this manages that
   /// @param   HANDLE            event to be added to socket_events
   /// @param   uint16_t          returning index where event was added
   /// @return  bool              Result of the addition
   //+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   extern bool Add_Event_To_Event_List(HANDLE& event, uint16_t& event_list_index);

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
      uint16_t port,
      std::string name);

   //+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   /// @brief   Opens the socket ready to receive and or send
   /// @return  bool       Result of the opening of the socket
   //+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   bool Start();

   //+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   /// @brief   Handles the event that was triggered on this socket
   /// @return  bool              Result of the event handle
   //+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   bool Handle_Event();

   //+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   /// @brief   Sends a message over the socket
   /// @param   unsigned char*    Buffer containing the message
   /// @return  bool              Result of the send
   //+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   bool Send(const char* buffer);

   //+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   /// @brief   Opens the socket ready to receive and or send
   /// @param   unsigned char*    Buffer where message will be put in
   /// @return  bool              Result of the receive
   //+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   bool Receive(char* &buffer);

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

   //+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   /// @brief   Fetches the current connection status
   /// @return  bool              _is_socket_connected
   //+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   inline bool Is_Socket_Connected() { return _is_socket_connected; }

   //+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   /// @brief   Gets Client or Server connection type
   /// @return  Connection_Type   _connection
   //+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   inline Socket_Vars::Connection_Type Get_Connection_Type() { return _connection; }

private:

   SOCKET _socket;
   SOCKET _listen_socket;
   SOCKADDR_IN _address;
   Socket_Vars::Protocol _protocol;
   Socket_Vars::Connection_Type _connection;
   std::string _ip_address;
   uint16_t _port;
   std::string _socket_name;
   uint16_t _event_handle_index;
   bool _is_socket_connected;

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