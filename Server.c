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
#define MAX 1024		   /* message length */
#define BACKLOG 10		   /* how many pending connections queue will hold */
#define CHANNEL_MAX 255	/* max number of channels */

/* Golbal variables */
int sockfd, new_fd, port;	  /* listen on sock_fd, new connection on new_fd */
struct sockaddr_in my_addr;	/* my address information */
struct sockaddr_in their_addr; /* connector's address information */
socklen_t sin_size;
int channel_id[254] = {0}; // ID=0 Available, 1 = Not available/subbed
pid_t childpid;
volatile int client_id, client_counter;

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
	Queue *Q;
} CHANNEL_ID;
CHANNEL_ID channels[CHANNEL_MAX];
//------------------------------------End of Channel ID-------------------------------------------------------
//------------------------------------Client ID-------------------------------------------------------------
typedef struct
{
	int ID;
	int total;
	int subChannel[CHANNEL_MAX];
} CLIENT_ID;
// Function to create client
CLIENT_ID *createClient()
{
	// // declare pointer for id and counter
	// int *id, *counter;

	// // Point to the global id and counter
	// id = &client_id;
	// counter = &client_counter;

	CLIENT_ID *client;
	client = (CLIENT_ID *)malloc(sizeof(CLIENT_ID));

	// Initialise properties when there is not holes
	client->ID = client_id;
	client_id++;
	client->subChannel;
	client_counter++;
	client->total = client_counter;
	// return the pointer
	return client;
}

void disconnectClient(CLIENT_ID *client)
{
	client_counter--;
	// Reset subbed channel for sure
	memset(client->subChannel, 0, sizeof(client->subChannel));
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

void subscribe(int sockfd)
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

	if (input_id > 0 && input_id < 255)
	{

		if (channel_id[input_id] == 0)
		{
			//printf("Subscribe to channel %d\n", input_id);
			bzero(&tmp, sizeof(tmp));
			tmp = 0;
			write(sockfd, &tmp, sizeof(tmp));
			channel_id[input_id] = input_id;
		}
		else
		{
			bzero(&tmp, sizeof(tmp));
			tmp = 1;
			write(sockfd, &tmp, sizeof(tmp));
			//printf("Channel already subscribed\n");
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

void unsubscribe(int sockfd)
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

	if (input_id > 0 && input_id < 255)
	{

		if (channel_id[input_id] == input_id)
		{
			//printf("unsubscribe to channel %d\n", input_id);
			bzero(&tmp, sizeof(tmp));
			tmp = 0;
			write(sockfd, &tmp, sizeof(tmp));
			channel_id[input_id] = 0;
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
	// Send back successful message
	write(sockfd, "Successfully saved message!", 28);
}

void livefeed(int new_fd, int channel, CLIENT_ID *client)
{
	int i = 0;
	char message[MAX] = {0};
	int t = htonl(channels[channel].Q->size);
	// Validation for subscription channels
	if (client->subChannel[channel] == 0)
	{
		t = -1;
		write(new_fd, &t, sizeof(t));
	}
	else
	{
		write(new_fd, &t, sizeof(t));
		printf("LIVEFEED <channelid> loop start\n");
		while (1)
		{
			while (channels[channel].Q->size > 0)
			{
				strcpy(message, front(channels[channel].Q));
				write(new_fd, message, sizeof(message));
				dequeue(channels[channel].Q);
				bzero(message, sizeof(message));
			}
			if (i == 0)
			{
				write(new_fd, "All unread messages have been shown.\nWaiting for new messages..\n", 64);
				i++;
			}
			// listening stage
			if ((t = read(new_fd, message, sizeof(message))) > 0)
			{
				if (strncmp(message, "1", 1) == 0)
				{
					break;
				}
			}
		}
		printf("LIVEFEED <channelid> loop break\n");
	}
}

void livefeed_all(int new_fd, CLIENT_ID *client)
{
	int sub_channel[CHANNEL_MAX] = {0};
	char **messages = malloc(sizeof(char *) * 50);
	// for each sub channel get all the unread message
	for (int i= 0; i < CHANNEL_MAX; i++)
	{
		if(client->subChannel[i] == 1)
		{
			sub_channel[i] = 1;
			for (int j = 0; j < channels[i].Q->size; j++)
			{
				messages[j] = (char *)malloc((sizeof channels[i].Q->front) * sizeof(char));
				strcpy(messages[j], front(channels[i].Q));
				dequeue(channels[i].Q);
			}
			// printf("%d: %s", client->subChannel[i], front(channels[i].Q));
		}
	}
	// Send array with subchannel and its total
	write(new_fd, sub_channel, sizeof(sub_channel));
	write(new_fd, messages, sizeof(messages));
	free(messages);
}

void loop_listen(int new_fd)
{
	char buff[MAX] = {0};
	int n;

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
		printf("server: got connection from %s:%d\n",
			   inet_ntoa(their_addr.sin_addr), ntohs(their_addr.sin_port));

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
				/* SUB */
				if ((strncmp(buff, "SUB", 3)) == 0)
				{
					printf("SUB process\n");
					subscribe(new_fd);
					bzero(buff, sizeof(buff));
				}

				/* SEND */
				if ((strncmp(buff, "SEND", 4)) == 0)
				{
					printf("SEND process\n");
					store_message(new_fd);
					bzero(buff, sizeof(buff));
				}

				/* LIVEFEED <channelid> */
				if (((strncmp(buff, "LIVEFEED", 8)) == 0) && (strncmp(&buff[9], "\0", 1) != 0) && (strncmp(&buff[10], "\0", 1)!=0) && (strncmp(&buff[11], "0", 1) !=0))
				{
					printf("LIVEFEED <channelid> process\n");
					char *channel = (char *)malloc(3);
					strncpy(channel, buff + 9, 3);
					livefeed(new_fd, atoi(channel), client);
					bzero(buff, sizeof(buff));
					free(channel);
				}

				/* LIVEFEED */
				if (((strncmp(buff, "LIVEFEED", 8)) == 0) && (strncmp(&buff[9], "\0", 1) == 0) && (strncmp(&buff[10], "\0", 1)==0) && (strncmp(&buff[11], "\0", 1) ==0))
				{
					printf("LIVEFEED process\n");
					livefeed_all(new_fd, client);
					bzero(buff, sizeof(buff));
				}

				/* BYE */
				if ((strncmp(buff, "BYE", 3)) == 0)
				{
					printf("Disconnected with %s:%d\n", inet_ntoa(their_addr.sin_addr), ntohs(their_addr.sin_port));
					// Unsubcribe with all subbed channels
					disconnectClient(client);
					break;
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
