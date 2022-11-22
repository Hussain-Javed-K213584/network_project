#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdatomic.h>
#include "proto.h"
#include "string.h"
#include "aes.h"
// Global variables
volatile sig_atomic_t flag = 0;
int sockfd = 0;
char nickname[LENGTH_NAME] = {};
FILE* fp1, *fp2;

void catch_ctrl_c_and_exit(int sig) {
    fp1 = fopen("keys.bin", "w+");
    fp2 = fopen("iv.bin", "w+");
    fprintf(fp1, "%s", ""); //destroy data
    fprintf(fp2, "%s", "");
    fclose(fp1);
    fclose(fp2);
    flag = 1;
}
void recv_msg_handler() {
    unsigned char receivedMessage[CIPHER_LEN] = {};
    unsigned char plaintext[CIPHER_LEN] = {};
    unsigned char key[KEY_LEN], iv[IV_LEN];
    while (1) {
        memset(receivedMessage, 0, sizeof(receivedMessage));
        memset(plaintext, 0, sizeof(plaintext));
        memset(key, 0, sizeof(key));
        memset(iv, 0, sizeof(iv));
        again:
        int receive_message = recv(sockfd, receivedMessage, CIPHER_LEN, 0);
        fp1 = fopen("keys.bin", "r+");
        fp2 = fopen("iv.bin", "r+");
        //DMA
        fscanf(fp1, "%s", key);
        fclose(fp1);
        fscanf(fp2, "%s", iv);
        fclose(fp2);
        //And once we recieve it, decrypt it
        /******Decryption starts********/
        int cipher_len = strlen((char*)receivedMessage);
        int plainTextLen = decrypt(receivedMessage, cipher_len, key, iv, plaintext);
        plaintext[plainTextLen] = '\0';
        /******Decryption ends********/
        if (cipher_len == 0)
            goto again;
        if (receive_message > 0) {
            printf("\r<Them>: %s\n", plaintext);
            str_overwrite_stdout();
        } else if (receive_message == 0) {
            break;
        } else { 
            // -1 
        }
    }
}

void send_msg_handler() {
    char message[LENGTH_MSG] = {};
    unsigned char cipherMessage[CIPHER_LEN];
    unsigned char key[KEY_LEN], iv[IV_LEN];
    while (1) {
        memset(message, 0, sizeof(message));
        memset(cipherMessage, 0, sizeof(cipherMessage));
        memset(key, 0, sizeof(key));
        memset(iv, 0, sizeof(iv));
        str_overwrite_stdout();
        while (fgets(message, LENGTH_MSG, stdin) != NULL) {
            str_trim_lf(message, LENGTH_MSG);
            if (strlen(message) == 0) {
                str_overwrite_stdout();
            } else {
                break;
            }
        }
        //Before sending the message we want to encrypt it.
        /******Encryption starts********/
        //Alocate space for key and iv
        //generate random string
        rand_str((unsigned char*)key, 32);
        rand_str((unsigned char*)iv, 16);
        //save key and iv in file
        fp1 = fopen("keys.bin", "w+");
        fprintf(fp1, "%s", key);
        fclose(fp1);
        fp2 = fopen("iv.bin", "w+");
        fprintf(fp2, "%s", iv);
        fclose(fp2);
        int encryptedTextLen = encrypt(message, strlen ((char*) message), key, iv, cipherMessage);
        // printf("%d\n", encryptedTextLen);
        /******Encryption ends here********/
        //free memory
        send(sockfd, cipherMessage, CIPHER_LEN, 0);
        if (strcmp(message, "exit") == 0) {
            break;
        }
    }
    catch_ctrl_c_and_exit(2);
}

int main()
{
    signal(SIGINT, catch_ctrl_c_and_exit);

    // Naming
    printf("Please enter your name: ");
    if (fgets(nickname, LENGTH_NAME, stdin) != NULL) {
        str_trim_lf(nickname, LENGTH_NAME);
    }
    if (strlen(nickname) < 2 || strlen(nickname) >= LENGTH_NAME-1) {
        printf("\nName must be more than one and less than thirty characters.\n");
        exit(EXIT_FAILURE);
    }

    // Create socket
    sockfd = socket(AF_INET , SOCK_STREAM , 0);
    if (sockfd == -1) {
        printf("Fail to create a socket.");
        exit(EXIT_FAILURE);
    }

    // Socket information
    struct sockaddr_in server_info, client_info;
    int s_addrlen = sizeof(server_info);
    int c_addrlen = sizeof(client_info);
    memset(&server_info, 0, s_addrlen);
    memset(&client_info, 0, c_addrlen);
    server_info.sin_family = PF_INET;
    server_info.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_info.sin_port = htons(5000);

    // Connect to Server
    int err = connect(sockfd, (struct sockaddr *)&server_info, s_addrlen);
    if (err == -1) {
        printf("Connection to Server error!\n");
        exit(EXIT_FAILURE);
    }
    
    // Names
    getsockname(sockfd, (struct sockaddr*) &client_info, (socklen_t*) &c_addrlen);
    getpeername(sockfd, (struct sockaddr*) &server_info, (socklen_t*) &s_addrlen);
    printf("Connected to Server: %s:%d\n", inet_ntoa(server_info.sin_addr), ntohs(server_info.sin_port));
    printf("You are: %s\n", nickname);
    //Send name encrypted
    
    send(sockfd, nickname, LENGTH_NAME, 0);
    pthread_t send_msg_thread;
    if (pthread_create(&send_msg_thread, NULL, (void *) send_msg_handler, NULL) != 0) {
        printf ("Create pthread error!\n");
        exit(EXIT_FAILURE);
    }
    pthread_t recv_msg_thread;
    if (pthread_create(&recv_msg_thread, NULL, (void *) recv_msg_handler, NULL) != 0) {
        printf ("Create pthread error!\n");
        exit(EXIT_FAILURE);
    }
    while (1) {
        if(flag) {
            printf("\nBye\n");
            break;
        }
    }

    close(sockfd);
    return 0;
}