#include <arpa/inet.h>
#include <err.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_TEXT_LEN 64

#define bailout(s) \
    {              \
        perror(s); \
        exit(1);   \
    }

typedef struct Node
{
    uint16_t small_value;
    uint32_t big_value;
    uint32_t text_size;
    char text[MAX_TEXT_LEN];
    struct Node *next_value;
} Node;

Node *create_node(uint16_t small_num, uint32_t big_num, const char *text)
{
    Node *node = (Node *)malloc(sizeof(Node));
    if (node == NULL)
    {
        return NULL;
    }

    node->small_value = small_num;
    node->big_value = big_num;
    strncpy(node->text, text, MAX_TEXT_LEN - 1);
    node->text[MAX_TEXT_LEN - 1] = '\0';
    node->text_size = strlen(node->text);
    node->next_value = NULL;
    return node;
}

Node *add_node(Node *head, uint16_t small_num, uint32_t big_num, const char *text)
{
    Node *added_node = create_node(small_num, big_num, text);
    if (added_node == NULL)
    {
        return head;
    }
    if (head == NULL)
    {
        return added_node;
    }
    Node *curr = head;
    while (curr->next_value != NULL)
    {
        curr = curr->next_value;
    }
    curr->next_value = added_node;
    return head;
}

void free_space(Node *head)
{
    Node *curr = head;
    while (curr != NULL)
    {
        Node *next_node = curr->next_value;
        free(curr);
        curr = next_node;
    }
}

int main(int argc, char *argv[])
{
    int sock = 0;
    int i = 0;
    ssize_t total_bytes_send = 0;
    ssize_t bytes_send = 0;
    ssize_t total_size = 0;
    Node *head = NULL;
    struct sockaddr_in server;
    struct hostent *hp = NULL;
    uint16_t curr_small_number;
    uint32_t curr_big_number;
    uint16_t curr_len;
    char data[8 + MAX_TEXT_LEN];

    const uint16_t small_numbers[] = {123, 6544, 3432, 4325, 6752};
    const uint32_t big_numbers[] = {34253453, 535345324, 253425345, 43255345, 675253425};
    const char *texts[] = {"Ala ma kota", "Test 1 2 3", "Lorem ipsum dolor sit amet, consectetur porttitor.", "PSI Z26 lab1", "WEITI"};

    if (argc != 3)
    {
        errx(3, "Incorrect number of argument: %d\n", argc);
    }

    // creating list
    for (i = 0; i < 5; i++)
    {
        head = add_node(head, small_numbers[i], big_numbers[i], texts[i]);
    }

    printf("Starting...\n");
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
        bailout("opening stream socket");

    server.sin_family = AF_INET;

    hp = gethostbyname2(argv[1], AF_INET);
    if (hp == (struct hostent *)0)
    {
        errx(2, "%s: unknown host\n", argv[1]);
    }
    memcpy((char *)&server.sin_addr, (char *)hp->h_addr, hp->h_length);
    server.sin_port = htons(atoi(argv[2]));

    printf("Connecting...\n");
    if (connect(sock, (struct sockaddr *)&server, sizeof server) == -1)
        bailout("connecting stream socket");
    printf("Connected.\n");

    Node *curr = head;
    while (curr != NULL)
    {
        curr_small_number = htons(curr->small_value);
        curr_big_number = htonl(curr->big_value);
        curr_len = htons(curr->text_size);

        memcpy(data, &curr_small_number, 2);
        memcpy(data + 2, &curr_big_number, 4);
        memcpy(data + 6, &curr_len, 2);
        memcpy(data + 8, &curr->text, curr->text_size);
        total_size = 8 + curr->text_size;

        total_bytes_send = 0;
        while (total_bytes_send < total_size)
        {
            bytes_send = write(sock, (char *)data + total_bytes_send, total_size - total_bytes_send);
            if (bytes_send == -1)
            {
                free_space(head);
                close(sock);
                bailout("writing on stream socket");
            }
            total_bytes_send += bytes_send;
        }
        curr = curr->next_value;
    }
    free_space(head);
    close(sock);
    exit(0);
}
