#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

int extract_message(char **buf, char **msg)
{
    char    *new_buf;
    int     i;

    *msg = 0;
    if (*buf == 0)
        return (0);
    i = 0;
    while ((*buf)[i])
    {
        if ((*buf)[i] == '\n')
        {
            new_buf = calloc(1, sizeof(*new_buf) * (strlen(*buf + i + 1) + 1));
            *msg = *buf;
            (*msg)[i + 1] = 0;
            *buf = new_buf;
            return (1);
        }
        i++;
    }
    return (0);
}

//char *str_join(char *buf, char *add)

int main()
{
    int sock_fd, conn_fd, len;
    struct sockaddr_in servaddr, cli;

    //Socket creation and verification
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1)
    {
        printf("Socket creation failed...\n");
        exit(0);
    }
    else
    {
        printf("Socket succesfully created...\n");
    }
    bzero(&servaddr, sizeof(servaddr));

    //Assign ip port
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(2130706433);
    servaddr.sin_port = htons(8081);

    if ((bind(sock_fd, (const struct sockaddr *) &servaddr, sizeof(servaddr)))!= 0)
    {
        printf("Socket binding failed...\n");
        exit(0);
    }
    else
        printf("Socket succesfully binded...\n");
    if (listen(sock_fd, 10) != 0)
    {
        printf("Cant listen...\n");
        exit(0);
    }
    len = sizeof(cli);
    conn_fd = accept(sock_fd, (struct sockaddr *)&cli, &len);
    if (conn_fd < 0)
    {
        printf("Failed to accept...\n");
        exit(0);
    }
    else
        printf("Server accept the client...\n");
}
