/*
*  Materials downloaded from the web. See relevant web sites listed on OLT
*  Collected and modified for teaching purpose only by Jinglan Zhang, Aug. 2006
*/

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <limits.h>

#define DEFAULT_PORT 12345 /* the port users will be connecting to */
#define MAX 1000		   /* message length */
#define BACKLOG 10		   /* how many pending connections queue will hold */
#define CHANNEL_MAX 255	/* max number of channels */

/* Golbal variables */
int sockfd, new_fd, port;	  /* listen on sock_fd, new connection on new_fd */
struct sockaddr_in my_addr;	/* my address information */
struct sockaddr_in their_addr; /* connector's address information */
socklen_t sin_size;
int channel_id[254] = {0}; // ID=0 Available, 1 = Not available/subbed
int32_t client_id;		   // new connection +1
pid_t childpid;
char inbox[1000][254];
int read_count[256]={0};
//--------------------------------------Queue--------------------------------------------------------------
typedef struct Queue
{
	int capacity, size, front, rear;
	char **entries;
} Queue;

// Function to create queue with maxEntry
Queue *createQueue(int maxEntry)
{
	Queue *Q;
	Q = (Queue *)malloc(sizeof(Queue));
	// Initialise properties
	Q->entries = malloc(sizeof(char *) * maxEntry);
	Q->size = 0;
	Q->capacity = maxEntry;
	Q->front = 0;
	Q->rear = -1;
	// return the pointer
	return Q;
}

void dequeue(Queue *Q)
{
	if (Q->size != 0)
	{
		Q->size--;
		Q->front++;
		// As we fill entries in circular fashion
		if (Q->front == Q->capacity)
		{
			Q->front = 0;
		}
		return;
	}
}

char *front(Queue *Q)
{
	if (Q->size != 0)
	{
		// return the front entry
		return Q->entries[Q->front];
	}
	return NULL;
}

void enqueue(Queue *Q, char *element)
{
	// if queue is full, we cannot push an element into it as there is no space
	if (Q->size == Q->capacity)
	{
		printf("Inbox is full\n");
	}
	else
	{
		Q->size++;
		Q->rear = Q->rear + 1;
		// As we fill the queue in circular fashopm
		if (Q->rear == Q->capacity)
		{
			Q->rear = 0;
		}
		// Insert the elements in its rear side
		Q->entries[Q->rear] = (char *)malloc((sizeof element + 1) * sizeof(char));
		strcpy(Q->entries[Q->rear], element);
		printf("Saved message successfully\n");
	}
	return;
}
// -------------------------------------------End of Queue-----------------------------------------------
//------------------------------------Channel ID------------------------------------------------------------
typedef struct
{
	int index; 
	int status; //Subscribe status
	Queue *Q;
} CHANNEL_ID;

//Initialising channel ID (255)
CHANNEL_ID channels[CHANNEL_MAX];

//------------------------------------End of Channel ID-------------------------------------------------------
//------------------------------------Client ID-------------------------------------------------------------
typedef struct 
{
	int ID;
	int total;
	int status;
	int subChannel[CHANNEL_MAX];
}CLIENT_ID;

// Function to create client
CLIENT_ID *createClient()
{
	CLIENT_ID *client;
	client = (CLIENT_ID *)malloc(sizeof(CLIENT_ID));
	// Initialise properties
	client->ID = client_id;
	client_id++;
	client->subChannel;
	client->total++;
	// return the pointer
	return client;
}

void disconnectClient(CLIENT_ID *client)
{
	client->total--;
	// Thinking about ID
}
//------------------------------------End of Client ID------------------------------------------------------
void shutdown_server(int sig)
{
	while (waitpid(-1, NULL, WNOHANG) > 0)
		;		   /* clean up child processes */
	close(new_fd); /* parent doesn't need this */
	close(sockfd);
	exit(0);
}

void subscribe(int sockfd, CLIENT_ID *client)
{
	int input_id = 0;
	int32_t tmp;

	bzero(&tmp, sizeof(tmp));

	// read the message from client and copy it in buffer
	read(sockfd, &tmp, sizeof(tmp));
	// print buffer which contains the client contents
	printf("Client request channel %d\n", (int)tmp);

	input_id = (int)tmp;
	bzero(&tmp, sizeof(tmp));

	if (input_id > 0 && input_id < 256)
	{

		if (client->subChannel[input_id] == 0)
		{
			//printf("Subscribe to channel %d\n", input_id);
			bzero(&tmp, sizeof(tmp));
			tmp = 0;
			write(sockfd, &tmp, sizeof(tmp));
			client->subChannel[input_id] = 1;
			printf("Test status:%d\n", client->subChannel[input_id]);
//Testing
		/*	printf("Before:%d\n", channels[input_id].status);
			channel_id[input_id] = input_id;
			channels[input_id].status = 1;
			printf("Test status:%d\n", channels[input_id].status);*/
		}
		else
		{
			bzero(&tmp, sizeof(tmp));
			tmp = 1; //Channel already subscribe
			write(sockfd, &tmp, sizeof(tmp));
			//printf("Channel already subscribed\n");
		}
	}
	else
	{
		bzero(&tmp, sizeof(tmp));
		tmp = 2; //Error range
		write(sockfd, &tmp, sizeof(tmp));
		//printf("Channel range 0 to 255 only\n");
	}
}

