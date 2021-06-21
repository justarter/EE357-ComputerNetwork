#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT "3490" // the port client will be connecting to 
#define MAXDATASIZE 100 // max number of bytes we can get at once 

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
	int listener, numbytes;  
	char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

    struct timeval tv;
	fd_set readfds;
	int select_rv, fdmax; 
    socklen_t nbytes;
    
    printf("Connect to host1 by default\n");
	// if (argc != 2) {//input server IP
    //     fprintf(stderr,"usage: client hostname\n");
    //     exit(1);
    // }

    // get us a socket
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo("10.0.0.1", PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((listener = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(listener, p->ai_addr, p->ai_addrlen) == -1) {
            close(listener);
			perror("client: connect");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

    //get the server's IP
	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);
	printf("client: connecting to %s\n", s);
	freeaddrinfo(servinfo); // all done with this structure
    
	for(;;){
        FD_ZERO(&readfds);
        fdmax = 0;

        FD_SET(0, &readfds);//own descriptor
        FD_SET(listener, &readfds);

        if (fdmax < listener){
            fdmax = listener;
        }
        tv.tv_sec = 2;
        tv.tv_usec = 500000;
        select_rv = select(fdmax+1, &readfds, NULL, NULL, &tv);
        if(select_rv == -1){
            perror("select");
            break;
        }else{
            if(select_rv == 0){
                continue;
            }else{
                //get data
                if(FD_ISSET(listener, &readfds)){
                    bzero(buf, MAXDATASIZE);//clear buffer
                    if(nbytes = recv(listener, buf, MAXDATASIZE, 0) > 0){
                        printf("Receive Message：%s\n", buf);
                    }else{
                        if(nbytes < 0){
                            printf("Receive message failed!\n");
                        }else{
                            printf("server exited, connection finished!\n");
                        }
                        break; 
                    }
                }
                //client input and send data
                if(FD_ISSET(0, &readfds)){ 
                    //send data
                    bzero(buf, MAXDATASIZE);
                    fgets(buf, MAXDATASIZE, stdin);//input
               
                    if(!strncasecmp(buf, "exit", 4)){//exit
                        printf("Client request to exit!\n");
                        break;
                    }
                    if(send(listener, buf, strlen(buf), 0) > 0){
                        printf("Meaasge send successfully：%s\n",buf); 
                    }else{
                        printf("Send message failed!\n");
                        break; 
                    } 
                }
            }
        } 
    }
    //close
    close(listener);

	return 0;
}

