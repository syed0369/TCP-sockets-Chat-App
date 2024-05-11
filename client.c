#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define MAX_NAME 32

struct sockaddr_in* createaddr(char* ip, int port);
void* listenandprint(void* fd);

int main()
{
    char* line;
    char* name;
    char namecopy[MAX_NAME];
    pthread_t tid;
    int socketfd, ret;
    ssize_t charcount, namecount;
    struct sockaddr_in* address;
    
    line = NULL;
    charcount = 0;
    name = NULL;
    namecount = 0;
    
    socketfd = socket(AF_INET, SOCK_STREAM, 0);
    address = createaddr("127.0.0.1", 2000);
    ret = connect(socketfd, (struct sockaddr*)address, sizeof(*address));
    if(ret == 0) {
        printf("Connection successful\n");
    }
    printf("Enter your name\n");
    namecount = getline(&name, &namecount, stdin);
    if(namecount == 0) {
        printf("No name entered\n");
        return 1;
    }
    name[namecount - 1] = ':';
    strcpy(namecopy, name);
    pthread_create(&tid, NULL, listenandprint, (void*)&socketfd);
    printf("Type message\n");
    while(1) {
        charcount = getline(&line, &charcount, stdin);
        fflush(stdin);
        
        if(charcount > 0) {
            if(strcmp(line, "exit\n") == 0) 
                break;
            strcat(name, line);
            send(socketfd, name, strlen(name), 0);
        }
        strcpy(name, namecopy);
    }
    close(socketfd);
    return 0;
}

struct sockaddr_in* createaddr(char* ip, int port) {
    struct sockaddr_in* address = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));
    address -> sin_family = AF_INET;
    address -> sin_port = htons(port);
    inet_pton(AF_INET, ip, &address -> sin_addr.s_addr);
    return address;
}

void* listenandprint(void* fd) {
    int* socketfd = (int*)fd;
    char message[1024];
    ssize_t charcount;
    while(1) {
        charcount = recv(*socketfd, message, 1024, 0);
        message[charcount] = '\0';
        if(charcount == 0)
            break;
        printf("%s", message);      
    }
    close(*socketfd);
}
 
