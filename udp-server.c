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
#define MAXLINE 1024 * 32
   
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
    unsigned long total, prev_total = 0;
   
    len = sizeof(cliaddr);  //len is value/resuslt
    n = recvfrom(sockfd, (char *)buffer, MAXLINE, 
                MSG_WAITALL, ( struct sockaddr *) &cliaddr,
                &len);
        total += n;
    

    struct timeval t;
    gettimeofday(&t, NULL);
    unsigned long prev_t = t.tv_sec * 1000000ULL + t.tv_usec;
    unsigned long current_t = t.tv_sec * 1000000ULL + t.tv_usec;

    printf("Clocks per s %lu \n", CLOCKS_PER_SEC);
    printf("Start clock %lu \n", prev_t);

    unsigned long seq_num, prev_seq_num, missing_packets, prev_missing_packets = 0;
    unsigned long step = 0;
    while(1)
    {

        gettimeofday(&t, NULL);
        step++;
        n = recvfrom(sockfd, (char *)buffer, MAXLINE, 
                MSG_WAITALL, ( struct sockaddr *) &cliaddr,
                &len);
        memcpy(&seq_num, buffer, 4);
        missing_packets += seq_num - prev_seq_num - 1;

        total += n;
        
        current_t = t.tv_sec * 1000000ULL + t.tv_usec;

        
        if ((current_t - prev_t)*1000/(CLOCKS_PER_SEC) > 100) {
            
            double times = (current_t - prev_t) * 1.0/ 1000000ULL;
            
            printf("Total %lu Time %lf Seq %lu PrevSeq %lu Speed %lf Mbit/s Missing Rate %lf\n", total, times, seq_num, prev_seq_num, (total - prev_total)/(times * 128 * 1024), (missing_packets - prev_missing_packets)/(times));
            //printf("%lu %lu %lu %lu\n", total, times, seq_num, prev_seq_num);
            prev_t = current_t;
            prev_total = total;
            prev_missing_packets = missing_packets;
        }
        
        prev_seq_num = seq_num;
    }

    printf("Total message received %lu \n", total);
    
    
    sendto(sockfd, (const char *)hello, strlen(hello), 
        0, (const struct sockaddr *) &cliaddr,
            len);
    //printf("Total %lu \n", total);
    printf("Hello message sent.\n"); 
       
    return 0;
}
