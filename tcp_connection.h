#ifndef __TCP_CONNECTION_H_INCLUDE__
#define __TCP_CONNECTION_H_INCLUDE__

#include <sys/types.h>
#include <sys/socket.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define IP_ADDRESS_STR_MAX_LEN 32
#define TCP_RECV_BUFFER_MAX_LEN 4096

struct tcp_connection {
	int fd;
	char ip[IP_ADDRESS_STR_MAX_LEN];
	unsigned short port;
};

static inline void tcp_connection_init(
	struct tcp_connection* connection,
	const int fd) {
	assert(connection);
	connection->fd = fd;
}

static inline int tcp_connection_recv(struct tcp_connection* connection, char** buffer, int* len) {
	char* buff = NULL;
	int recv_size = 0;
	int total = 0;
	int offset = 0;
	int count = 0;
	
	assert(connection && buffer && len);

	buff = (char*)malloc(TCP_RECV_BUFFER_MAX_LEN);

	while (1) {
		recv_size = recv(connection->fd, buff+offset, TCP_RECV_BUFFER_MAX_LEN, 0);
		if (recv_size < 0) {
			switch (errno) {
			case EAGAIN:
			case EWOULDBLOCK:
				goto recv_out;
				break;
			default:
				goto err_out;
			}
		}else if (recv_size == 0) {
			close(connection->fd);
			connection->fd = -1;
			goto close_out;
		}else if (recv_size < TCP_RECV_BUFFER_MAX_LEN) {
			total += recv_size;
			goto recv_out;
		}else{
			total += recv_size;
			offset += recv_size;
			count++;
			buff = (char*)realloc(buff, TCP_RECV_BUFFER_MAX_LEN*count);
			if (NULL == buff) {
				goto err_out;
			}
		}
	}

close_out:
	return 0;

recv_out:
	*buffer = buff;
	*len = total;
	return 0;

err_out:
	if (NULL != buff) {
		free(buff);
		buff = NULL;
	}

	return -1;
	
}

static inline int tcp_connection_send(struct tcp_connection* connection, const char* buffer, const int len) {
	int send_size = 0;
	int offset = 0;
	
	assert(connection && buffer);

	do {
		send_size = send(connection->fd, buffer+offset, len-offset, 0);
		if (-1 == send_size) goto err_out;
		offset += send_size;
	}while (offset < len);

	return 0;
	
err_out:
	close(connection->fd);
	connection->fd = -1;
	return -1;
}

static inline void tcp_connection_close(struct tcp_connection* connection) {
	assert(connection);
	close(connection->fd);
	connection->fd = -1;
}
#endif
