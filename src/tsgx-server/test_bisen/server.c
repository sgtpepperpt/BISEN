#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

// mpc
#include "attke.h"
#include "lac_attest.h"
#include "mach.h"
#include "mpc_process.h"
#include "time_definitions.h"

#include "f/public_key.h"
#include "secret_key.h"

#include "f/commands.h"

// sockets
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <signal.h>

#include "../../Common/Utils.h"

int listen_sock;

void *handle = NULL;

size_t process_message(void * recv_buffer, size_t recv_len, unsigned char ** res_buffer) {
    unsigned char op;
    memcpy(&op, recv_buffer, sizeof(unsigned char));
    recv_buffer++;

    size_t res_len;

    if(op == 'l') {
        char * filename = (char *)recv_buffer;
        int res = mach_load(&handle, filename);

        *res_buffer = malloc(sizeof(unsigned char) * sizeof(int));
        memcpy(*res_buffer, &res, sizeof(int));

        res_len = sizeof(int);
    } else if(op == 'q') {
        unsigned long long omsglen;
        memcpy(&omsglen, recv_buffer, sizeof(unsigned long long));

        unsigned long long imsglen;
        memcpy(&imsglen, recv_buffer + sizeof(unsigned long long), sizeof(unsigned long long));

        *res_buffer = malloc(sizeof(unsigned char) * (omsglen + sizeof(int)));

        int res = mach_quote(*res_buffer + sizeof(int), omsglen, recv_buffer + 2 * sizeof(unsigned long long), imsglen);
        memcpy(*res_buffer, &res, sizeof(int));

        res_len = sizeof(int) + omsglen;
    }

    return res_len;
}

void close_all() {
    close(listen_sock);
    fflush(NULL);
    exit(0);
}

int main(int argc, char *argv[]) {
    signal(SIGINT, close_all);

	// port to start the server on
	int SERVER_PORT = 6969;

    bytes msg_lr;
    size msg_lr_len;
    bytes msg_rl;
    size msg_rl_len;

    bytes sigpk_p1 = pubs[0];
    attke_local_state local_st_p1 = lsts[0];

    bytes sigpk_p2 = pubs[1];
    attke_local_state local_st_p2 = lsts[1];

	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if ((listen_sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		printf("could not create listen socket\n");
		return 1;
	}

    int yes = 1;
    if (setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        perror("setsockopt");
        return 1;
    }

	// bind it to listen to the incoming connections on the created server
	// address, will return -1 on error
	if ((bind(listen_sock, (struct sockaddr *)&server_addr, sizeof(server_addr))) < 0) {
		printf("could not bind socket\n");
		return 1;
	}

	if (listen(listen_sock, 16) < 0) {
		printf("could not open socket for listening\n");
		return 1;
	}

	// socket address used to store client address
	struct sockaddr_in client_addr;
	socklen_t client_addr_len = 0;

    printf("Starting server...\n");
	while (1) {
        int sock;
		if ((sock = accept(listen_sock, (struct sockaddr *)&client_addr, &client_addr_len)) < 0) {
			printf("could not open a socket to accept data\n");
			return 1;
		}

		printf("client connected with ip address: %s\n", inet_ntoa(client_addr.sin_addr));

        // receive message from client
        size_t recv_len;
        receiveAll(sock, (unsigned char *)&recv_len, sizeof(size_t));

        unsigned char * recv_buffer = (unsigned char *)malloc(sizeof(unsigned char) * recv_len);
        receiveAll(sock, recv_buffer, recv_len);
printf("gotall\n");
        // prepare response
        unsigned char * res_buffer;
        size_t res_len = process_message(recv_buffer, recv_len, &res_buffer);

        // send response
        sendAll(sock, (unsigned char *)&res_len, sizeof(size_t));
        sendAll(sock, res_buffer, res_len);
printf("sentall\n");
        free(recv_buffer);
        free(res_buffer);

		close(sock);
	}

	//close(listen_sock);
	return 0;
}
