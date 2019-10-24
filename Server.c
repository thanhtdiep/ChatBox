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

#define DEFAULT_PORT 12345 /* the port users will be connecting to */
#define MAX 1000		   /* message length */
#define BACKLOG 10		   /* how many pending connections queue will hold */
/* Golbal variables */
int sockfd, new_fd, port;	  /* listen on sock_fd, new connection on new_fd */
struct sockaddr_in my_addr;	/* my address information */
struct sockaddr_in their_addr; /* connector's address information */
socklen_t sin_size;
int channel_id[254] = {0}; // ID=0 Available, 1 = Not available/subbed
int32_t client_id;		   // new connection +1
char inbox[1000][254];

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
	int32_t tmp = 0;
	char message[MAX] = {0};
	// bzero(&message, sizeof(message));
	// read the message from client and copy it in buffer
	read(sockfd, message, sizeof(message));
	printf("Client1: %s\n", message);
	// for (int i = 0; i < strlen(message); i++)
	// {
	// 	printf("Client2: %c\n", message[i]);
	// }
	// Validating the channelid with the client
	// for (int i =0; i < sizeof(channel_id); i++){
	// 	if (channel_id[i] == tmp && channel_id[i] == 1){
	// 		printf("Matched");
	// 		counter++;
	// 	}
	// }
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
		// // if (!fork()) { /* this is the child process */
		// // 	if (send(new_fd, "Welcome! Your client ID is \n", 28, 0) == -1)
		// // 		perror("send");
		// 		continue;
		// // }
		send(new_fd, "1", 1, 0);
		bzero(buff, sizeof(buff));
		// read the message from client and copy it in buffer
		read(new_fd, buff, sizeof(buff));
		printf("%s", buff);

		// Actions
		if ((strncmp(buff, "SUB", 3)) == 0)
		{
			printf("SUB process\n");
			subscribe(new_fd);
			break;
		}

		// SEND command
		if ((strncmp(buff, "SEND", 4)) == 0)
		{
			printf("SEND process\n");
			store_message(new_fd);
		}
	}
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

	// main loop once found a connection
	loop_listen(new_fd);
}
