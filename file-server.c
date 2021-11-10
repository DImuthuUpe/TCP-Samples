#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <time.h>
#define PORT 8080

int main(int argc, char const *argv[])
{
    int server_fd, new_socket;
    unsigned long valread = 0;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024 * 1024] = {0};
    char *hello = "Hello from server";
    FILE *fp;
    char *filename = "recv.txt";
    fp = fopen(filename, "w");
       
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
       
    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );
       
    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    int i = 0;
    clock_t t;
    t = clock();

    while(1)
    { 
        valread += read( new_socket , buffer, 1024 * 1024);
        fwrite(buffer, sizeof(char), valread, fp);
        if (valread >= 1024 * 1024 * 1024 * 1l) 
        {
            break;
        }
    }

    //printf("%s\n",buffer );
    t = clock() - t;
    double time_taken = ((double)t)/CLOCKS_PER_SEC; // calculate the elapsed time
    printf("The program took %f seconds to execute \n", time_taken);
    
    printf("Read GB %lu \n", valread / (1024 * 1024 * 1024));
    printf("Speed %f GB/s \n", valread / (1024 * 1024 * 1024 * time_taken));
    send(new_socket , hello , strlen(hello) , 0 );
    return 0;
}