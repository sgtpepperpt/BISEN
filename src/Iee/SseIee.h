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

#include "util/IeeUtils.h"
#include "util/vec_token.h"
#include "IeeCrypt.h"
#include "QueryEvaluator.h"

int readServerPipe;
int writeServerPipe;
//int clientBridgePipe;

void init_pipes();
void destroy_pipes();

//int f(char* data, int data_size, char** output);
void f(unsigned char **out, unsigned long long *out_len, const unsigned long long pid, const unsigned char * in, const unsigned long long in_len);

void setup(unsigned char **out, unsigned long long *out_len, const unsigned char* in, const unsigned long long in_len);
void add(unsigned char **out, unsigned long long *out_len, const unsigned char* in, const unsigned long long in_len);
void search(unsigned char **output, unsigned long long *out_len, const unsigned char* in, const unsigned long long in_len);

void get_docs_from_server(vec_token *query, unsigned count_words);
#endif /* SSE_IEE_H */
