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
void subscribe();

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
		printf("%s", buff);

		if ((strncmp(buff, "sub", 3)) == 0)
		{
			printf("SUB process\n");
			/*	Unscribes all channels*/
			// Clean buffer
			bzero(buff, sizeof(buff));
			// Send selected commnand to server
			subscribe(new_fd);

			break;
		}

		// if ((strncmp(buff, "bye", 3)) == 0)
		// {
		// 	printf("Client Exit...\n");
		// 	/*	Unscribes all channels*/
		// 	break;
		// }
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
	int sockfd, numbytes, port;
	char buf[MAXDATASIZE];
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

	if ((numbytes = recv(sockfd, buf, MAXDATASIZE, 0)) == -1)
	{
		perror("recv");
		exit(1);
	}

	buf[numbytes] = '\0';

	printf("Received: %s", buf);

	loop_listen(sockfd);

	close(sockfd);

	return 0;
}
