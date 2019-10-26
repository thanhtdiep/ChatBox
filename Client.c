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

/* Golbal variables */
int sockfd, numbytes, port;
int tmp, n;
struct hostent *he;
struct sockaddr_in their_addr; /* connector's address information */

int input;
volatile sig_atomic_t stop;
void send_message(int sockfd, char *channel, char message[]);
void subscribe();
void livefeed(int sockfd, int channel);
void shutdown_client(int signum);
void livefeed_all(int sockfd);

void loop_listen(int new_fd)
{
	char buff[MAX];
	int n;
	/* repeat: accept, send, close the connection */
	/* for every accepted connection, use a sepetate process or thread to serve it */
	while (1)
	{ /* main accept() loop */
		signal(SIGINT, shutdown_client);
		bzero(buff, sizeof(buff));
		n = 0;
		while ((buff[n++] = getchar()) != '\n')
			;
		write(new_fd, buff, sizeof(buff));
		if ((strncmp(buff, "SUB", 3)) == 0)
		{
			printf("SUB process\n");
			/*	Unscribes all channels*/
			// Clean buffer
			bzero(buff, sizeof(buff));
			// Send selected commnand to server
			subscribe(new_fd);
			break;
		}

		if ((strncmp(buff, "SEND", 4)) == 0)
		{
			int i = 0;
			n = 0;
			char message[MAX];
			char *channel = (char *)malloc(3);
			// Send SEND signal to server
			strncpy(channel, buff + 5, 3);
			//	Filter message
			for (i = 0; i < sizeof(buff); i++)
			{
				message[i] = buff[5 + i];
			}
			// Run send commands
			send_message(new_fd, channel, message);
			free(channel);
		}

		/* LIVEFEED <channelid> */
		if (((strncmp(buff, "LIVEFEED", 8)) == 0) && (strncmp(&buff[9], "\0", 1) != 0) && (strncmp(&buff[10], "\0", 1) != 0) && (strncmp(&buff[11], "\0", 1) != 0))
		{
			printf("%s\n", &buff[9]);
			printf("%s\n", &buff[10]);
			printf("%s\n", &buff[11]);

			char *channel = (char *)malloc(3);
			strncpy(channel, buff + 9, 3);
			// Handling the message
			livefeed(new_fd, atoi(channel));
			bzero(buff, sizeof(buff));
			free(channel);
		}

		/* LIVEFEED */
		if ((strncmp(buff, "LIVEFEED", 8) == 0) && (strncmp(&buff[9], "\0", 1) == 0) && (strncmp(&buff[10], "\0", 1) == 0) && (strncmp(&buff[11], "\0", 1) == 0))
		{
			char *channel = (char *)malloc(3);
			strncpy(channel, buff + 9, 3);
			// Handling the message
			livefeed_all(new_fd);
			bzero(buff, sizeof(buff));
			free(channel);
		}

		if ((strncmp(buff, "BYE", 3)) == 0)
		{
			printf("Client: Exiting...\n");
			/*	Unscribes all channels*/
			break;
		}
	}
}
void exit_loop(int signum)
{
	stop = 1;
	printf("Exit LIVEFEED <channelid> loop\n");
}

void shutdown_client(int sig)
{
	while (waitpid(-1, NULL, WNOHANG) > 0)
		; /* clean up child processes */
	close(sockfd);
	exit(0);
}

void livefeed(int sockfd, int channel)
{
	char message[MAX] = {0};
	int t, j;
	int h = 0;
	int i = 1;
	signal(SIGINT, exit_loop); /* exit when ctrl + c is pressed */
	read(sockfd, &t, sizeof(t));
	// Receive validation whether client is subbed for the channel
	if (t == -1)
	{
		printf("Server: Not subscrbed to channel %d\n", channel);
	}
	else if (t == 0)
	{
		printf(""); // Display nothing
	}
	else
	{
		// Welcome message of LIVEFEED
		printf("You are now in LIVEFEED <channelid>. Please press CTRL + C to back to menu.\nAll unread messages of channel %d are below:\n", channel);
		t = ntohl(t);
		// printf("test T: %d\n", t);
		while (!stop)
		{
			while (t >= i)
			{
				if (((j = read(sockfd, message, sizeof(message))) > 0))
				{
					printf("%d: %s", channel, message);
				}
				i++;
			}
			if (h == 0)
			{
				read(sockfd, message, sizeof(message));
				printf("%s\n", message);
				h++;
			}
		}
		write(sockfd, "1", 1);
	}
}

void livefeed_all(int sockfd)
{
	int sub_channel[255] = {0};
	char **messages = malloc(sizeof(char *) * 50);
	int counter =0;
	for (int i = 0; i < 2; i++)
	{
		read(sockfd, sub_channel, sizeof(sub_channel));
		read(sockfd, messages, sizeof(messages));
	}
	// filtering out the and printf the data
	for (int i = 0; i < sizeof(sub_channel); i++)
	{
		if (sub_channel[i] == 1)
		{
			counter++;
		}
	}
	if (counter == 0)
	{
		printf("Not subscribed to any channels");
	}
	else
	{
		printf("This is LIVEFEED. Please press CTRL + C to back to menu.\nAll unread messages of all channels that you subscribed to are below:\n");
		for (int i = 0; i < sizeof(sub_channel); i++)
		{
			if (sub_channel[i] == 1)
			{
				counter++;
				printf("Channel %d\n", i);
				for (int j =0; j < sizeof(messages); j++)
				{
					printf("%d: %s", i, *messages[j]);
				}
			}
		}
	}
}

void send_message(int sockfd, char *channel, char message[])
{
	// Clean buffer
	int32_t tmp = 0;
	tmp = atoi(channel);
	// Check if input is within the range of channel id
	if (tmp < 0 || tmp > 255)
	{
		printf("Invalid channel: %d\n", atoi(channel));
	}
	else
	{
		// Send message to server
		if (strlen(message) > 1024 + 4)
			printf("Warning: Maximum length of message is 1024. Only 1024 characters will be sent this time \n");
		write(sockfd, message, 1024 + 4);
		// Reading message just sent
		read(sockfd, message, 28);
		printf("Server: %s\n", message);
	}
}

void subscribe(int sockfd)
{
	int32_t tmp;
	int status;

	bzero(&tmp, sizeof(tmp));
	printf("Enter channel to subscribe : ");
	scanf("%d", &input);

	tmp = input;

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
		printf("Channel already subscribed\n");
	}
	else if (status == 2)
	{
		printf("Channel range 0 to 255 only\n");
	}
	else
	{
	}
}

void unsubscribe(int sockfd)
{

	int32_t tmp;
	int status;

	bzero(&tmp, sizeof(tmp));
	printf("Enter channel to unsubscribe : ");
	scanf("%d", &input);

	tmp = input;

	write(sockfd, &tmp, sizeof(tmp));
	bzero(&tmp, sizeof(tmp));

	read(sockfd, &tmp, sizeof(tmp));
	status = (int)tmp;

	if (status == 0)
	{
		printf("Unsubscribed to channel %d\n", input);
	}
	else if (status == 1)
	{
		printf("Channel not subscribed\n");
	}
	else if (status == 2)
	{
		printf("Channel range 0 to 255 only\n");
	}
	else
	{
	}
}

int main(int argc, char *argv[])
{
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
