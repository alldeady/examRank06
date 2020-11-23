#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>

typedef struct			s_client {
	int					fd;
	int					id;
	struct s_client		*next;
}						t_client;

fd_set				cpy_read, cpy_write, curr_sock;
int					max_sock, sock_fd, g_id = 0;
t_client			*g_clients;
char				str[4096], buf[4096 + 42];

void	exit_fatal(void) {
	write(2, "Fatal error\n", strlen("Fatal error\n"));
	exit (1);
}

void	send_all(int i, char *str) {
	for(int fd = 0; fd <= max_sock; fd++) {
		if (FD_ISSET(fd, &cpy_write)) {
			if (fd != i && fd != sock_fd)
				send(fd, str, 4096, 0);
		}
	}
}

int		add_client_to_list(int fd) {
	t_client	*tmp, *new_client;

	if (!(new_client = calloc(1, sizeof(t_client))))
		exit_fatal();
	new_client->fd = fd;
	new_client->id = g_id++;
	if (!g_clients)
		g_clients = new_client;
	else {
		tmp = g_clients;
		while (tmp->next)
			tmp = tmp->next;
		tmp->next = new_client;
	}
	return (new_client->id);
}

void		add_new_client(void) {
	struct sockaddr_in	client_address;
	socklen_t			len = sizeof(client_address);
	int					client_fd;

	if ((client_fd = accept(sock_fd, (struct sockaddr *)&client_address, &len)) == -1)
		exit_fatal();

	FD_SET(client_fd, &curr_sock);
	if (client_fd > max_sock)
		max_sock = client_fd + 1;

	sprintf(str, "server: client %d just arrived\n", add_client_to_list(client_fd));
	send_all(client_fd, str);
}

int		remove_client_from_list(int fd) {
	int			id = 0;
	t_client	*del, *tmp = g_clients;

	if (g_clients) {
		if (tmp->fd == fd) {
			id = tmp->id;
			g_clients = tmp->next;
			free(tmp);
		} else {
			while (tmp && tmp->next->fd != fd)
				tmp = tmp->next;
			id = tmp->next->id;
			del = tmp->next;
			tmp->next = tmp->next->next;
			free(del);
		}
	}
	return (id);
}

int		get_client_id(int fd) {
	t_client *tmp = g_clients;

	while (tmp && tmp->fd != fd)
		tmp = tmp->next;
	if (!tmp)
		return (-1);
	return (tmp->id);
}

int main(int ac, char **av) {
	if (ac != 2) {
		write(2, "Wrong number of arguments\n", strlen("Wrong number of arguments\n"));
		exit(1);
	}

	if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		exit_fatal();

	struct sockaddr_in	serv_addr;
	bzero(&serv_addr, sizeof(serv_addr));
	serv_addr.sin_port = htons(atoi(av[1]));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(2130706433);

	if (bind(sock_fd, (const struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
		exit_fatal();
	if (listen(sock_fd, 0) < 0)
		exit_fatal();

	max_sock = sock_fd + 1;
	FD_ZERO(&curr_sock);
	FD_SET(sock_fd, &curr_sock);

	while (1) {
		cpy_read = cpy_write = curr_sock;
		if (select(max_sock + 1, &cpy_read, &cpy_write, NULL, NULL) == -1)
			exit_fatal();
		for (int i = 0; i <= max_sock; i++) {
			if (FD_ISSET(i, &cpy_read)) {
				bzero(&str, 4096);
				if (i == sock_fd)
					 add_new_client();
				else {
					if (recv(i, str, 4096, 0) <= 0) {
						sprintf(str, "server: client %d just left\n", remove_client_from_list(i));
						send_all(i, str);
						FD_CLR(i, &curr_sock);
						close(i);
					} else {
						bzero(&buf, 4096 + 42);
						sprintf(buf, "client %d: %s", get_client_id(i), str);
						send_all(i, buf);
					}
				}
			}
		}
	}
	close (sock_fd);
	return (0);
}
