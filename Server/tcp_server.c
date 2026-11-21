#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdint.h>

#define MAX_TEXT_LEN 64
#define HDR_SIZE 8 /* Struktura 2B a, 4B b, 2B len */
#define bailout(s) {perror(s); exit(1); }
#define DEF_PORT "8080"

struct node {
    int16_t a;
    int32_t b;
    char *str;
    struct node *next;
};

int recv_all(int fd, void *buf, size_t len){
    char *p = buf;
    size_t alr_read = 0;

    while (len > alr_read){
        ssize_t n = recv(fd, p + alr_read, len - alr_read, 0);
        if(n < 0){
            return -1;
        }else if(n == 0){
            if(alr_read == 0){
                return 0;
            }else{
                return -1;
            }
        }
        alr_read += n;
    }
    return 1;
}
void print_list(struct node *head){
    while(head != NULL){
        printf("a=%d b=%d str=\"%s\"\n", head->a, head->b, head->str);
        head = head->next;
    }
}
void free_list(struct node *head){
    while(head != NULL){
        struct node *next = head->next;
        free(head->str);
        free(head);
        head = next;
    }
}

int moreWork(void) {
    return 1;
}


int  main(int argc, char **argv) {
    uint16_t a_net, len_net;
    uint32_t b_net;

    int sock, msgsock, ListenQueueSize=5;
    struct addrinfo *bindto_address;
    struct addrinfo hints;

    char hdr_buf[HDR_SIZE];



    /* initialize socket type, address and port */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ( getaddrinfo(0, DEF_PORT, &hints, &bindto_address) != 0 )
        bailout("getting local address");

    sock = socket(bindto_address->ai_family, bindto_address->ai_socktype, bindto_address->ai_protocol);
    if (sock == -1)
        bailout("opening stream socket");

    /* dowiaz adres do gniazda */
    if (bind(sock, bindto_address->ai_addr, bindto_address->ai_addrlen )
            == -1)
        bailout("binding stream socket");
    freeaddrinfo(bindto_address);

    /* zacznij przyjmowa� polaczenia... */
    listen(sock, ListenQueueSize);

    do {
        msgsock = accept(sock,(struct sockaddr *) 0, (socklen_t *) 0);
        int proper_read = 1;
        struct node *head = NULL;
        struct node *tail = NULL;
        int done = 0;

        if (msgsock == -1 ) {
            bailout("accept");
	} else do {
                int hdr_result = recv_all(msgsock, hdr_buf, HDR_SIZE);
                if(hdr_result == 1) {
                    // odczyt danych nagłówka
                    memcpy(&a_net, hdr_buf + 0, 2);
                    memcpy(&b_net, hdr_buf + 2, 4);
                    memcpy(&len_net, hdr_buf + 6, 2);

                    int16_t a = (int16_t) ntohs(a_net);
                    int32_t b = (int32_t) ntohl(b_net);
                    uint16_t len = ntohs(len_net);

                    if(len > MAX_TEXT_LEN) {
                        proper_read = 0;
                    }else{// normalne dzialanie
                        char *str = malloc(len + 1);
                        if (len > 0 && recv_all(msgsock, str, len) != 1){
                            free(str);
                            proper_read = 0;
                        }else{
                            str[len] = '\0';
                            struct node *new_node = malloc(sizeof(struct node));
                            new_node->a = a;
                            new_node->b = b;
                            new_node->str = str;
                            new_node->next = NULL;
                            if(tail == NULL){
                                head = new_node;
                                tail = new_node;
                            }else{
                                tail->next = new_node;
                                tail = new_node;
                            }
                        }
                    }
                }else if(hdr_result == 0){
                    printf("%s: Ending connection\n", argv[0]);
                    print_list(head);
                    done = 1;
                }else{
                    proper_read = 0;
                }

                if (proper_read == 0){
                    free_list(head);
                    bailout("reading stream message");
                }

            } while (done == 0);
        free_list(head);
        close(msgsock);
        fflush( stdout );
    } while( moreWork() );

    exit(0);
}
