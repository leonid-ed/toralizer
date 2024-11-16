#if !defined(_TORALIZE_H_)
#define _TORALIZE_H_

#include <arpa/inet.h>
#include <dlfcn.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define LOG_PREFIX "TORALIZE_LIB"
#define ENV_PROXY_ADDRESS "PROXY_ADDRESS"
#define ENV_PROXY_PORT "PROXY_PORT"
#define ENV_SOCKS_VER "SOCKS_VER"
#define DEFAULT_PROXY_ADDRESS "127.0.0.1"
#define DEFAULT_PROXY_PORT 9050
#define DEFAULT_SOCKS_VER 5
#define USERNAME "toraliz"


#define IPV4_SIZE sizeof(struct in_addr)
#define IPV6_SIZE sizeof(struct in6_addr)

// === SOCKS 4 STRUCTURES ===

/*
             +----+----+----+----+----+----+----+----+----+----+....+----+
             | VN | CD | DSTPORT |      DSTIP        | USERID       |NULL|
             +----+----+----+----+----+----+----+----+----+----+....+----+
 # of bytes:    1    1      2              4           variable       1
 */

typedef struct {
  uint8_t vn;
  uint8_t cd;
  uint16_t dstport;
  uint32_t dstip;
  unsigned char userid[8];
} Socks4Request;

#define SOCKS4_REQUEST_SIZE sizeof(Socks4Request)

/*
             +----+----+----+----+----+----+----+----+
             | VN | CD | DSTPORT |      DSTIP        |
             +----+----+----+----+----+----+----+----+
 # of bytes:    1    1      2              4
 */

typedef struct {
  uint8_t vn;
  uint8_t cd;
  uint16_t _;
  uint32_t __;
} Socks4Response;

#define SOCKS4_RESPONSE_SIZE sizeof(Socks4Response)


// === SOCKS 5 STRUCTURES ===

/*
                   +----+----------+----------+
                   |VER | NMETHODS | METHODS  |
                   +----+----------+----------+
                   | 1  |    1     | 1 to 255 |
                   +----+----------+----------+
 */

typedef struct {
  uint8_t vn;
  uint8_t num_methods;
  uint8_t methods[1];
} Socks5Request1;

#define SOCKS5_REQUEST1_SIZE sizeof(Socks5Request1)

/*
                         +----+--------+
                         |VER | METHOD |
                         +----+--------+
                         | 1  |   1    |
                         +----+--------+
 */

typedef struct {
  uint8_t vn;
  uint8_t method_num;
} Socks5Response1;

#define SOCKS5_RESPONSE1_SIZE sizeof(Socks5Response1)

/*
        +----+-----+-------+------+----------+----------+
        |VER | CMD |  RSV  | ATYP | DST.ADDR | DST.PORT |
        +----+-----+-------+------+----------+----------+
        | 1  |  1  | X'00' |  1   | Variable |    2     |
        +----+-----+-------+------+----------+----------+
 */

typedef union {
  struct ip4_ {
    uint8_t addr[IPV4_SIZE];
    uint16_t port;
  } ip4;
  struct ip6_ {
    uint8_t addr[IPV6_SIZE];
    uint16_t port;
  } ip6;
} ip_t;

typedef struct {
  uint8_t vn;
  uint8_t cd;
  uint8_t rsv;
  uint8_t atyp;
  ip_t ip;
} Socks5Request2;


#define SOCKS5_REQUEST2_IP4_SIZE                                               \
  (sizeof(Socks5Request2) - (sizeof(struct ip6_) - sizeof(struct ip4_)))
#define SOCKS5_REQUEST2_IP6_SIZE sizeof(Socks5Request2)

/*
        +----+-----+-------+------+----------+----------+
        |VER | REP |  RSV  | ATYP | BND.ADDR | BND.PORT |
        +----+-----+-------+------+----------+----------+
        | 1  |  1  | X'00' |  1   | Variable |    2     |
        +----+-----+-------+------+----------+----------++
 */

typedef struct {
  uint8_t vn;
  uint8_t rep;
  uint8_t rsv;
  uint8_t atyp;
  ip_t ip;
} Socks5Response2;

#define SOCKS5_RESPONSE2_IP4_SIZE                                              \
  (sizeof(Socks5Response2) - (sizeof(struct ip6_) - sizeof(struct ip4_)))
#define SOCKS5_RESPONSE2_IP6_SIZE sizeof(Socks5Response2)

#define SOCKS5_REQUEST_IP4 1
#define SOCKS5_REQUEST_IP6 4


void get_env_variables(char *, const size_t, int *, int *);
void print_socket_and_address(const int, const struct sockaddr *);
int run_socks4_flow(const int, const struct sockaddr *);
int run_socks5_flow(const int, const struct sockaddr *);

int connect(int, const struct sockaddr *, socklen_t);


#endif  // _TORALIZE_H_
