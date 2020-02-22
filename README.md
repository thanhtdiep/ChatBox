# Process Management and Distributed Computing (CAB403 Assignment System Programming)

## Description
The  purpose  of  this service  is  to  manage  a  queue  of  text  messages  to  be  delivered to connected/subscribed connections. All connected clients can add a message and each message is associated with a channel ID. Clients are subscribed to multiple channels and when any of the subscribed channels has a new message, the clients can fetch and see it.
The client and server implemented in the C programming language using BSD sockets on the Linux operating system.

## Commands
### Server-side
#### ./server 12345
The server will take only one command line parameter that indicates which port the server is to listen on. If no port number is supplied the default port of 12345 is to be used by the server. The above command will run the server program on port 12345. 

### Client-side
#### ./client server_IP_address 12345
The  client  will  take  two  (2)  command  line  parameters:  hostname  and  port  number.  The  above command will run the client program connecting to the server on port 12345

A client will receive a welcome message from the server, which must be displayed to the user.  The welcome  message  will  include  an  ID  number  automatically  assigned to  the  client.    e.g.  “Welcome! Your client ID is <client id>.” 

#### SUB {channelid}
Subscribe  the  client  to  the  given  channel  ID  to  follow  future  messages  sent  to  this  channel. channelid  is  an  integer  from  0-  255.  If  the subscription  is  successful,  the server  will  send  a  confirmation  message  to  the  client.  The  message should  be  in  the  format  “Subscribed  to  channel  <channelid>.”   If  the  channel  ID  is  invalid,  a  message  like  “Invalid  channel:  <channelid>.”  will  be  displayed.  
  
If  the  given  channel  ID  is  valid  but  the  user  is  already  subscribed to this channel, the message shown will instead be “Already subscribed to channel <channelid>.” 
  
#### CHANNELS
Show a list of subscribed channels together with the statistics of how many messages have beensent to that channel, how many messages are marked as read by this client, how many messages are ready but have not yet been read by the client:
- The  first  value  counts  messages  since  the  start  of  the  server  and  includes  messages  before the client subscribed to this channel. 
- The  second  value  counts  messages  that  have  been  read  by  the  client.  This  means  messages sent after the client subscribed to the channel that have been retrieved with the commands NEXT / LIVEFEED.
- The third value counts messages that have not yet been read by the client. 
This means messages sent after the client subscribed to the channel but have not yet been retrieved with NEXT / LIVEFEED.The list is sorted by channel order. Each channel is on a separate line, and values are delimited with a tab character. 
This command will notlist channels that the client has not subscribed to. If the client has not subscribed to any channels, this command will display no output at all.

#### UNSUB {channelid}
Unsubscribe  the  client  from  the  channel  with  the  given  channel  ID.  If  successful, the server will send a confirmation message to the client, which the client must display to the user, in the format “Unsubscribed from channel <channelid>.”  If the channel ID is invalid, a message like “Invalid channel: <channelid>.” will be displayed. If the given channel ID is valid but the user is not subscribed to this channel, the message shown will instead be “Not subscribed to channel <channelid>.”
  
#### NEXT {channelid}
Fetch and display the next unread message on the channel with the given ID. If the channel ID is  invalid,  a  message  like  “Invalid  channel:  <channelid>.”  will  be  displayed.  If  the  given  channel ID is valid but the user is not subscribed to this channel, the message shown will instead be  “Not  subscribed  to  channel  <channelid>.”  If  the  channel  ID  is  valid  and  the  user  is  subscribed but there are no new messages, nothing should be displayed at all. 

Note that the only messages the client can receive are ones that are sent to the channel after the user subscribed to that channel.
  
#### LIVEFEED {channelid}
This  command  acts  as  a  continuous  version  of  the  NEXT  command,  displaying  all  unread  messages on the given channel, then waiting, displaying new messages as they come in. Using Ctrl+C to send a SIGINT signal to the client while in the middle of LIVEFEED will cause the client to stop displaying the live feed and return to normal operation. (If a SIGINT is received by  the  client  at  any  other  time,  the  client  should  instead  gracefully  exit.)  The  LIVEFEED  command displays the same messages as the NEXT command if the channel ID is invalid or if the user is not subscribed to it.

#### NEXT
This  version  of  the  NEXT  command  (without  the  channelid  argument)  will  display  the  next  unread message out of all the channels the user is subscribed to. If the user is not subscribed to any channels, it will instead display the message “Not subscribed to any channels.” If there are no new messages to display, nothing will be output. 

When displayed, the message will be prefixed with “<channelid>:” to indicate which channel the  message  came  from.  So  if,  for  example,  channel  127  has  the  unread  message  “Hello  world.”, the text “127:Hello world.” will be displayed to the user if it is retrieved using this version of the NEXT command. 
  
Note that the server will need to keep track of either the time each message was received or the order they were received in across all channels for this command to work. 
  
#### LIVEFEED
This version of the LIVEFEED command displays messages continuously in the same way the regular LIVEFEED command works, except it displays messages that appear on any channel the user is subscribed to, like the parameterless NEXT command. Like the parameterless NEXT command, each line should also be prefixed with the channel it came from. If the user is not subscribed  to  any  channels,  it  will  instead  display  the  message  “Not  subscribed  to  any channels.”

#### SEND {channelid} {message}
Send a new message to the channel with the given ID. Everything between the space after the channel ID and before the line ending is the message that should be sent. You can assume a maximum message length of 1024 bytes. Users that want to send longer messages will have to divide then up into 1024 byte segments. 

If the channel ID is invalid, a message like “Invalid channel: <channelid>.” will be displayed. It is legal to send a message to a channel, whether the user sending the message is subscribed to that channel or not.
  
#### BYE
Closes the client's connection to the server gracefully. This also effectively unsubscribes the user from all channel on that     server, so if the user reconnects, they will have to resubscribe. The same thing should happen if the client is closed due to   SIGINT command being received outside of LIVEFEED mode.

