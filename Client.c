#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 12345 /* the port client will be connecting to */
#define MAX 1000
#define MAXDATASIZE 100 /* max number of bytes we can get at once */
int input;
void send_message(int sockfd, char *channel, char message[]);
void subscribe();
void Next(int sockfd, char *channel);
void unsubscribe();
void channel(int sockfd);
void NextLive(int sockfd);


void loop_listen(int new_fd)
{
	char buff[MAX];
	int n;
	/* repeat: accept, send, close the connection */
	/* for every accepted connection, use a sepetate process or thread to serve it */
	while (1)
	{ /* main accept() loop */
		bzero(buff, sizeof(buff));
		n = 0;
		while ((buff[n++] = getchar()) != '\n')
			;
		write(new_fd, buff, sizeof(buff));
		
		if ((strncmp(buff, "SUB", 3)) == 0)
		{
			printf("SUB process\n");
			char *channel = (char *)malloc(3);
			strncpy(channel, buff + 4, 3);
			printf("%d\n", atoi(channel));
			subscribe(new_fd, channel);
		}

		if ((strncmp(buff, "UNSUB", 5)) == 0)
		{
			printf("UNSUB process\n");
			char *channel = (char *)malloc(3);
			strncpy(channel, buff + 6, 3);
			printf("%d\n", atoi(channel));
			unsubscribe(new_fd, channel);
		}

		if ((strncmp(buff, "SEND", 4)) == 0)
		{
			int i = 0;
			n = 0;
			char message[MAX];
			char *channel = (char *)malloc(3);
			// Send SEND signal to server
			strncpy(channel, buff + 5, 3);
			printf("%d\n", atoi(channel));
			//	Filter message
			for (i = 0; i < sizeof(buff); i++)
			{
				message[i] = buff[5 + i];
			}
			// Run send commands
			send_message(new_fd, channel, message);
		}


		if ((strncmp(buff, "NEXT", 4) == 0) && (strncmp(&buff[5], "\0", 1)!=0)&& (strncmp(&buff[6], "\0", 1)!=0) && (strncmp(&buff[7], "\0", 1) !=0)
)
		{
			char *channel = (char *)malloc(3);
			strncpy(channel, buff + 5, 3);
			//printf("%d\n", atoi(channel));
			Next(new_fd, channel);
		}

		if ((strncmp(buff, "NEXT", 4) == 0) && (strncmp(&buff[5], "\0", 1)==0)&& (strncmp(&buff[6], "\0", 1)==0) && (strncmp(&buff[7], "\0", 1) ==0)
)
		{
			printf("NXT LIVE process\n");
			NextLive(new_fd);
		}

		if ((strncmp(buff, "CHANNEL", 7)) == 0)
		{
			printf("Channel process\n");
			channel(new_fd);
		}

		

		if ((strncmp(buff, "TEST", 4)) == 0)
		{
			printf("Testing");
		}

		if ((strncmp(buff, "BYE", 3)) == 0)
		{
			printf("Client: Exiting...\n");
			/*	Unscribes all channels*/
			break;
		}
	}
}

void send_message(int sockfd, char *channel, char message[])
{
	// Clean buffer
	int32_t tmp = 0;
	int status;
	tmp = atoi(channel);
	// Check if input is within the range of channel id
	if (tmp < 0 || tmp > 255)
	{
		printf("Invalid channel: %d\n", atoi(channel));
	}
	else
	{
		// Send message to server
		printf("%s\n", message);
		if (strlen(message) > 1024 + 4)	printf("Warning: Maximum length of message is 1024. Only 1024 characters will be sent this time \n");
		write(sockfd, message, 1024 + 4);
	}
}

void subscribe(int sockfd, char *channel)
{
	int32_t tmp;
	int status;
	int input;

	tmp = atoi(channel);
	input = atoi(channel);

	write(sockfd, &tmp, sizeof(tmp));
	bzero(&tmp, sizeof(tmp));

	read(sockfd, &tmp, sizeof(tmp));
	status = (int)tmp;

	if (status == 0)
	{
		printf("Subscribed to channel %d\n", input);
	}
	else if (status == 1)
	{
		printf("Already subscribed to Channel %d\n", input);
	}
	else if (status == 2)
	{
		printf("Invalid channel:%d\n", input);
	}
	else
	{
	}
}