void unsubscribe(int sockfd, CLIENT_ID *client)
{

	int input_id = 0;
	int32_t tmp;

	bzero(&tmp, sizeof(tmp));

	// read the message from client and copy it in buffer
	read(sockfd, &tmp, sizeof(tmp));
	// print buffer which contains the client contents

	printf("Client request unsubscribe %d\n", (int)tmp);

	input_id = (int)tmp;
	bzero(&tmp, sizeof(tmp));
	//channel_id[5] = 0;

	if (input_id > 0 && input_id < 256)
	{

		if (client->subChannel[input_id] != 0)
		{
			//printf("unsubscribe to channel %d\n", input_id);
			bzero(&tmp, sizeof(tmp));
			tmp = 0;
			write(sockfd, &tmp, sizeof(tmp));
			client->subChannel[input_id] = 0;
			
		}
		else
		{
			bzero(&tmp, sizeof(tmp));
			tmp = 1;
			write(sockfd, &tmp, sizeof(tmp));
			//printf("Channel already unsubscribed\n");
		}
	}
	else
	{
		bzero(&tmp, sizeof(tmp));
		tmp = 2;
		write(sockfd, &tmp, sizeof(tmp));
		//printf("Channel range 0 to 255 only\n");
	}
}

void store_message(int sockfd)
{
	char message[MAX] = {0};
	char *channel = (char *)malloc(3);
	// bzero(&message, sizeof(message));
	// read the message from client and copy it in buffer
	read(sockfd, message, sizeof(message));
	// Filter channel
	strncpy(channel, message, 3);
	//	Filter message
	for (int i = 0; i < sizeof(message); i++)
	{
		message[i] = message[4 + i];
	}
	// enqueue message to the inbox queue
	enqueue(channels[atoi(channel)].Q, message);
	//channels[atoi(channel)].index ++;
	
	// Validating the channelid with the client
	// for (int i =0; i < sizeof(channel_id); i++){
	// 	if (channel_id[i] == tmp && channel_id[i] == 1){
	// 		printf("Matched");
	// 		counter++;
	// 	}
	// }
}


// Gets the <channel id> and prints the next unread messages sent to that <channel id>
void Next(int sockfd, CLIENT_ID *client){

	int request_id = 0;
	int32_t tmp;
	char message[MAX];
	
	//Reads the channel ID requested from client
	read(sockfd, &tmp, sizeof(tmp));
	
	//Get message from the channel_id
	request_id = (int)tmp;
	printf("Request:%d\n", request_id);

if (request_id > 0 && request_id < 256)
{
	//Pass the message to client
	if (client->subChannel[request_id] == 1){
	tmp = 1;
	write(sockfd, &tmp, sizeof(tmp));

	strcpy(message, front(channels[request_id].Q));
	write(sockfd, message, sizeof(message));
	printf("Transfer:%s\n", message);

	//To remove read messages
	dequeue(channels[(request_id)].Q);
	read_count[request_id] ++;

	}

	else
	{
		tmp = 0;
		write(sockfd, &tmp, sizeof(tmp));
	}
}

else{
	printf("Invalid channel\n");
	//pass status
	tmp = 2;
	write(sockfd, &tmp, sizeof(tmp));
}


	// -----------------------------------
	
}


//  Prints all the UNREAD Messages from ALL channels and waits for the next one
void NextLive(int sockfd, CLIENT_ID *client){

char message[MAX];
int count=0;
//int count2=0;
int32_t tmp;

for (int i=0; i<MAX; i++){
	if (client->subChannel[i] == 1)
	{
		count++;
	}
}

tmp = count;

	//Sends the count to client get the loop count
write(sockfd, &tmp, sizeof(tmp));


// For loop to get all channels
for (int i=0; i<MAX; i++){

	if (client->subChannel[i] == 1)
	{
		strcpy(message, front(channels[i].Q));
		//Pass channel id
		printf("\nChannel:%d\n", i);
		tmp = i;
		write(sockfd, &tmp, sizeof(tmp));

		//Pass message
		strcpy(message, front(channels[i].Q));
		write(sockfd, message, sizeof(message));
		printf("Transfer:%s\n", message);

		dequeue(channels[i].Q);
		// printf("\nChannel:%d\n", i);
		// printf("Transfer:%s\n", message);
		//count2++;
	}



}

	printf("Count msge:%d\n", (int)count);

	
	
}

