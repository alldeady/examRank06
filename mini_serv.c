#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct		s_client {
	int				id, fd;
	struct s_client	*next;
}					t_client;

t_client			*g_clients;
fd_set				curr_sock, cpy_read, cpy_write;
int					sock_fd, g_id = 0;
char				str[4096], buf[4096 + 42];

void	fatal(void) {
	write(2, "Fatal error\n", strlen("Fatal error\n"));
	exit(1);
}

int		get_max_fd(void) {
	t_client	*it = g_clients;
	int			max = 0;

	if (!g_clients)
		return (sock_fd);
	while (it) {
		max < it->fd ? max = it->fd: max;
		it = it->next;
	}
	return (max);
}

void	send_all(int i, char *strP) {
	for (int fd = 0; fd <= get_max_fd(); fd++)
		if (FD_ISSET(fd, &cpy_write))
			if (fd != i && fd != sock_fd)
				send(fd, strP, 4096, 0);
}

int		add_client_to_list(int fd) {
	t_client	*new, *it;

	if (!(new = calloc(1, sizeof(t_client))))
		fatal();
	new->id = g_id++;
	new->fd = fd;
	if (!g_clients)
		g_clients = new;
	else {
		it = g_clients;
		while (it && it->next)
			it = it->next;
		it->next = new;
	}
	return (new->id);
}

void	add_client(void) {
	struct sockaddr_in	client_addr;
	socklen_t			len = sizeof(client_addr);
	int					client_fd;

	if ((client_fd = accept(sock_fd, (struct sockaddr *)&client_addr, &len)) < 0)
		fatal();

	sprintf(str, "server: client %d just arrived\n", add_client_to_list(client_fd));
	send_all(client_fd, str);

	FD_SET(client_fd, &curr_sock);
}

int		rm_client(int fd) {
	t_client	*it = g_clients, *del;
	int			id = -1;

	if (g_clients) {
		if (it->fd == fd) {
			id = it->id;
			g_clients = it->next;
			free(it);
		} else {
			while (it && it->next->fd != fd)
				it = it->next;
			id = it->next->id;
			del = it->next;
			it->next = it->next->next;
			free(del);
		}
	}
	return (id);
}

int		get_id(int fd) {
	t_client	*it = g_clients;

	while (it && it->fd != fd)
		it = it->next;
	return (it ? it->id: -1);
}

int		main(int ac, char **av) {
	if (ac != 2) {
		write(2, "Wrong number of arguments\n", strlen("Wrong number of arguments\n"));
		exit(1);
	}

	if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		fatal();

	struct sockaddr_in	serv_addr;
	uint32_t			host = 2130706433;
	uint16_t			port = atoi(av[1]);
	bzero(&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = host >> 24 | host << 24;
	serv_addr.sin_port = port >> 8 | port << 8;

	if (bind(sock_fd, (const struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
		fatal();
	if (listen(sock_fd, 0) < 0)
		fatal();

	FD_ZERO(&curr_sock);
	FD_SET(sock_fd, &curr_sock);

	while (1) {
		cpy_read = cpy_write = curr_sock;
		if (select(get_max_fd() + 1, &cpy_read, &cpy_write, NULL, NULL) < 0)
			fatal();
		for (int i = 0; i <= get_max_fd(); i++) {
			if (FD_ISSET(i, &cpy_read)) {
				bzero(&str, 4096);
				if (i == sock_fd) {
					add_client();
					break ;
				} else {
					if (recv(i, str, 4096, 0) <= 0) {
						sprintf(str, "server: client %d just left\n", rm_client(i));
						send_all(i, str);
						FD_CLR(i, &curr_sock);
						close(i);
					} else {
						bzero(&buf, 4096 + 42);
						sprintf(buf, "client %d: %s", get_id(i), str);
						send_all(i, buf);
					}
				}
			}
		}
	}
	close(sock_fd);
	return (0);
}
