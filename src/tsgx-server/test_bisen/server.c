#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
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

extern int SGX_MPC_MACH_SIGLEN;

int listen_sock;
void *handle = NULL;

void printbuf(unsigned char* b, size_t len) {
    for(unsigned i = 0; i < len; i++) {
        printf("%02x ", b[i]);
    }
    printf("\n");
}

size_t process_message(int socket, void * recv_buffer, size_t recv_len, unsigned char ** res_buffer) {
    unsigned char op;
    memcpy(&op, recv_buffer, sizeof(unsigned char));
    //printf("op %c\n", op);
    void* tmp = recv_buffer + 1;

    size_t res_len;

    if(op == 'l') {
        char * filename = (char *)tmp;
        int res = mach_load(&handle, filename);
        printf("Loaded \"%s\" : %d\n", filename, res);

        // allocate response buffer
        res_len = 2 * sizeof(int);
        *res_buffer = malloc(sizeof(unsigned char) * res_len);

        memcpy(*res_buffer, &res, sizeof(int));
        memcpy(*res_buffer + sizeof(int), &SGX_MPC_MACH_SIGLEN, sizeof(int));
    } else if(op == 'q') {
        size omsglen;
        memcpy(&omsglen, tmp, sizeof(size));
        tmp += sizeof(size);

        size imsglen;
        memcpy(&imsglen, tmp, sizeof(size));
        tmp += sizeof(size);

        bytes imsg = tmp;

        // allocate response buffer
        res_len = sizeof(int) + omsglen;
        *res_buffer = malloc(sizeof(unsigned char) * res_len);

        int res = mach_quote(*res_buffer + sizeof(int), omsglen, imsg, imsglen);
        //printf("Quote : %d\n", res);
        memcpy(*res_buffer, &res, sizeof(int));

    } else if(op == 'r') {
        label l;
        memcpy(&l, tmp, sizeof(label));
        tmp += sizeof(label);

        /*size omsglen;
        memcpy(&omsglen, tmp, sizeof(size));
        tmp += sizeof(size);*/

        size imsglen;
        memcpy(&imsglen, tmp, sizeof(size));
        tmp += sizeof(size);

        bytes imsg = tmp;
        size omsglen;
        bytes omsg = NULL;

        int res = mach_run(&omsg, &omsglen, handle, l, imsg, imsglen);

        // allocate response buffer
        res_len = sizeof(int) + sizeof(size) + omsglen;
        *res_buffer = malloc(sizeof(unsigned char) * res_len);

        memcpy(*res_buffer, &res, sizeof(int));
        memcpy(*res_buffer + sizeof(int), &omsglen, sizeof(size));
        memcpy(*res_buffer + sizeof(int) + sizeof(size), omsg, omsglen);

        //printbuf(*res_buffer, res_len);
        //printf("Run : %d; len %llu; res_len %lu\n", res, omsglen, res_len);
    } else if(op == 'v') {printf("v\n");
        size imsglen;
        memcpy(&imsglen, tmp, sizeof(size));
        tmp += sizeof(size);

        bytes imsg = tmp;

        size codelen;
        memcpy(&codelen, tmp, sizeof(size));
        tmp += sizeof(size);

        bytes code = tmp;

        int res = mach_verify(imsg, imsglen, code, codelen);

        // allocate response buffer
        res_len = sizeof(int);
        *res_buffer = malloc(sizeof(unsigned char) * res_len);

        memcpy(*res_buffer, &res, sizeof(int));
    } else if(op == 'f') {
        printf("finalise\n");
        mach_finalize(handle);

        // allocate response buffer
        res_len = sizeof(int);
        *res_buffer = malloc(sizeof(unsigned char) * res_len);

        int res = 0;
        memcpy(*res_buffer, &res, sizeof(int));
    }

    return res_len;
}

void close_all() {
    close(listen_sock);
    fflush(NULL);
    exit(0);
}

void * process_client(void * args) {
    int sock = *((int *)args);

    while (1) {
        // receive message from client
        size_t recv_len;
        receiveAll(sock, (unsigned char *)&recv_len, sizeof(size_t));

        unsigned char * recv_buffer = (unsigned char *)malloc(sizeof(unsigned char) * recv_len);
        receiveAll(sock, recv_buffer, recv_len);
        //printf("gotall\n");

        // prepare response
        unsigned char * res_buffer;
        size_t res_len = process_message(sock, recv_buffer, recv_len, &res_buffer);

        // send response
        sendAll(sock, (unsigned char *)&res_len, sizeof(size_t));
        sendAll(sock, res_buffer, res_len);
        //printf("sentall\n");

        free(recv_buffer);
        free(res_buffer);
    }

    close(sock);
}

int main(int argc, char *argv[]) {
    signal(SIGINT, close_all);

	// port to start the server on
	const int SERVER_PORT = 6969;

	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if ((listen_sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		printf("could not create listen socket\n");
		exit(1);
	}

    int yes = 1;
    if (setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        exit(1);
    }

	if ((bind(listen_sock, (struct sockaddr *)&server_addr, sizeof(server_addr))) < 0) {
		printf("could not bind socket\n");
		exit(1);
	}

	if (listen(listen_sock, 16) < 0) {
		printf("could not open socket for listening\n");
		exit(1);
	}

	struct sockaddr_in client_addr;
	socklen_t client_addr_len = 0;

    printf("Listening for requests...\n");
	while (1) {
        int sock;
		if ((sock = accept(listen_sock, (struct sockaddr *)&client_addr, &client_addr_len)) < 0) {
			printf("Client accept failed\n");
			exit(1);
		}

        printf("Client connected with ip address: %s\n", inet_ntoa(client_addr.sin_addr));

        pthread_t tid;
        pthread_create(&tid, NULL, process_client, (void*)&sock);
	}

	close(listen_sock);
	return 0;
}
