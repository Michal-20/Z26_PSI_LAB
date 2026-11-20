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

#define DATA "Half a league, half a league . . ."
#define MAX_TEXT_LEN 64

#define bailout(s) \
    {              \
        perror(s); \
        exit(1);   \
    }

typedef struct Node
{
    short small_value;
    int big_value;
    char text[MAX_TEXT_LEN];
    struct Node *next_value;
} Node;

Node *create_node(short small_num, int big_num, const char *text)
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
    node->next_value = NULL;
    return node;
}

Node *add_node(Node *head, short small_num, int big_num, const char *text)
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

main(int argc, char *argv[])
{
    int sock = 0;
    struct sockaddr_in server;
    struct hostent *hp = NULL;

    // creating list
    Node *head = create_node(38, 158, "test");

    for (int i = 0; i < 10; i++)
    {
    }

    printf("Starting...\n");
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
        bailout("opening stream socket");

    server.sin_family = AF_INET;

    if (argc == 3)
    {
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

        if (write(sock, DATA, sizeof DATA) == -1)
            bailout("writing on stream socket");

        close(sock);
        exit(0);
    }
    else
    {
        errx(3, "Incorrect number of argument: %d\n", argc);
    }
}
