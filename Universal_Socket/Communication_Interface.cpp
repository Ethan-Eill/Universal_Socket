//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// 
//  ___________ __  .__                       ___________
//  \_   _____//  |_|  |__ _____    ____      \_   _____/
//   |    __)_\   __\  |  \\__  \  /    \      |    __)_ 
//   |        \|  | |   Y  \/ __ \|   |  \     |        \
//  /_______  /|__| |___|  (____  /___|  / /\ /_______  /
//          \/           \/     \/     \/  \/         \/ 
// 
// Communication Interface Implementation File
// 08-11-2024
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

#include "Communication_Interface.h"

namespace Comms_Interface
{
   // List of all sockets
   std::vector<Universal_Socket> Socket_List;

   //+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+    
   /// Creates and starts all sockets
   //+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   bool Setup_Comms()
   {
      bool result = true;
      printf("Setting up the server socket!\n");
      Socket_Vars::Protocol protocol_type = Socket_Vars::TCP;
      Socket_Vars::Connection_Type connect_type = Socket_Vars::SERVER;
      std::string ip_address = "127.0.0.1";
      std::string socket_name = "Universal_Socket->Socket_Tester";
      int port = 8080;

      //
      // 1. Create the all the Socket objects
      Universal_Socket Server_Socket = Universal_Socket
      (
         protocol_type,
         connect_type,
         ip_address,
         port,
         socket_name
      );

      //
      // 2. Add all Socket objects to the list of socket objects
      Socket_List.push_back(std::move(Server_Socket));

      //
      // 3. Start all the sockets in the Socket_List
      for (int i = 0; i != Socket_List.size(); i++)
      {
         result &= Socket_List[i].Start();
         if (!result)
         {
            printf("Problem occurred starting socket %d\n", i);
            Socket_List[i].Stop();
            exit(EXIT_FAILURE);
         }

      }

      return result;
   }  // END Setup_Comms()

   //+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   /// Main processing loop for all Socket Sending
   //+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   void Socket_Sending_Thread()
   {
      bool keep_comms_alive = true;
      bool send_result = true;
      std::string message;    // second item in the pair

      // Main socket loop to receive and send messages to any connections
      while (keep_comms_alive)
      {
         for (int i = 0; i < Socket_List.size(); i++)
         {
            //
            // 1. Lock the send_mutex for thread safety
            std::unique_lock<std::mutex> lock(Socket_Vars::send_mutex[i]);

            //
            // 2. If the queue is not empty, then send the message waiting in the queue
            if (false == Socket_Vars::send_queue[i].empty())
            {
               //
               // 2a. Dequeue the message from the send queue
               message = Socket_Vars::send_queue[i].front();
               Socket_Vars::send_queue[i].pop();

               //
               // 2b. Send the message
               send_result = Socket_List[i].Send(message.c_str());
               if (!send_result)
               {
                  printf("Socket %d Send() failed!\n", i);
               }
            }
         }  // END Socket_List loop

      }  // END while keep_comms_alive

      printf("Closing Socket_Sending_Thread()!\n");

   }  // END Socket_Sending_Thread()

   //+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   /// Main processing loop for all Socket Receiving
   //+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   void Socket_Receiving_Thread()
   {
      bool result = true;
      bool keep_comms_alive = true;
      DWORD event_number;

      // Main receiving loop for all sockets
      while (keep_comms_alive)
      {
         //
         // 1. Wait until an event is triggered on a socket
         //    return value is the index which event was triggered
         event_number = WSAWaitForMultipleEvents(
            Socket_Vars::event_count,    // Number of events to wait for
            Socket_Vars::socket_events,  // List of event handles
            FALSE,                       // Wait for all events to be signaled
            WSA_INFINITE,                // Timeout interval
            TRUE                         // Whether the wait should be alterable
         );

         //
         // 2. If the event that triggered was a failure then exit the Communication Loop
         if (WSA_WAIT_FAILED == event_number)
         {
            printf("WSAWaitForMultipleEvents() failed with: %u\n", WSAGetLastError());
            keep_comms_alive = false;
         }
         //
         // 3. If the event that triggered was a timeout then exit the Communication Loop
         else if (WSA_WAIT_TIMEOUT == event_number)
         {
            printf("WSAWaitForMultipleEvents() Timeout failed, shouldn't happen, %u\n", WSAGetLastError());
            keep_comms_alive = false;
         }
         //
         // 4. Otherwise, handle the event for the socket that triggered the event
         else
         {
            result = Socket_List[(event_number - WAIT_OBJECT_0)].Handle_Event();
            if (!result)
            {
               printf("Handle_Event() returned false!\n");
            }
         }

      }  // END while keep_comms_alive
      printf("Closing Socket_Receiving_Thread()!\n");

   }  // END Socket_Receiving_Thread()

}  // END namespace Comms_Interface