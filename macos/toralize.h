#if !defined(_TORALIZE_H_)
#define _TORALIZE_H_

#include <arpa/inet.h>
#include <dlfcn.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define LOG_PREFIX "TORALIZE_LIB"
#define ENV_PROXY_ADDRESS "PROXY_ADDRESS"
#define ENV_PROXY_PORT "PROXY_PORT"
#define DEFAULT_PROXY_ADDRESS "127.0.0.1"
#define DEFAULT_PROXY_PORT 9050
#define USERNAME "toraliz"

// 8 bit
typedef unsigned char int8;

// 16 bit
typedef short int int16;

// 32 bit
typedef unsigned int32;

/*
             +----+----+----+----+----+----+----+----+----+----+....+----+
             | VN | CD | DSTPORT |      DSTIP        | USERID       |NULL|
             +----+----+----+----+----+----+----+----+----+----+....+----+
 # of bytes:    1    1      2              4           variable       1
 */

struct proxy_request {
  int8 vn;
  int8 cd;
  int16 dstport;
  int32 dstip;
  unsigned char userid[8];
};

#define REQUEST_SIZE sizeof(struct proxy_request)

/*
             +----+----+----+----+----+----+----+----+
             | VN | CD | DSTPORT |      DSTIP        |
             +----+----+----+----+----+----+----+----+
 # of bytes:    1    1      2              4
 */

struct proxy_response {
  int8 vn;
  int8 cd;
  int16 _;
  int32 __;
};

#define RESPONSE_SIZE sizeof(struct proxy_response)

/*
 * https://github.com/opensource-apple/dyld/blob/master/include/mach-o/dyld-interposing.h
 */
#define DYLD_INTERPOSE(_replacement, _replacee)                                \
  __attribute__((used)) static struct {                                        \
    const void *replacement;                                                   \
    const void *replacee;                                                      \
  } _interpose_##_replacee __attribute__((section("__DATA,__interpose"))) = {  \
      (const void *)(unsigned long)&_replacement,                              \
      (const void *)(unsigned long)&_replacee};


void get_proxy_address(char *, const size_t, int *);
void print_socket_and_address(const int, const struct sockaddr *);
struct proxy_request *make_request(struct sockaddr_in *);

int my_connect(int original_socket, const struct sockaddr *address,
               socklen_t addrlen);


DYLD_INTERPOSE(my_connect, connect)

#endif