void unsubscribe(int sockfd, char *channel)
{

	int32_t tmp;
	int status;
	int input;

	tmp = atoi(channel);
	input = atoi(channel);

	write(sockfd, &tmp, sizeof(tmp));
	bzero(&tmp, sizeof(tmp));

	read(sockfd, &tmp, sizeof(tmp));
	status = (int)tmp;

	if (status == 0)
	{
		printf("Unsubscribed from channel %d\n", input);
	}
	else if (status == 1)
	{
		printf("Not subscribed to channel %d\n", input);
	}
	else if (status == 2)
	{
		printf("Invalid channel:%d\n", input);
	}
	else
	{
	}
}


void Next(int sockfd, char *channel)
{

	int32_t tmp;
	char message[MAX];
	int status=0;
	int channel_input;
	
	tmp = atoi(channel);
	channel_input = atoi(channel);
// Check if channel is subscribed
	write(sockfd, &tmp, sizeof(tmp));
	read(sockfd, &tmp, sizeof(tmp));
	status = (int)tmp;

if (status == 1){
	write(sockfd, &tmp, sizeof(tmp));
	read(sockfd, message, sizeof(message));
	printf("%d:%s\n", channel_input, message);
}

else if (status == 0){
	printf("Channel %d not subscribed\n", channel_input);
}

else if (status == 2){
	printf("Invalid channel:%d\n", channel_input);
}
else{

}

	
	
}

void NextLive(int sockfd)
{

	int32_t tmp;
	char message[MAX];
	int count=0;
	int channel_input=0;
	
	bzero(&tmp, sizeof(tmp));
	read(sockfd, &tmp, sizeof(tmp));
	count = (int)tmp;

// To read all message passed by server
	for (int n=0; n<count; n++){
		read(sockfd, &tmp, sizeof(tmp));
		channel_input = (int)tmp;
		read(sockfd, message, sizeof(message));
		printf("%d:%s\n", channel_input, message);
	}


}


void channel(int sockfd){

	int32_t tmp;
	
	int count = 0;
	int total=0;
	int read_msge=0;
	int unread_msge=0;
	int channel=0;


	bzero(&tmp, sizeof(tmp));
	read(sockfd, &tmp, sizeof(tmp));

	count = (int)tmp;

	for (int n=0; n<count; n++){

		read(sockfd, &tmp, sizeof(tmp));
		channel = (int)tmp;
		printf("\nChannel:%d\n", channel);

		read(sockfd, &tmp, sizeof(tmp));
		total = (int)tmp;
		printf("Total message:%d\n", total);

		read(sockfd, &tmp, sizeof(tmp));
		unread_msge = (int)tmp;
		printf("Unread messages:%d\n", unread_msge);


		read(sockfd, &tmp, sizeof(tmp));
		read_msge = (int)tmp;
		printf("Read messages:%d\n", read_msge);
	}

	
}


int main(int argc, char *argv[])
{
	int sockfd, numbytes, port;
	// char buf[MAXDATASIZE];
	int tmp,n;
	struct hostent *he;
	struct sockaddr_in their_addr; /* connector's address information */

	if (argc < 2)
	{
		fprintf(stderr, "usage: client hostname (or zPAddress [portNumber])\n");
		exit(1);
	}

	if (argc > 2)
	{
		port = atoi(argv[2]);
	}
	else
		port = PORT;

	if ((he = gethostbyname(argv[1])) == NULL)
	{ /* get the host info */
		herror("gethostbyname");
		exit(1);
	}

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("socket");
		exit(1);
	}

	their_addr.sin_family = AF_INET;   /* host byte order */
	their_addr.sin_port = htons(port); /* short, network byte order */
	their_addr.sin_addr = *((struct in_addr *)he->h_addr);
	bzero(&(their_addr.sin_zero), 8); /* zero the rest of the struct */

	if (connect(sockfd, (struct sockaddr *)&their_addr,
				sizeof(struct sockaddr)) == -1)
	{
		perror("connect");
		exit(1);
	}

	if ((numbytes = recv(sockfd, &tmp, sizeof(tmp), 0)) == -1)
	{
		perror("recv");
		exit(1);
	}

	// buf[numbytes] = '\0';
	n = ntohl(tmp);
	// 	Welcome message for new client
	printf("Welcome! Your client ID is %d.\n", n);
	printf("A number of available commands:\n");
	printf("1. SUB <channelid>\t2. CHANNELS\n3. UNSUB <channelid>\t4. NEXT <channelid>\n5. LIVEFEED <channelid>\t6. NEXT\n7. SEND <channelid> <message>\t8. BYE\n");
	printf("Enter command: ");
	loop_listen(sockfd);

	close(sockfd);

	return 0;
}
