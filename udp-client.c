// Client side implementation of UDP client-server model
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

#define PORT	 8888
#define MAXLINE 1024

// Driver code
int main() {
	int sockfd;
	char buffer[MAXLINE];
	
	struct sockaddr_in	 servaddr;

	// Creating socket file descriptor
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}

	memset(&servaddr, 0, sizeof(servaddr));
	
	// Filling server information
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);
	servaddr.sin_addr.s_addr = INADDR_ANY;
	
	int n, len;
	int i;

    long delay_ticks = 1;
    struct timeval t;

    gettimeofday(&t, NULL);
    unsigned long start_t = t.tv_sec * 1000000ULL + t.tv_usec;
    unsigned long current_t = t.tv_sec * 1000000ULL + t.tv_usec;
    unsigned long perf_t = t.tv_sec * 1000000ULL + t.tv_usec;

    long total = 0;

    for (i = 0; i < 10000000; i++) 
    {
	    sendto(sockfd, buffer, MAXLINE, 0, (const struct sockaddr *) &servaddr, sizeof(servaddr));
        total += MAXLINE;
        gettimeofday(&t, NULL);
        current_t = t.tv_sec * 1000000ULL + t.tv_usec;

        if (perf_t + 1000000ULL < t.tv_sec * 1000000ULL + t.tv_usec) {
            perf_t = t.tv_sec * 1000000ULL + t.tv_usec;
            unsigned long times = (perf_t - start_t)/ 1000000ULL;
            printf("Total %lu Time %lu Speed %lu MB/s\n", total, times, total/(times * 1000000));
        }

        while(t.tv_sec * 1000000ULL + t.tv_usec - current_t < delay_ticks) {
            gettimeofday(&t, NULL);
        }
    }
    
    printf("Hello message sent.\n");
		
	n = recvfrom(sockfd, (char *)buffer, MAXLINE,
				MSG_WAITALL, (struct sockaddr *) &servaddr,
				&len);
	buffer[n] = '\0';
	printf("Server : %s\n", buffer);

	close(sockfd);
	return 0;
}
