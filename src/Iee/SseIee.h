//
//  SseIee.h
//  BooleanSSE
//
//  Created by Bernardo Ferreira on 27/03/17.
//  Copyright © 2017 Bernardo Ferreira. All rights reserved.
//

#ifndef SSE_IEE_H
#define SSE_IEE_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>

#include "util/IeeUtils.h"
#include "util/vec_token.h"
#include "IeeCrypt.h"
#include "QueryEvaluator.h"

int readServerPipe;
int writeServerPipe;
int clientBridgePipe;

void init_pipes();
void destroy_pipes();

int f(char* data, int data_size, char** output);
int process(char* ciphertext, int ciphertext_size, char** enc_output);

void setup(char* enc_data, int enc_data_size);
void add(char* data, int data_size);
int search(char* data, int data_size, char** output);
void get_docs_from_server(vec_token *query, unsigned count_words);
#endif /* SSE_IEE_H */
