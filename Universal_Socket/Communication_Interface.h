//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// 
//  ___________ __  .__                       ___________
//  \_   _____//  |_|  |__ _____    ____      \_   _____/
//   |    __)_\   __\  |  \\__  \  /    \      |    __)_ 
//   |        \|  | |   Y  \/ __ \|   |  \     |        \
//  /_______  /|__| |___|  (____  /___|  / /\ /_______  /
//          \/           \/     \/     \/  \/         \/ 
// 
// Communication Interface Header File
// 
//  Author   Date         Description
// --------------------------------------------------------
//  ECE      08-11-2024   Initial Implementation
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

#pragma once

#include "Universal_Socket.h"

#include <vector>

namespace Comms_Interface
{
   // List of all sockets
   extern std::vector<Universal_Socket> Socket_List;

   //+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   /// @brief   Creates and starts all sockets
   /// @return  bool              Result of sockets creation
   //+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   bool Setup_Comms();

   //+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   /// @brief   Main processing loop for all Socket Sending
   /// @return  None
   //+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   void Socket_Sending_Thread();
   
   //+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   /// @brief   Main processing loop for all Socket Receiving
   /// @return  None
   //+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   void Socket_Receiving_Thread();

}  // END namespace Comms_Interface