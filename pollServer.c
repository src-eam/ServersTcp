#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 2013
#define MAX_BUFFER_SIZE 1024
#define MAX_CLIENTS 30

int main(int argc, char *argv[]) {
	char buffer[MAX_BUFFER_SIZE];
	struct pollfd fds[MAX_CLIENTS];
	struct sockaddr_in serveraddr, clientaddr;
	int addrlen = sizeof(clientaddr);
	int listener, fdmax, newfd,nbytes;
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
	fdmax = 1;
	fds[0].fd = listener;
	fds[0].events = POLLIN;
	for (;;) {
		if (poll(fds, fdmax, -1) == -1) {
			perror("Error Server-poll().");
			exit(1);
		}
		for (int i = 0; i < fdmax; i++) {
			if (fds[i].revents != 0) {
				if (fds[i].fd == listener) {
					if ((newfd = accept(listener,
							(struct sockaddr *) &clientaddr,
							(socklen_t*) &addrlen)) < 0) {
						perror("Error Server-accept().");
					} else {
						if (newfd > fdmax) {
							fdmax = newfd;
						}
						fds[newfd].fd = newfd;
						fds[newfd].events = POLLIN;
						fds[newfd].revents = 0;
						fdmax++;
						printf("Server: New connection from %s on socket %d\n",
								inet_ntoa(clientaddr.sin_addr), newfd);
					}
				} else if(fds[i].revents & POLLIN){
					if ((nbytes = recv(fds[i].fd, buffer, sizeof(buffer), 0)) <= 0) {
						if (nbytes == 0)
							printf("Server: socket %d out\n", fds[i].fd);
						close(fds[i].fd);
					} else {
						if (nbytes > MAX_BUFFER_SIZE)
							nbytes = MAX_BUFFER_SIZE;
						buffer[nbytes] = '\0';
						printf("socket %d: %s\n", fds[i].fd, buffer);
						send(fds[i].fd, buffer, nbytes, 0);
					}
				}
			}
		}
	}
	return 0;
}
