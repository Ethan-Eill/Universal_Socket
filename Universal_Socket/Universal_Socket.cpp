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
// 
//  Author   Date         Description
// --------------------------------------------------------
//  ECE      08-11-2024   Initial Implementation
//  ECE      08-14-2024   Added _is_socket_connected flag
//  ECE      08-15-2024   Implemented TCP Client option
//  ECE      08-16-2024   Implemented UDP option
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")

#include "Universal_Socket.h"

#include <tchar.h>

namespace Socket_Vars
{
   // List of all socket event handles
   HANDLE socket_events[MAX_EVENTS];

   // Number of events in the socket_events list
   uint16_t event_count = 0;

   // Mutexs for send and receive queues
   std::mutex receive_mutex[MAX_SOCKETS];
   std::mutex send_mutex[MAX_SOCKETS];

   // Receive and send queues for each socket,
   // if you have a message to be sent, place it on the send queue
   // if a message is received, it will be placed on the receive queue
   std::queue<std::string> receive_queue[MAX_SOCKETS];
   std::queue<std::string> send_queue[MAX_SOCKETS];

   extern bool Add_Event_To_Event_List(HANDLE& event, uint16_t& event_list_index)
   {
      bool result = true;

      //
      // 1. Check size of the event list
      if (MAX_EVENTS <= event_count)
      {
         printf("Too many sockets!\n");
         result &= false;
         return result;
      }

      //
      // 2. Add event to event list and increment size
      event_list_index = event_count;
      socket_events[event_list_index] = event;  // Might be false warning, should be ok
      event_count++;

      return result;
   }  // END Add_Event_To_Event_List()

}  // END namespace Socket_Vars

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
   _address.sin_port = htons(_port);
   // InetPton() requires IP Address as a PCWSTR
   // this is the conversion of std::string to PCWSTR
   std::wstring w_str(_ip_address.begin(), _ip_address.end());
   PCWSTR pcwstr_ip = w_str.c_str();
   InetPton(AF_INET, pcwstr_ip, &_address.sin_addr.s_addr);
 
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
      printf("TCP-Server %s awaiting new connections!\n", _socket_name.c_str());
   }

   //
   // 6. Create an event for the listen socket
   HANDLE event = WSACreateEvent();
   Socket_Vars::socket_events[_event_handle_index] = event;
   if (WSA_INVALID_EVENT == Socket_Vars::socket_events[_event_handle_index])
   {
      printf("ERROR, Server WSACreateEvent() failed with: %u\n", WSAGetLastError());
      result &= false;
      return result;
   }
   
   //
   // 7. Use 'WSAEventSelect' to associate an event with the socket
   func_result = WSAEventSelect(
      _listen_socket, 
      Socket_Vars::socket_events[_event_handle_index],
      FD_ACCEPT | FD_READ | FD_WRITE | FD_CLOSE);
   if (SOCKET_ERROR == func_result)
   {
      printf("ERROR, Server WSAEventSelect() failed with: %u\n", WSAGetLastError());
      result &= false;
      return result;
   }

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
   _address.sin_port = htons(_port);
   // InetPton() requires IP Address as a PCWSTR
   // this is the conversion of std::string to PCWSTR
   std::wstring w_str(_ip_address.begin(), _ip_address.end());
   PCWSTR pcwstr_ip = w_str.c_str();
   InetPton(AF_INET, pcwstr_ip, &_address.sin_addr.s_addr);

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
      printf("TCP Client %s: Ready for sending and/or receiving messages...\n", _socket_name.c_str());
      result &= true;
      _is_socket_connected = true;
   }

   //
   // 4. Create an event for the listen socket
   HANDLE event = WSACreateEvent();
   Socket_Vars::socket_events[_event_handle_index] = event;
   if (WSA_INVALID_EVENT == Socket_Vars::socket_events[_event_handle_index])
   {
      printf("ERROR, Client WSACreateEvent() failed with: %u\n", WSAGetLastError());
      result &= false;
      return result;
   }

   //
   // 5. Use 'WSAEventSelect' to associate an event with the socket
   func_result = WSAEventSelect(
      _socket,
      Socket_Vars::socket_events[_event_handle_index],
      FD_ACCEPT | FD_READ | FD_WRITE | FD_CLOSE);
   if (SOCKET_ERROR == func_result)
   {
      printf("ERROR, Server WSAEventSelect() failed with: %u\n", WSAGetLastError());
      result &= false;
      return result;
   }

   return result;
}  // END TCP_Client_Start()

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+    
/// Opens the socket as UDP
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
bool Universal_Socket::UDP_Socket_Start()
{
   bool result = true;
   int func_result;

   //
   // 1. Create a UDP socket
   _socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
   if (INVALID_SOCKET == _socket) {
      printf("UDP %s socket() failed with: %u\n", _socket_name.c_str(), WSAGetLastError());
      result &= false;
      return result;
   }

   //
   // 2. Initialize the SOCKADDR_IN
   _address.sin_family = AF_INET;
   _address.sin_port = htons(_port);
   // InetPton() requires IP Address as a PCWSTR
   // this is the conversion of std::string to PCWSTR
   std::wstring w_str(_ip_address.begin(), _ip_address.end());
   PCWSTR pcwstr_ip = w_str.c_str();
   InetPton(AF_INET, pcwstr_ip, &_address.sin_addr.s_addr);

   //
   // 3. Listen on specific port for incoming messages
   func_result = bind(_socket, (SOCKADDR*)&_address, sizeof(_address));
   if (0 != func_result)
   {
      printf("ERROR, UDP bind() failed with: %u\n", WSAGetLastError());
      result &= false;
      return result;
   }
   else
   {
      printf("UDP Socket %s: Ready for sending and/or receiving messages...\n", _socket_name.c_str());
      result &= true;
      _is_socket_connected = true;
   }

   //
   // 4. Create an event for the listen socket
   HANDLE event = WSACreateEvent();
   Socket_Vars::socket_events[_event_handle_index] = event;
   if (WSA_INVALID_EVENT == Socket_Vars::socket_events[_event_handle_index])
   {
      printf("ERROR, Client WSACreateEvent() failed with: %u\n", WSAGetLastError());
      result &= false;
      return result;
   }

   //
   // 5. Use 'WSAEventSelect' to associate an event with the socket
   func_result = WSAEventSelect(
      _socket,
      Socket_Vars::socket_events[_event_handle_index],
      FD_ACCEPT | FD_READ | FD_WRITE | FD_CLOSE);
   if (SOCKET_ERROR == func_result)
   {
      printf("ERROR, Server WSAEventSelect() failed with: %u\n", WSAGetLastError());
      result &= false;
      return result;
   }

   return result;
}  // END UDP_Socket_Start()

