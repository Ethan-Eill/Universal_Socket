#include "Universal_Socket.h"

#include <thread>

void Server_Socket_Driver(Universal_Socket Server_Socket);

//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
/// Entry point for Universal_Socket project
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
int main()
{
   printf("This program will be the server!\n");

   Socket_Vars::Protocol protocol_type = Socket_Vars::TCP;
   Socket_Vars::Connection_Type connect_type = Socket_Vars::SERVER;
   std::string ip_address = "127.0.0.1";
   int port = 8080;

   // Create the Server Socket object
   Universal_Socket Server_Socket = Universal_Socket
   (
      protocol_type,
      connect_type,
      ip_address,
      port
   );

   // Server thread to handle function loop for receiving and sending
   std::thread Server_Thread(Server_Socket_Driver, Server_Socket);

   Server_Thread.join();

   return 0;
}  // END main()

void Server_Socket_Driver(Universal_Socket Server_Socket)
{
   bool keep_socket_alive = true;
   unsigned char* buffer;

   // Start the server socket
   keep_socket_alive &= Server_Socket.Start();

   // Main socket loop to receive and send messages to any client connections
   while (keep_socket_alive)
   {
      
   }

   // Stop the server socket
   if (!Server_Socket.Stop())
   {
      printf("Problem stopping the Server Socket!\n");
   }

}  // END Server_Socket_Driver()