# Universal_Socket
Visual Studio 2022 project for the Universal_Socket class. Also contains an example Driver to create send and receive threads to drive the Universal_Socket

### Summary
Universal_Socket is a class designed to abstract away socket functions to simple function calls that are universal to TCP or UDP, Client or Server.
It is an event based socket using winsock2.

### Requirements
1. Visual Studio 2019 and above

### Implementation
All that the user of the Universal_Socket class must do is...
1. Create one or many Universal_Socket objects by specifying the following in order...
   - Socket_Vars::Protocol (TCP or UDP)
   - Socket_Vars::Connection_Type (Server or Client)
   - IP Address
   - Port Number
   - Name or short description of the socket
2. Start the Universal_Socket objects, Universal_Socket::Start()
3. Create one thread to handle all receiving for every Universal_Socket
4. Create one thread to handle all sending for every Universal_Socket

### Dev Notes
I'm not sure if this is best practice, but I used the Socket_Receiving_Thread as the handler to all events that are raised.
- Socket accept
- Socket close
- Socket receive

Socket_Receiving_Thread() in Communication_Interface.cpp uses the WSAWaitForMultipleEvents() to wait on the array of events, one event being for each socket.

The Socket_Sending_Thread is purely monitoring the send_queue, and calling Universal_Socket::Send() for every message that is placed on the queue. Before sending a message over the socket,
you must make sure the socket is alive by calling Is_Socket_Connected(). If the socket is not connected, the send will fail and return false.

Receive() and Send() work differently depending on UDP or TCP.
- UDP calls recvfrom() and sendto() to receive and send data over a specific IP address and port
- TCP calls recv() and send() to receive and send data over a connected socket
