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
#define MAXLINE 1024 * 32

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
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	
	int n, len;
	int i;

    long delay_ticks = 2;
    struct timeval t;

    gettimeofday(&t, NULL);
    unsigned long start_t = t.tv_sec * 1000000ULL + t.tv_usec;
    unsigned long current_t = t.tv_sec * 1000000ULL + t.tv_usec;
    unsigned long prev_t = t.tv_sec * 1000000ULL + t.tv_usec;

	unsigned long total, prev_total = 0;
	unsigned long seq_num = 1;

    while (1) 
    {
		gettimeofday(&t, NULL);
		current_t = t.tv_sec * 1000000ULL + t.tv_usec;

		if (current_t > start_t + delay_ticks) 
		{
			memcpy(buffer, &seq_num, 4);
			sendto(sockfd, buffer, MAXLINE, 0, (const struct sockaddr *) &servaddr, sizeof(servaddr));
			seq_num++;
			total += MAXLINE;
			start_t = current_t;
		}
		
		if ((current_t - prev_t)*1000/(CLOCKS_PER_SEC) > 100) {
            
            double times = (current_t - prev_t) * 1.0/ 1000000ULL;
            
            printf("Total %lu Time %lf Seq %lu Speed %lf Mbit/s\n", total, times, seq_num, (total - prev_total)/(times * 128 * 1024));
            //printf("%lu %lu %lu %lu\n", total, times, seq_num, prev_seq_num);
            prev_t = current_t;
            prev_total = total;
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
