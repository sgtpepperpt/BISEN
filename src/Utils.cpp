//
//  Util.cpp
//  MIE
//
//  Created by Bernardo Ferreira on 05/03/15.
//  Copyright (c) 2015 NovaSYS. All rights reserved.
//

#include "Utils.h"

uint64_t pack754(long double f, unsigned bits, unsigned expbits)
{
    long double fnorm;
    int shift;
    long long sign, exp, significand;
    unsigned significandbits = bits - expbits - 1; // -1 for sign bit
    
    if (f == 0.0) return 0; // get this special case out of the way
    
    // check sign and begin normalization
    if (f < 0) { sign = 1; fnorm = -f; }
    else { sign = 0; fnorm = f; }
    
    // get the normalized form of f and track the exponent
    shift = 0;
    while(fnorm >= 2.0) { fnorm /= 2.0; shift++; }
    while(fnorm < 1.0) { fnorm *= 2.0; shift--; }
    fnorm = fnorm - 1.0;
    
    // calculate the binary form (non-float) of the significand data
    significand = fnorm * ((1LL<<significandbits) + 0.5f);
    
    // get the biased exponent
    exp = shift + ((1<<(expbits-1)) - 1); // shift + bias
    
    // return the final answer
    return (sign<<(bits-1)) | (exp<<(bits-expbits-1)) | significand;
}

long double unpack754(uint64_t i, unsigned bits, unsigned expbits)
{
    long double result;
    long long shift;
    unsigned bias;
    unsigned significandbits = bits - expbits - 1; // -1 for sign bit
    
    if (i == 0) return 0.0;
    
    // pull the significand
    result = (i&((1LL<<significandbits)-1)); // mask
    result /= (1LL<<significandbits); // convert back to float
    result += 1.0f; // add the one back on
    
    // deal with the exponent
    bias = (1<<(expbits-1)) - 1;
    shift = ((i>>significandbits)&((1LL<<expbits)-1)) - bias;
    while(shift > 0) { result *= 2.0; shift--; }
    while(shift < 0) { result /= 2.0; shift++; }
    
    // sign it
    result *= (i>>(bits-1))&1? -1.0: 1.0;
    
    return result;
}

//int denormalize(float val, int size) {
//    return round(val * size);
//}

struct timespec getTime() {
    struct timespec ts;
#ifdef __MACH__ // OS X does not have clock_gettime, use clock_get_time
    clock_serv_t cclock;
    mach_timespec_t mts;
    host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
    clock_get_time(cclock, &mts);
    mach_port_deallocate(mach_task_self(), cclock);
    ts.tv_sec = mts.tv_sec;
    ts.tv_nsec = mts.tv_nsec;
#else
    clock_gettime(CLOCK_REALTIME, &ts);
#endif
    return ts;
}

struct timespec diff(struct timespec start, struct timespec end) {
    struct timespec temp;
    if ((end.tv_nsec-start.tv_nsec)<0) {
        temp.tv_sec = end.tv_sec-start.tv_sec-1;
        temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
    } else {
        temp.tv_sec = end.tv_sec-start.tv_sec;
        temp.tv_nsec = end.tv_nsec-start.tv_nsec;
    }
    return temp;
}

double diffSec(struct timespec start, struct timespec end) {
    double startNano = start.tv_sec+(start.tv_nsec/1000000000.0);
    double endNano = end.tv_sec+(end.tv_nsec/1000000000.0);
    return endNano - startNano;
}


std::string getHexRepresentation(const unsigned char * Bytes, size_t Length)
{
    std::ostringstream os;
    os.fill('0');
    os<<std::hex;
    for(const unsigned char * ptr=Bytes;ptr<Bytes+Length;ptr++)
        os<<std::setw(2)<<(unsigned int)*ptr;
    return os.str();
}

void pee(const char *msg)
{
    perror(msg);
    exit(0);
}

int connectAndSend (char* buff, long size) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct hostent *server = gethostbyname(serverIP);
    struct sockaddr_in serv_addr;
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,(char*)&serv_addr.sin_addr.s_addr,server->h_length);
    serv_addr.sin_port = htons(serverPort);
    if (connect(sockfd,(struct sockaddr*) &serv_addr,sizeof(serv_addr)) < 0)
        pee("ERROR connecting");
    socketSend (sockfd, buff, size);
    return sockfd;
}

void socketSend (int sockfd, char* buff, long size) {
    if (sendall(sockfd, buff, size) < 0)
        pee("ERROR writing to socket");
}

int sendall(int s, char *buf, long len)
{
    long total = 0;        // how many bytes we've sent
    long bytesleft = len; // how many we have left to send
    long n = 0;
    
    while(total < len) {
        n = write(s, buf+total, bytesleft);
        if (n == -1) { break; }
        total += n;
        bytesleft -= n;
    }
    
    return n==-1||total!=len ? -1 : 0; // return -1 on failure, 0 on success
}

void socketReceive(int sockfd, char* buff, long size) {
    if (receiveAll(sockfd, buff, size) < 0)
        pee("ERROR reading from socket");
}

int receiveAll (int socket, char* buff, long len) {
    int r = 0;
    while (r < len) {
        ssize_t n = read(socket,&buff[r],len-r);
        if (n < 0) pee("ERROR reading from socket");
        r+=n;
    }
    return r;
}

void addToArr (void* val, int size, char* arr, int* pos) {
    memcpy(&arr[*pos], val, size);
    *pos += size;
}

void addIntToArr (int val, char* arr, int* pos) {
    uint32_t x = htonl(val);
    addToArr (&x, sizeof(uint32_t), arr, pos);
}

void addFloatToArr (float val, char* arr, int* pos) {
    uint64_t x = pack754_32(val);
    addToArr (&x, sizeof(uint64_t), arr, pos);
}

void readFromArr (void* val, int size, char* arr, int* pos) {
    memcpy(val, &arr[*pos], size);
    *pos += size;
}

int readIntFromArr (char* arr, int* pos) {
    uint32_t x;
    readFromArr(&x, sizeof(uint32_t), arr, pos);
    return ntohl(x);
}

float readFloatFromArr (char* arr, int* pos) {
    uint64_t x;
    readFromArr(&x, sizeof(uint64_t), arr, pos);
    return (float)unpack754_32(x);
}

double readDoubleFromArr (char* arr, int* pos) {
    uint64_t x;
    readFromArr(&x, sizeof(uint64_t), arr, pos);
    return unpack754_64(x);
}