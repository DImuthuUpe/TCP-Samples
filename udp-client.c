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
#include <unistd.h>
#include <pthread.h>

#define PORT	 8888

unsigned long MAXLINE = 1024 * 48 - 1;
int sockfd;
struct sockaddr_in	 servaddr;
volatile unsigned long server_speed = 0;
volatile unsigned long ts = 1;
pthread_rwlock_t srv_speed_rwlock;
pthread_rwlock_t monitor_rwlock;

volatile unsigned long total = 0;
volatile unsigned long seq_num = 1;


void *controller_thread(void *vargp)
{
	char buffer[MAXLINE];
	int len, n;
	
    while(1)
    {
        n = recvfrom(sockfd, (char *)buffer, MAXLINE,
				MSG_WAITALL, (struct sockaddr *) &servaddr,
				&len);

		pthread_rwlock_wrlock(&srv_speed_rwlock);

        memcpy(&server_speed, buffer, 4);
		// Calulate the expected colck cycle delay to next packet send
		if (server_speed > 0) {
			ts = (MAXLINE * CLOCKS_PER_SEC) / (1024 * 128 * server_speed);
		} else {
			ts = 200;
		}
		// Add bias to the clock delay to avoid badnwidth shrinking
		if (ts > 1) {
			ts = ts - 1;
		}

		pthread_rwlock_unlock(&srv_speed_rwlock);
		

		printf("Server speed : %lu ts %lu\n", server_speed, ts);
    }
    return NULL;
}

void monitor_thread() 
{
    
	struct timeval t;
	gettimeofday(&t, NULL);
	unsigned long current_t = t.tv_sec * 1000000ULL + t.tv_usec;
    unsigned long prev_t = t.tv_sec * 1000000ULL + t.tv_usec;
	unsigned long prev_total = 0;

	while(1)
	{
		sleep(1);
		gettimeofday(&t, NULL);
		current_t = t.tv_sec * 1000000ULL + t.tv_usec;
		
		double times = (current_t - prev_t) * 1.0/ 1000000ULL;
		pthread_rwlock_rdlock(&monitor_rwlock);
		printf("Total %lu Time %lf Seq %lu Speed %lf Mbit/s\n", total, times, seq_num, (total - prev_total)/(times * 128 * 1024));
		//printf("%lu %lu %lu %lu\n", total, times, seq_num, prev_seq_num);
		prev_t = current_t;
		prev_total = total;
		pthread_rwlock_unlock(&monitor_rwlock);
	}   
}

// Driver code
int main() 
{
	
	pthread_rwlock_init(&srv_speed_rwlock, NULL);

	char buffer[MAXLINE];

	// Creating socket file descriptor
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) 
	{
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

    unsigned long delay_ticks = 2;
    struct timeval t;

    gettimeofday(&t, NULL);
    unsigned long start_t = t.tv_sec * 1000000ULL + t.tv_usec;
    unsigned long current_t = t.tv_sec * 1000000ULL + t.tv_usec;

	pthread_t thread_id;
    pthread_create(&thread_id, NULL, controller_thread, NULL);

	pthread_t monitor_thread_id;
    pthread_create(&monitor_thread_id, NULL, monitor_thread, NULL);

	socklen_t sock_len = sizeof(servaddr);
	struct sockaddr *addr = (const struct sockaddr *) &servaddr;

	int sendbuf_size = 65525;
	setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &sendbuf_size, sock_len);

    while (1) 
    {
		gettimeofday(&t, NULL);
		current_t = t.tv_sec * 1000000ULL + t.tv_usec;

		if (current_t > start_t + delay_ticks) 
		{
			pthread_rwlock_rdlock(&srv_speed_rwlock);
			
			delay_ticks = ts;

			pthread_rwlock_unlock(&srv_speed_rwlock);
			memcpy(buffer, &seq_num, 4);
			sendto(sockfd, buffer, MAXLINE, 0, addr, sock_len);
			

			pthread_rwlock_wrlock(&monitor_rwlock);
			seq_num++;
			total += MAXLINE;
			pthread_rwlock_unlock(&monitor_rwlock);
			
			start_t = current_t;
		}
    }
    
	close(sockfd);
	return 0;
}
