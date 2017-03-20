//
//  MainTEE.c
//  BooleanSSE
//
//  Created by Bernardo Ferreira on 15/11/16.
//  Copyright © 2016 Bernardo Ferreira. All rights reserved.
//

//opensgx framework includes

#include "test.h"

void enclave_main()
{
    int port = 5566;
    int srvr_fd;
    int clnt_fd;
    char buf[1];
    struct sockaddr_in addr;
    
    srvr_fd = socket(PF_INET, SOCK_STREAM, 0);
    
    if (srvr_fd == -1) {
        sgx_exit(NULL);
    }
    
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    
    if (bind(srvr_fd, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        sgx_exit(NULL);
    }
    
    if (listen(srvr_fd, 10) != 0) {
        sgx_exit(NULL);
    }
    
    while (1) {
        struct sockaddr_in addr;
        socklen_t len = sizeof(addr);
        clnt_fd = accept(srvr_fd, (struct sockaddr *)&addr, &len);
        if (clnt_fd < 0) {
            puts("ERROR on accept\n");
            continue;
        }
        
        memset(buf, 0, 1);
        //int n = sgx_read(clnt_fd, buf, 255);
        int n = recv(clnt_fd, buf, 1, 0);
        if (n < 0)
            puts("ERROR on read\n");
        
        //puts(buf);
        switch (buf[0]) {
            case 'i':
                initServer(newsockfd);
                break;
            case 'a':
                receiveDocs(newsockfd);
                break;
            case 's':
                this->search(newsockfd);
                break;
            default:
                printf("unkonwn command!\n");
        }
        
        
        //n = sgx_write(clnt_fd, "Successfully received", 21);
        n = send(clnt_fd, "Successfully received", 21, 0);
        if (n < 0)
            puts("ERROR on write\n");
        
        close(clnt_fd);
    }
    
    close(srvr_fd);
    
    sgx_exit(NULL);

}


void initServer (int newsockfd) {

    
}