void channel(int sockfd, CLIENT_ID *client){

int count=0;
int32_t tmp;


for (int i=0; i<MAX; i++){
		if (client->subChannel[i] == 1)
		{
			count++;
		}
	}

	tmp = count;

	//Sends the count to client get the loop count
	write(sockfd, &tmp, sizeof(tmp));

for (int i=0; i<MAX; i++){
	if (client->subChannel[i] == 1)
	{
		printf("\nChannel:%d\n", i);
		tmp = i;
		write(sockfd, &tmp, sizeof(tmp));

		printf("Total messages:%d\n", read_count[i]+channels[i].Q->size);
		
		tmp = read_count[i]+channels[i].Q->size;
		write(sockfd, &tmp, sizeof(tmp));
		
		printf("Unread messages:%d\n", channels[i].Q->size);

		tmp = channels[i].Q->size;
		write(sockfd, &tmp, sizeof(tmp));

		printf("Read messages:%d\n", read_count[i]);

		tmp = read_count[i];
		write(sockfd, &tmp, sizeof(tmp));


	}

}


}



void loop_listen(int new_fd)
{
	char buff[MAX] = {0};
	int n, ch;
	/* repeat: accept, send, close the connection */
	/* for every accepted connection, use a sepetate process or thread to serve it */
	while (1)
	{ /* main accept() loop */
		sin_size = sizeof(struct sockaddr_in);
		if ((new_fd = accept(sockfd, (struct sockaddr *)&their_addr,
							 &sin_size)) == -1)
		{
			perror("Accepting message");
			continue;
		}
		printf("server: got connection from %s\n",
			   inet_ntoa(their_addr.sin_addr));
			   
		CLIENT_ID *client = createClient();
		int tmp = htonl(client->ID);
		send(new_fd, &tmp, sizeof(tmp), 0);
		if ((childpid = fork()) == 0)
		{ /* this is the child process */
			// Send Client ID to the client side
			while (1)
			{
				// read the message from client and copy it in buffer
				read(new_fd, buff, sizeof(buff));
				// Actions
				if ((strncmp(buff, "SUB", 3)) == 0)
				{
					printf("SUB process\n");
					subscribe(new_fd, client);
					bzero(buff, sizeof(buff));					
				}

				if ((strncmp(buff, "UNSUB", 5)) == 0)
				{
					printf("UNSUB process\n");
					unsubscribe(new_fd, client);
					bzero(buff, sizeof(buff));					
				}

				// SEND command
				if ((strncmp(buff, "SEND", 4)) == 0)
				{
					printf("SEND process\n");
					store_message(new_fd);
					bzero(buff, sizeof(buff));
				}


				if ((strncmp(buff, "NEXT", 4)) == 0)
				{
					printf("NEXT process\n");
					Next(new_fd, client);
					bzero(buff, sizeof(buff));
				}
//(strncmp(buff, "NEXT", 4) == 0) && (strncmp(&buff[5], "\0", 1)==0)&& (strncmp(&buff[6], "\0", 1)==0) && (strncmp(&buff[7], "\0", 1) ==0)
				if ((strncmp(buff, "NXT", 3)) == 0)
				{
					printf("NXT LIVE process\n");
					NextLive(new_fd, client);
					bzero(buff, sizeof(buff));					
				}

				if ((strncmp(buff, "CHANNEL", 7)) == 0)
				{
					printf("CHANNEL LIVE process\n");
					channel(new_fd, client);
					bzero(buff, sizeof(buff));					
				}
				

				if ((strncmp(buff, "BYE", 3)) == 0)
				{
					printf("Disconnected with %s\n", inet_ntoa(their_addr.sin_addr));
					// Unsubcribe with all subbed channels
					bzero(buff, sizeof(buff));
				}
			}
		}
	}
}

void generateChannels()
{
	for (int i = 0; i < CHANNEL_MAX; i++)
	{
		channels[i].index = i;
		channels[i].Q = createQueue(50);
	}
	printf("System: All channels initialised\n");
}

int main(int argc, char *argv[])
{
	signal(SIGINT, shutdown_server); /* Let program exit when ctrl + c is pressed */

	//	Check port number if not given use default port 12345
	if (argc < 2)
	{
		printf("No port provided. Using default port %d\n", DEFAULT_PORT);
		port = DEFAULT_PORT;
	}
	else if (argc == 2)
	{
		port = atoi(argv[1]);
	}
	else
	{
		fprintf(stderr, "Usage: %s [PORT]", argv[0]);
		exit(1);
	}

	/* generate the socket */
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("socket");
		exit(1);
	}

	/* generate the end point */
	my_addr.sin_family = AF_INET;			  /* host byte order */
	my_addr.sin_port = htons(port);			  /* short, network byte order */
	my_addr.sin_addr.s_addr = INADDR_ANY;	 /* auto-fill with my IP */
	/* bzero(&(my_addr.sin_zero), 8);   ZJL*/ /* zero the rest of the struct */

	/* bind the socket to the end point */
	if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1)
	{
		perror("bind");
		exit(1);
	}

	/* start listnening */
	if (listen(sockfd, BACKLOG) == -1)
	{
		perror("listen");
		exit(1);
	}

	printf("Server starts listening on port %d...\n", port);

	// Generate channels upon startup
	generateChannels();

	// main loop once found a connection
	loop_listen(new_fd);
	close(new_fd);
}
