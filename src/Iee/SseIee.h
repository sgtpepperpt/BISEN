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
#include "types.h" // mpc data types

#define OP_SETUP 0x69
#define OP_ADD 0x61
#define OP_SRC 0x73

int readServerPipe;
int writeServerPipe;
//int clientBridgePipe;

static void init_pipes();
static void destroy_pipes();

void f(bytes* out, size* out_len, const unsigned long long pid, const bytes in, const size in_len);

static void setup(bytes* out, size* out_len, const bytes in, const size in_len);
static void add(bytes* out, size* out_len, const bytes in, const size in_len);
static void search(bytes* out, size* out_len, const bytes in, const size in_len);

static void get_docs_from_server(vec_token *query, unsigned count_words);
#endif /* SSE_IEE_H */
