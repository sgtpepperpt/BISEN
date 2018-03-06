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

// sockets
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <signal.h>

#include "../../Common/Utils.h"

extern int SGX_MPC_MACH_SIGLEN;

static int listen_socket;

void printbuf(unsigned char* b, size_t len) {
    for(unsigned i = 0; i < len; i++) {
        printf("%02x ", b[i]);
    }
    printf("\n");
}

void close_all() {
    close(listen_socket);
    fflush(NULL);
    exit(0);
}

void process_mach_load(const int socket, void** handle) {
    size flen;
    receiveAll(socket, &flen, sizeof(size));

    char * filename = (char *)malloc(sizeof(char) * (flen + 1));
    receiveAll(socket, filename, flen);
    filename[flen] = '\0';

    int res = mach_load(handle, filename);
    printf("Loaded \"%s\" %d\n", filename, res);

    sendAll(socket, &res, sizeof(int));
    sendAll(socket, &SGX_MPC_MACH_SIGLEN, sizeof(int));
    free(filename);
}

void process_mach_quote(const int socket) {
    size omsglen;
    receiveAll(socket, &omsglen, sizeof(size));

    size imsglen;
    receiveAll(socket, &imsglen, sizeof(size));

    unsigned char* imsg = (unsigned char*)malloc(sizeof(unsigned char) * imsglen);
    receiveAll(socket, imsg, imsglen);

    unsigned char* omsg = (unsigned char*)malloc(sizeof(unsigned char) * omsglen);

    int res = mach_quote(omsg, omsglen, imsg, imsglen);
    //printf("Quote : %d\n", res);

    sendAll(socket, &res, sizeof(int));
    sendAll(socket, omsg, omsglen);
    free(imsg);
    free(omsg);
}

void process_mach_run(const int socket, const void* handle) {
    label l;
    receiveAll(socket, &l, sizeof(label));

    size imsglen;
    receiveAll(socket, &imsglen, sizeof(size));

    unsigned char* imsg = (unsigned char*)malloc(sizeof(unsigned char) * imsglen);
    receiveAll(socket, imsg, imsglen);

    size omsglen;
    bytes omsg = NULL;

    int res = mach_run(&omsg, &omsglen, handle, l, imsg, imsglen);

    sendAll(socket, &res, sizeof(int));
    sendAll(socket, &omsglen, sizeof(size));
    sendAll(socket, omsg, omsglen);
    free(imsg);
    free(omsg);
}

void process_mach_verify(const int socket) {
    size imsglen;
    receiveAll(socket, &imsglen, sizeof(size));

    unsigned char* imsg = (unsigned char*)malloc(sizeof(unsigned char) * imsglen);
    receiveAll(socket, imsg, imsglen);

    size codelen;
    receiveAll(socket, &codelen, sizeof(size));

    unsigned char* code = (unsigned char*)malloc(sizeof(unsigned char) * codelen);
    receiveAll(socket, code, codelen);

    int res = mach_verify(imsg, imsglen, code, codelen);

    sendAll(socket, &res, sizeof(int));

    free(imsg);
    free(code);
}

void process_mach_finalize(const int socket, const void* handle) {
    printf("finalise\n");
    mach_finalize(handle);

    // always success
    int res = 0;
    sendAll(socket, &res, sizeof(int));
}

void* process_client(void * args) {
    int socket = *((int *)args);
    void* handle = NULL;

    while (1) {
        unsigned char op;
        receiveAll(socket, &op, sizeof(unsigned char));

        switch (op) {
        case 'l':
            process_mach_load(socket, &handle);
            break;
        case 'q':
            process_mach_quote(socket);
            break;
        case 'r':
            process_mach_run(socket, handle);
            break;
        case 'v':
            process_mach_verify(socket);
            break;
        case 'f':
            process_mach_finalize(socket, handle);
            break;
        default:
            printf("Unrecognised op: %c\n", op);
            break;
        }
    }

    printf("Client closed\n");
    close(socket);
}

int main(int argc, char *argv[]) {
    signal(SIGINT, close_all);

	// port to start the server on
	const int server_port = 7901;

	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(server_port);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if ((listen_socket = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Could not create socket\n");
		exit(1);
	}

    int yes = 1;
    if (setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        exit(1);
    }

	if ((bind(listen_socket, (struct sockaddr *)&server_addr, sizeof(server_addr))) < 0) {
		printf("Could not bind\n");
		exit(1);
	}

	if (listen(listen_socket, 16) < 0) {
		printf("Could not open for listening\n");
		exit(1);
	}

	struct sockaddr_in client_addr;
	socklen_t client_addr_len = 0;

    printf("Listening for requests...\n");
	while (1) {
        int socket;
		if ((socket = accept(listen_socket, (struct sockaddr *)&client_addr, &client_addr_len)) < 0) {
			printf("Accept failed\n");
			exit(1);
		}

        printf("Client connected (%s)\n", inet_ntoa(client_addr.sin_addr));

        pthread_t tid;
        pthread_create(&tid, NULL, process_client, (void*)&socket);
	}

	close(listen_socket);
	return 0;
}