//-+-+-+-+-+-+-+-+-+-+-+PUBLIC FUNCTIONS+-+-+-+-+-+-+-+-+-+-+-+-+-+

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
/// Constructor
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
Universal_Socket::Universal_Socket
(
   Socket_Vars::Protocol protocol_type,
   Socket_Vars::Connection_Type connection_type,
   std::string ip_address,
   uint16_t port,
   std::string name
)
{
   //Initialize private variables
   _protocol = protocol_type;
   _connection = connection_type;
   _ip_address = ip_address;
   _port = port;
   _socket_name = name;
   _socket = INVALID_SOCKET;
   _listen_socket = INVALID_SOCKET;
   _is_socket_connected = false;
   memset(&_address, 0, sizeof(SOCKADDR_IN));

   // function level variables
   int func_result = 0;
   WORD wVersionRequested = MAKEWORD(2, 2);  // Request Winsock v2
   WSADATA wsa_data;

   //
   // 1. Retrieve details of Windows Sockets implementation
   func_result = WSAStartup(wVersionRequested, &wsa_data);
   if (0 != func_result)
   {
      printf("ERROR loading WinSock v2!\n");
   }

   //
   // 2. Create a new event and add it to the global list of socket events
   HANDLE event = WSA_INVALID_EVENT;
   if (!Socket_Vars::Add_Event_To_Event_List(event, _event_handle_index))
   {
      printf("ERROR, too many events exist, failed creation of socket!\n");
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
      result &= UDP_Socket_Start();
   }
   else
   {
      printf("Invalid protocol type specified for Socket!\n");
      result &= false;
   }

   return result;
}  // END Start()

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
/// Handles the event that was triggered on this socket
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
bool Universal_Socket::Handle_Event()
{
   bool result = true;
   int func_result;
   WSANETWORKEVENTS networkEvents;
   char* buffer;
   static int send_count = 1;

   //
   // 1. If _socket is not initialized yet, then the event fired for this socket
   //    must be an accept, in which case it is on the _listen_socket
   if (INVALID_SOCKET == _socket)
   {
      func_result = WSAEnumNetworkEvents(
         _listen_socket,
         Socket_Vars::socket_events[_event_handle_index],
         &networkEvents
      );
   }
   else
   {
      func_result = WSAEnumNetworkEvents(
         _socket,
         Socket_Vars::socket_events[_event_handle_index],
         &networkEvents
      );
   }

   //
   // 2. Check for socket error
   if (SOCKET_ERROR == func_result)
   {
      printf("WSAEnumNetworkEvents() failed with: %u\n", WSAGetLastError());
      result &= false;
      return result;
   }

   //
   // 3. Event fired for this socket was a socket accept
   if ((networkEvents.lNetworkEvents & FD_ACCEPT))
   {
      _socket = accept(_listen_socket, nullptr, nullptr);
      if (INVALID_SOCKET == _socket) 
      {
         printf("%s accept() failed with: %u!\n", _socket_name.c_str(), WSAGetLastError());
         result &= false;
         return result;
      }
      printf("%s accept() succeeded!\n", _socket_name.c_str());
      // If socket is connected and ready to send and receive, then
      // set the flag to true
      _is_socket_connected = true;
   }

   // 
   // 4. Event fired for this socket was a socket accept
   if ((networkEvents.lNetworkEvents & FD_READ))
   {
      result &= Receive(buffer);
      std::string my_string = "Hey Client!";
      my_string += std::to_string(send_count++);
      Socket_Vars::send_queue[_event_handle_index].push(my_string);
   }

   //
   // 5. Event fired for this socket was a close
   if ((networkEvents.lNetworkEvents & FD_CLOSE))
   {
      printf("%s Socket Disconnected!\n", _socket_name.c_str());
      std::thread reconnect_thread(&Universal_Socket::Reconnect, this);
      reconnect_thread.detach();
   }

   return result;
}  // END Handle_Event()

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
/// Sends a message over the socket
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
bool Universal_Socket::Send(const char* buffer)
{
   bool result = true;
   int bytes_sent;
   int buffer_length;
   
   //
   // 1. Get the length of the message
   buffer_length = static_cast<int>(strlen(buffer));

   //
   // 2. Send the message
   if (Socket_Vars::TCP == _protocol)
   {
      bytes_sent = send(_socket, buffer, buffer_length, 0);
   }
   else   // UDP
   {
      bytes_sent = sendto(_socket, buffer, buffer_length, 0, (SOCKADDR*)&_address, sizeof(_address));
   }

   //
   // 3. Check if the send operation was successful
   if (bytes_sent == SOCKET_ERROR) {
      printf("%s Send() failed with error: %u\n", _socket_name.c_str(), WSAGetLastError());
      result &= false;
   }

   //
   // 4. Ensure all bytes were sent
   if (bytes_sent != buffer_length) {
      printf("%s Send() not all bytes were sent!\n", _socket_name.c_str());
      result &= false;
   }

   return result;
}  // END Send()

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
/// Opens the socket ready to receive and or send
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
bool Universal_Socket::Receive(char* &buffer)
{
   bool result = true;
   int bytes_received = 0;
   constexpr uint16_t MAX_RECV_SIZE{ 1024 };
   char recv_buffer[MAX_RECV_SIZE];
   int address_size = sizeof(_address);

   //
   // 1. Receive the message from the socket
   if (Socket_Vars::TCP == _protocol)
   {
      bytes_received = recv(_socket, recv_buffer, MAX_RECV_SIZE - 1, 0);
   }
   else   // UDP
   {
      bytes_received = recvfrom(_socket, recv_buffer, MAX_RECV_SIZE - 1, 0, (SOCKADDR*)&_address, &address_size);
   }
   if (bytes_received > 0)
   {
      //
      // 2. Add a null-terminator to the end of the message
      recv_buffer[bytes_received] = '\0';
      buffer = recv_buffer;
      printf("%s Receive() = %s\n", _socket_name.c_str(), buffer);
      result &= true;
   }
   else if (0 == bytes_received) 
   {
      printf("%s Receive() 0 bytes, closing socket!\n", _socket_name.c_str());
      result &= false;
   }
   else {
      printf("%s Receive() failed, closing socket!\n", _socket_name.c_str());
      result &= false;
   }

   return result;
}  // END Receive()

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
/// Reconnects the socket
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
bool Universal_Socket::Reconnect()
{
   bool result = true;
   bool is_reconnecting = true;
   int func_result;
   WSANETWORKEVENTS networkEvents;

   //
   // 1. Close the socket and mark it as invalid
   closesocket(_socket);
   _socket = INVALID_SOCKET;
   _is_socket_connected = false;

   //
   // 2. Attempt to reconnect by listening for a new connection
   //    this reconnect logic only applicable for TCP
   while (is_reconnecting && Socket_Vars::TCP == _protocol)
   {
      printf("%s Waiting for reconnection...\n", _socket_name.c_str());

      //
      // 2a. Wait for the FD_ACCEPT event to be triggered
      func_result = WSAWaitForMultipleEvents(
         1,
         &Socket_Vars::socket_events[_event_handle_index],
         FALSE,
         WSA_INFINITE, // Wait indefinitely for a new connection
         FALSE
      );

      //
      // 2b. Check for a failure
      if (WSA_WAIT_FAILED == func_result)
      {
         printf("%s WSAWaitForMultipleEvents() failed with: %u\n", _socket_name.c_str(), WSAGetLastError());
         result &= false;
         return result;
      }

      //
      // 2c. Check if the event was for FD_ACCEPT
      func_result = WSAEnumNetworkEvents(_listen_socket, Socket_Vars::socket_events[_event_handle_index], &networkEvents);
      if (SOCKET_ERROR == func_result)
      {
         printf("%s WSAEnumNetworkEvents() failed with: %u\n", _socket_name.c_str(), WSAGetLastError());
         result &= false;
         return result;
      }

      //
      // 2d. Event fired was a socket accept, accept the new connection and exit reconnection logic
      if ((networkEvents.lNetworkEvents & FD_ACCEPT) &&
         (0 == networkEvents.iErrorCode[FD_ACCEPT_BIT]))
      {
         _socket = accept(_listen_socket, nullptr, nullptr);
         if (INVALID_SOCKET == _socket)
         {
            printf("accept() failed with: %u!\n", WSAGetLastError());
            continue; // Try again
         }
         else
         {
            printf("accept() succeeded! New client connected.\n");
            _is_socket_connected = true;
            is_reconnecting = false; // Exit the loop and return to normal processing
         }
      }
   }  // END while is_reconnecting && TCP

   //
   // 3. Reconnection logic for UDP socket
   if (Socket_Vars::UDP == _protocol)
   {
      result &= UDP_Socket_Start();
   }  // END if UDP

   return result;
}  // END Reconnect()

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
   WSACloseEvent(Socket_Vars::socket_events[_event_handle_index]);
   closesocket(_socket);
   WSACleanup();

   return result;
}  // END Stop()