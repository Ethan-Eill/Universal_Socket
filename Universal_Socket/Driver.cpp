//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// 
//  ___________ __  .__                       ___________
//  \_   _____//  |_|  |__ _____    ____      \_   _____/
//   |    __)_\   __\  |  \\__  \  /    \      |    __)_ 
//   |        \|  | |   Y  \/ __ \|   |  \     |        \
//  /_______  /|__| |___|  (____  /___|  / /\ /_______  /
//          \/           \/     \/     \/  \/         \/ 
// 
// Main Driver for Project
// 08-11-2024
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

#include "Universal_Socket.h"
#include "Communication_Interface.h"

#include <thread>
#include <vector>
#include <string>

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
/// Entry point for Universal_Socket project
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
int main()
{
   bool result = true;
   printf("This program will be the server!\n");

   // Setup_Comms() handles the creation of the sockets
   result &= Comms_Interface::Setup_Comms();
   if (!result)
   {
      printf("Problem occurred setting up communication interfaces!\n");
   }

   // Socket threads to handle main processing loop 
   // for receiving and sending through all sockets
   std::thread Socket_Receiving_Thread(Comms_Interface::Socket_Receiving_Thread);
   std::thread Socket_Sending_Thread(Comms_Interface::Socket_Sending_Thread);

   Socket_Sending_Thread.detach();
   Socket_Receiving_Thread.detach();

   while (true)
   {
      // Loop forever
   }

   return 0;
}  // END main()