#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>

#define MAX_CLIENTS 1000

struct acceptSocket{
    int acceptedfd;
    struct sockaddr_in address;
    int success;
};
struct acceptSocket clients[MAX_CLIENTS];
int clientscount = 0;
void* recvandprint(void* clientfd);
void startconnecting(int socketfd);
struct sockaddr_in* createaddr(char* ip, int port);
struct acceptSocket* acceptconnection(int socketfd);

int main()
{
    struct sockaddr_in* address;
    int socketfd, ret1;
    socketfd = socket(AF_INET, SOCK_STREAM, 0);
    address = createaddr("", 2000);
    ret1 = bind(socketfd, (struct sockaddr*)address, sizeof(*address));
    if(ret1 == 0) {
        printf("Socket bound successfully\n");
    }
    listen(socketfd, 10);
    startconnecting(socketfd);
    shutdown(socketfd, SHUT_RDWR);
    return 0;
}

struct sockaddr_in* createaddr(char* ip, int port) {
    struct sockaddr_in* address = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));
    address -> sin_family = AF_INET;
    address -> sin_port = htons(port); 
    address -> sin_addr.s_addr = INADDR_ANY;
    return address;
}

void startconnecting(int socketfd) {
    struct acceptSocket* clientsocket;
    while(1) {
        clientsocket = acceptconnection(socketfd);
        pthread_t tid;
        clients[clientscount++] = *clientsocket;
        pthread_create(&tid, NULL, recvandprint, (void*) &clientsocket -> acceptedfd);       
    }
}

struct acceptSocket* acceptconnection(int socketfd) {
    int sizesockaddr, clientfd;
    struct sockaddr_in clientaddr;
    struct acceptSocket* clientsocket;
    clientsocket = (struct acceptSocket*)malloc(sizeof(struct acceptSocket));
    sizesockaddr = sizeof(struct sockaddr_in);
    clientfd = accept(socketfd, (struct sockaddr*)&clientaddr, &sizesockaddr);
    clientsocket -> acceptedfd = clientfd;
    clientsocket -> address = clientaddr;
    clientsocket -> success = clientfd > 0;
    return clientsocket;
}

void* recvandprint(void* fd) {
    int i, outputfd;
    char message[1024];
    ssize_t charcount;
    int* clientfd = (int*) fd;
    outputfd = open("messages.txt", O_APPEND | O_RDWR | O_CREAT, 0644);
    while(1) {
        charcount = recv(*clientfd, message, 1024, 0);
        if(charcount == 0)
            break;
        message[charcount] = 0;
        for(i = 0; i < clientscount; i++) {
            if(clients[i].acceptedfd != *clientfd)
                send(clients[i].acceptedfd, message, 1024, 0);    
        }
        write(outputfd, message, (ssize_t)strlen(message));
        printf("%s", message);      
    }
    close(outputfd);
    close(*clientfd);
}

