#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "../../Common/Utils.h"

int create_socket() {
	const char* server_name = "localhost";
	const int server_port = 6969;

	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;

    // store in struct
	inet_pton(AF_INET, server_name, &server_addr.sin_addr);
	server_addr.sin_port = htons(server_port);

	// open a stream socket
	int sock;
	if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		printf("could not create socket\n");
		exit(1);
	}

	if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
		printf("could not connect to server\n");
		exit(1);
	}

    return sock;
}

size_t send_and_receive(int sock, void* buffer, size_t buf_len, void** res_buffer) {
    sendAll(sock, (unsigned char *)&buf_len, sizeof(size_t));
    sendAll(sock, buffer, buf_len);

    size_t res_len;
    receiveAll(sock, (unsigned char *)&res_len, sizeof(size_t));

    *res_buffer = (unsigned char *)malloc(sizeof(unsigned char) * res_len);
    receiveAll(sock, *res_buffer, res_len);

    return res_len;
}
