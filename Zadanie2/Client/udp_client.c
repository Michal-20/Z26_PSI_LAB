#include <arpa/inet.h>
#include <err.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define DGRAMSIZE 512
#define RESPONSESIZE 2

#define bailout(s) \
    {              \
        perror(s); \
        exit(1);   \
    }
#define Usage()                                                              \
    {                                                                        \
        errx(0, "Usage: %s <host> <port> <count> <delay-in-ms>\n", argv[0]); \
    }
#define timeinms(tv) ((tv.tv_sec) * 1000.0 + (tv.tv_usec) / 1000.0)

void print_address(const char *s, struct sockaddr *address, socklen_t addrlen)
{
    char addrbuffer[NI_MAXHOST];

    getnameinfo(address, addrlen, addrbuffer, NI_MAXHOST, 0, 0, NI_NUMERICHOST);
    printf("%s %s\n", s, addrbuffer);
}

int main(int argc, char *argv[])
{
    int sock;
    struct sockaddr_in server;
    struct hostent *hp;
    char databuf[DGRAMSIZE];
    char responsebuf[RESPONSESIZE];
    long int udelay;
    long int i = 0;
    struct timeval waiting_time;
    waiting_time.tv_sec = 2;
    waiting_time.tv_usec = 0;

    if (argc != 5)
    {
        Usage();
    }

    udelay = atol(argv[4]) * 1000;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == -1)
        bailout("opening datagram socket");

    server.sin_family = AF_INET;
    hp = gethostbyname2(argv[1], AF_INET);
    if (hp == (struct hostent *)0)
    {
        errx(2, "%s: unknown host\n", argv[1]);
    }
    printf("address resolved...\n");

    memcpy((char *)&server.sin_addr, (char *)hp->h_addr, hp->h_length);
    server.sin_port = htons(atoi(argv[2]));

    if (connect(sock, (struct sockaddr *)&server, sizeof server) == -1)
        bailout("connecting datagram socket");

    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &waiting_time, sizeof(waiting_time)) < 0)
    {
        perror("Setting up SO_RCVTIMEO error");
    }

    print_address("sending packets to: ", (struct sockaddr *)&server, sizeof(server));

    while (i < atol(argv[3]))
    {
        struct timeval tv;

        memset(databuf, '*', DGRAMSIZE);
        gettimeofday(&tv, NULL);

        sprintf(databuf, "%ld %lf ", i, timeinms(tv));

        printf("-> #%s  \n", databuf);
        fflush(stdout);

        if ((send(sock, databuf, DGRAMSIZE, 0)) < 0)
        {
            perror("sendto error!\n");
        }

        memset(responsebuf, 0, sizeof(responsebuf));
        while (recv(sock, responsebuf, sizeof(responsebuf) - 1, 0) > 0)
        {
            if (responsebuf[0] == ('0' + (i % 2)))
            {
                i += 1;
                usleep(udelay);
                break;
            }
            memset(responsebuf, 0, sizeof(responsebuf));
        }
    }

    close(sock);
    exit(0);
}
