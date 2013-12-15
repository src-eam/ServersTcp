#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 2013
#define MAX_BUFFER_SIZE 1024
#define MAX_CLIENTS 30

int main(int argc, char *argv[]) {
	fd_set readfds;
	fd_set master;
	char buffer[MAX_BUFFER_SIZE];
	struct sockaddr_in serveraddr;
	struct sockaddr_in clientaddr;
	int addrlen = sizeof(clientaddr);
	int listener, fdmax, newfd, nbytes;
	if ((listener = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Error Server-socket().");
		exit(1);
	}
	printf("Server-socket() is OK...\n");
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = INADDR_ANY;
	serveraddr.sin_port = htons(PORT);
	if (bind(listener, (struct sockaddr *) &serveraddr, sizeof(serveraddr))
			== -1) {
		perror("Error Server-bind().");
		exit(1);
	}
	printf("Server-bind() is OK...\n");
	if (listen(listener, MAX_CLIENTS) == -1) {
		perror("Error Server-listen().");
		exit(1);
	}
	printf("Server-listen() is OK...\n");
	FD_SET(listener, &master);
	fdmax = listener;
	for (;;) {
		readfds = master;
		if (select(fdmax + 1, &readfds, NULL, NULL, NULL) == -1) {
			perror("Error Server-select().");
			exit(1);
		}
		for (int i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &readfds)) {
				if (i == listener) {
					if ((newfd = accept(listener,
							(struct sockaddr *) &clientaddr,
							(socklen_t*) &addrlen)) < 0) {
						perror("Error Server-accept().");
					} else {
						FD_SET(newfd, &master);
						if (newfd > fdmax) {
							fdmax = newfd;
						}
						printf("Server: New connection from %s on socket %d\n",
								 inet_ntoa(clientaddr.sin_addr), newfd);
					}
				} else {
					if ((nbytes = recv(i, buffer, sizeof(buffer), 0)) <= 0) {
						if (nbytes == 0)
							printf("Server: socket %d out\n", i);
						close(i);
						FD_CLR(i, &master);
					} else {
						if(nbytes > MAX_BUFFER_SIZE)
							nbytes = MAX_BUFFER_SIZE;
						buffer[nbytes] = '\0';
						printf("socket %d: %s\n",i, buffer);
						send(i, buffer, nbytes, 0);
					}
				}
			}
		}
	}
	return 0;
}
