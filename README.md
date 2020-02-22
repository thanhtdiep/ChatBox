# Process Management and Distributed Computing (CAB403 Assignment System Programming)

## Description
The  purpose  of  this service  is  to  manage  a  queue  of  text  messages  to  be  delivered to connected/subscribed connections. All connected clients can add a message and each message is associated with a channel ID. Clients are subscribed to multiple channels and when any of the subscribed channels has a new message, the clients can fetch and see it.
The client and server implemented in the C programming language using BSD sockets on the Linux operating system.

## Commands
### Server-side
- ./server 12345
The server will take only one command line parameter that indicates which port the server is to listen on. If no port number is supplied the default port of 12345 is to be used by the server. The above command will run the server program on port 12345. 

### Client-side
- ./client server_IP_address 12345

- SUB {channelid}

- CHANNELS
- UNSUB {channelid}
- NEXT {channelid}
- LIVEFEED {channelid}
- NEXT
- LIVEFEED
- SEND {channelid} {message}
#### BYE
Closes the client's connection to the server gracefully. This also effectively unsubscribes the user from all channel on that     server, so if the user reconnects, they will have to resubscribe. The same thing should happen if the client is closed due to   SIGINT command being received outside of LIVEFEED mode.



Server file: gcc -o Server Server.c –lpthread \n
Client file: gcc -o Client Client.c

