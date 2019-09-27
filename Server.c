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

	#define DEFAULT_PORT 12345    /* the port users will be connecting to */

	#define BACKLOG 10     /* how many pending connections queue will hold */
	/* Golbal variables */
	int sockfd, new_fd, port;  /* listen on sock_fd, new connection on new_fd */
	struct sockaddr_in my_addr;    /* my address information */
	struct sockaddr_in their_addr; /* connector's address information */
	socklen_t sin_size;

void shutdown_server(int sig){
	close(sockfd);
	exit(0);
}

int main(int argc, char *argv[])
{
	// Variables
	int clientID = 0;
	char welcomeMessage[] = "";

	signal(SIGINT, shutdown_server);	/* Let program exit when ctrl + c is pressed */

//	Check port number if not given use default port 12345
	if (argc < 2) {
		printf("No port provided. Using default port %d\n", DEFAULT_PORT);
		port = DEFAULT_PORT;
	}
	else if (argc == 2){
		port = atoi(argv[1]);
	} else {
		fprintf(stderr, "Usage: %s [PORT]", argv[0]);
		exit(1);
	}

	/* generate the socket */
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	/* generate the end point */
	my_addr.sin_family = AF_INET;         /* host byte order */
	my_addr.sin_port = htons(port);     /* short, network byte order */
	my_addr.sin_addr.s_addr = INADDR_ANY; /* auto-fill with my IP */
		/* bzero(&(my_addr.sin_zero), 8);   ZJL*/     /* zero the rest of the struct */

	/* bind the socket to the end point */
	if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) \
	== -1) {
		perror("bind");
		exit(1);
	}

	/* start listnening */
	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	printf("Server starts listening on port %d...\n", port);

	/* repeat: accept, send, close the connection */
	/* for every accepted connection, use a sepetate process or thread to serve it */
	while(1) {  /* main accept() loop */
		sin_size = sizeof(struct sockaddr_in);
		if ((new_fd = accept(sockfd, (struct sockaddr *)&their_addr, \
		&sin_size)) == -1) {
			perror("accept");
			continue;
		}

		printf("server: got connection from %s\n", \
			inet_ntoa(their_addr.sin_addr));


		if (!fork()) { /* this is the child process */
			if (send(new_fd, "Welcome! Your client ID is \n", 28, 0) == -1)
				perror("send");
			close(new_fd);
			exit(0);
		}
		close(new_fd);  /* parent doesn't need this */

		while(waitpid(-1,NULL,WNOHANG) > 0); /* clean up child processes */
	}
}
