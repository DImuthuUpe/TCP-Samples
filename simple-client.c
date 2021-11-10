#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#define PORT 8080
#define BUFSIZE 1024 * 1024 * 2

EVP_CIPHER_CTX *ctx;
int encrypt(char *orig, char *encrypted, int txt_len, char *iv, char *key) {
    

    int len;

    int ciphertext_len;
    
    EVP_EncryptUpdate(ctx, encrypted, &len, orig, txt_len);
    ciphertext_len = len;
    EVP_EncryptFinal_ex(ctx, encrypted + len, &len);

    return ciphertext_len;
}

int main(int argc, char const *argv[])
{
    ctx = EVP_CIPHER_CTX_new();
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char *hello = "Hello from client";
    char buffer[BUFSIZE] = {0};
    char encrypted[BUFSIZE] = {2};

    unsigned char key[] = {0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xAA,0xBB,0xCC,0xDD,0xEE,0xFF};
    unsigned char *iv = (unsigned char *)"0123456789012345";

    EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, iv);

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }
   
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
       
    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) 
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
   
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }

    int i = 0;
    for(i = 0; i < 1024 * 1024 * 1; i++) { 
        encrypt(buffer, encrypted, BUFSIZE, iv, key);
        send(sock , buffer , BUFSIZE , 0 );
    }
    printf("Hello message sent\n");
    valread = read( sock , buffer, 1024);
    printf("%s\n",buffer );

    EVP_CIPHER_CTX_free(ctx);
    return 0;
}