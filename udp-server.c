// Server side implementation of UDP client-server model
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>
#include <sys/time.h>
   
#define PORT     8888
#define MAXLINE 1024 * 1024
   
// Driver code
int main() {
    int sockfd;
    char buffer[MAXLINE];
    char *hello = "Hello from server";
    struct sockaddr_in servaddr, cliaddr;
       
    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
       
    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));
       
    // Filling server information
    servaddr.sin_family    = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);
       
    // Bind the socket with the server address
    if ( bind(sockfd, (const struct sockaddr *)&servaddr, 
            sizeof(servaddr)) < 0 )
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
       
    int len, n;
    long total = 0;
   
    len = sizeof(cliaddr);  //len is value/resuslt
    n = recvfrom(sockfd, (char *)buffer, MAXLINE, 
                MSG_WAITALL, ( struct sockaddr *) &cliaddr,
                &len);
        total += n;
    

    struct timeval t;
    gettimeofday(&t, NULL);
    unsigned long start_t = t.tv_sec * 1000000ULL + t.tv_usec;
    unsigned long current_t = t.tv_sec * 1000000ULL + t.tv_usec;

    printf("Clocks per s %lu \n", CLOCKS_PER_SEC);
    printf("Start clock %lu \n", start_t);

    while(1)
    {
        n = recvfrom(sockfd, (char *)buffer, MAXLINE, 
                MSG_WAITALL, ( struct sockaddr *) &cliaddr,
                &len);
        total += n;
        
        gettimeofday(&t, 0);
        if (current_t + 1000000ULL < t.tv_sec * 1000000ULL + t.tv_usec) {
            current_t = t.tv_sec * 1000000ULL + t.tv_usec;
            unsigned long times = (current_t - start_t)/ 1000000ULL;
            printf("Total %lu Time %lu Speed %lu MB/s\n", total, times, total/(times * 1000000));
        }

        if (total == 1024ULL * 10000000) {
            break;
        }
    }

    printf("Total message received %lu \n", total);
    
    
    sendto(sockfd, (const char *)hello, strlen(hello), 
        0, (const struct sockaddr *) &cliaddr,
            len);
    //printf("Total %lu \n", total);
    printf("Hello message sent.\n"); 
       
    return 0;
}