#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <sys/ioctl.h>
#include <net/if.h>

#define SERVERPORT 3490 // the port users will be connecting to
#define PORT "3490"
#define MAXDATASIZE 100 // max number of bytes we can get at once 
#define HOSTNAME "10.255.255.255" //broadcast addr

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
    int sockfd, listener;
    struct sockaddr_in their_addr; // connector's address information
    struct hostent *he;
    int nbytes;
    int broadcast = 1;
    struct sockaddr_storage server_addr; //store the addr of sender
    char buf[MAXDATASIZE];
    //char broadcast = '1'; // if that doesn't work, try this
    struct addrinfo hints, *p, *servinfo;

    char s[INET6_ADDRSTRLEN], rv_addr[INET6_ADDRSTRLEN], local_addr[INET6_ADDRSTRLEN];
    int yes=1;        // for setsockopt() SO_REUSEADDR, below
    struct timeval tv;
	fd_set readfds;
	int rv, select_rv, fdmax; 
    socklen_t addr_len;
    
    if ((he=gethostbyname(HOSTNAME)) == NULL) {  // get the host info
        perror("gethostbyname");
        exit(1);
    }

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

	int local_sockfd;
	struct ifconf ifconf;
	struct ifreq *ifreq;
	char tmp_buf[1024];
    char* local_ip;
	//初始化ifconf
	ifconf.ifc_len = 1024;
	ifconf.ifc_buf = tmp_buf;
 
	if((local_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket error");
		exit(1);
	}
 
	//获取所有接口信息
	ioctl(local_sockfd, SIOCGIFCONF, &ifconf);
 
	//逐个获取Ip地址
	ifreq = (struct ifreq*)tmp_buf;
	for(int i = (ifconf.ifc_len/sizeof(struct ifreq)); i>0; i--)
	{
		// printf("name = [%s] : ",ifreq->ifr_name);
		local_ip=inet_ntoa( ((struct sockaddr_in *)&(ifreq->ifr_addr))->sin_addr);
		ifreq++;
    }
    strcpy(local_addr, local_ip);
    close(local_sockfd);

    // this call is what allows broadcast packets to be sent:
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast,
        sizeof broadcast) == -1) {
        perror("setsockopt (SO_BROADCAST)");
        exit(1);
    }

    their_addr.sin_family = AF_INET;     // host byte order
    their_addr.sin_port = htons(SERVERPORT); // short, network byte order
    their_addr.sin_addr = *((struct in_addr *)he->h_addr);
    memset(their_addr.sin_zero, '\0', sizeof their_addr.sin_zero);
    addr_len = sizeof their_addr;

    // get us a socket
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		exit(1);
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((listener = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}
        if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
            perror("setsockopt");
            exit(2);
        }

        //bind to the IP of host
        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
            close(listener);
            continue;
        }
        
		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

    //get the server's IP
	inet_ntop(their_addr.sin_family, get_in_addr((struct sockaddr *)&their_addr),
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

                    if((nbytes = recvfrom(listener, buf, MAXDATASIZE-1, 0, (struct sockaddr *)&server_addr, &addr_len)) > 0){
                        // printf("%d",nbytes);
                        inet_ntop(server_addr.ss_family,
                                get_in_addr((struct sockaddr *)&server_addr),
                                rv_addr, sizeof rv_addr);
                        
                        if(strcmp(rv_addr, local_addr)){
                            buf[nbytes] = '\0';
                            printf("Receive Message：%s which is %d bytes from %s\n", buf,nbytes, rv_addr);
                        }
                    }else{
                        if(nbytes < 0){
                            perror("recvfrom");
                        }else{
                            printf("server exited, connection finished!\n");
                        }
                        break; 
                    }
                }
                if(FD_ISSET(0, &readfds)){
                    bzero(buf, MAXDATASIZE);
                    fgets(buf, MAXDATASIZE, stdin);
                    if ((nbytes=sendto(sockfd, buf, strlen(buf), 0,
                            (struct sockaddr *)&their_addr, sizeof their_addr)) == -1) {
                        perror("sendto");
                        exit(1);
                    }
                    printf("sent %d bytes to %s\n", nbytes, inet_ntoa(their_addr.sin_addr));
                }
            }
        }
    }
    close(listener);
    close(sockfd);

    return 0;
}