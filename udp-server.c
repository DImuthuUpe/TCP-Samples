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
   
#define PORT     8888
#define MAXLINE 1024 * 32
#define RINGSIZE 64

int sockfd;
struct sockaddr_in servaddr, cliaddr;
char *hello = "Hello from server";

volatile unsigned long total, missing_packets = 0;
volatile unsigned long seq_num, prev_seq_num = 0;

pthread_rwlock_t total_rwlock;

struct RingBuffer {
    char ring[RINGSIZE][MAXLINE];
    int current_pos;
    unsigned long current_seq_num;
    int ring_status[RINGSIZE];
};

struct Nack {
    unsigned long seq_number;
    unsigned long offset;
    char confirmed;
    struct Nack *next;
};



int get_next_ring_pos(int current_pos, unsigned long current_seq_num, unsigned long next_seq_num) {
    if (current_pos < RINGSIZE -1) 
    {
        return current_pos + 1;
    } else 
    {
        return 0;
    }
    
}

void *controller_thread(void *vargp)
{
    char buffer[4];
    unsigned long speed = 12;
    unsigned long prev_total = 0;
    while(1)
    {

        int len = sizeof(cliaddr);
        sleep(1);
        printf("Printing from Controller Thread \n");

        pthread_rwlock_rdlock(&total_rwlock);
        speed = (total - prev_total)/(128 * 1024);
        prev_total = total;
        pthread_rwlock_unlock(&total_rwlock);


        memcpy(buffer, &speed, 4);
        sendto(sockfd, buffer, 4, 
        0, (const struct sockaddr *) &cliaddr, len);
    }
    return NULL;
}

void monitor_thread()
{
    struct timeval t;
    gettimeofday(&t, NULL);
    unsigned long prev_t = t.tv_sec * 1000000ULL + t.tv_usec;
    unsigned long current_t = t.tv_sec * 1000000ULL + t.tv_usec;

    unsigned long prev_missing_packets = 0;
    unsigned long prev_total = 0;
    printf("Clocks per s %lu \n", CLOCKS_PER_SEC);
    printf("Start clock %lu \n", prev_t);

    while(1) {

        usleep(200000);
        gettimeofday(&t, NULL);
        current_t = t.tv_sec * 1000000ULL + t.tv_usec;
        
        double times = (current_t - prev_t) * 1.0/ 1000000ULL;
        
        pthread_rwlock_rdlock(&total_rwlock);
        printf("Total %lu Time %lf Seq %lu PrevSeq %lu Speed %lf Mbit/s Missing Rate %lf\n", total, times, seq_num, prev_seq_num, (total - prev_total)/(times * 128 * 1024), (missing_packets - prev_missing_packets)/(times));
        //printf("%lu %lu %lu %lu\n", total, times, seq_num, prev_seq_num);
        prev_t = current_t;
        prev_total = total;
        prev_missing_packets = missing_packets;
        pthread_rwlock_unlock(&total_rwlock);
    
    }
}


// Driver code
int main() {
    
    struct RingBuffer ring_buffer;

    int i;
    for (i = 0; i < RINGSIZE; i++) {
        ring_buffer.ring_status[i] = 1;
    }
    ring_buffer.current_pos = 0;


    pthread_rwlock_init(&total_rwlock, NULL);

    char buffer[MAXLINE];
    
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
   
    len = sizeof(cliaddr);  //len is value/resuslt
    n = recvfrom(sockfd, (char *)buffer, MAXLINE, 
                MSG_WAITALL, ( struct sockaddr *) &cliaddr,
                &len);
        total += n;
    
    memcpy(&seq_num, buffer, 4);
    ring_buffer.current_seq_num = seq_num;

    pthread_t controller_thread_id;
    pthread_create(&controller_thread_id, NULL, controller_thread, NULL);

    pthread_t monitor_thread_id;
    pthread_create(&monitor_thread_id, NULL, monitor_thread, NULL);

    while(1)
    {

        n = recvfrom(sockfd, (char *)buffer, MAXLINE, 
                MSG_WAITALL, ( struct sockaddr *) &cliaddr,
                &len);

        pthread_rwlock_wrlock(&total_rwlock);
        memcpy(&seq_num, buffer, 4);
        missing_packets += seq_num - prev_seq_num - 1;
        total += n;
        pthread_rwlock_unlock(&total_rwlock);
        prev_seq_num = seq_num;
    }

    printf("Total message received %lu \n", total);
       
    return 0;
}
