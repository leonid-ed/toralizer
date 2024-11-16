#include "toralize.h"
#include <arpa/inet.h>
#include <dlfcn.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>


char kProxyAddress[INET6_ADDRSTRLEN];
int kProxyPort;
int kSocksVer;

__attribute__((constructor)) static void initializer(void)
{
  printf("%s: the library has been loaded\n", LOG_PREFIX);

  // Get proxy address and port, using defaults if environment variables are unset
  get_env_variables(kProxyAddress, sizeof(kProxyAddress), &kProxyPort,
                    &kSocksVer);
}

void print_socket_and_address(const int socket, const struct sockaddr *address)
{
  int ip_version = 4;
  char ip_str[INET6_ADDRSTRLEN];
  int port = 0;
  if (address->sa_family == AF_INET) {
    struct sockaddr_in *addr_in = (struct sockaddr_in *)address;
    inet_ntop(AF_INET, &(addr_in->sin_addr), ip_str, sizeof(ip_str));
    port = ntohs(addr_in->sin_port);
  }
  else if (address->sa_family == AF_INET6) {
    ip_version = 6;
    struct sockaddr_in6 *addr_in6 = (struct sockaddr_in6 *)address;
    inet_ntop(AF_INET6, &(addr_in6->sin6_addr), ip_str, sizeof(ip_str));
    port = ntohs(addr_in6->sin6_port);
  }
  printf("%s: original socket: %d, IPv%d: %s, port: %d\n", LOG_PREFIX, socket,
         ip_version, ip_str, port);
}

void get_env_variables(char *address, const size_t address_len, int *port,
                       int *socks_ver)
{
  const char *env_proxy_address = getenv(ENV_PROXY_ADDRESS);
  const char *env_proxy_port = getenv(ENV_PROXY_PORT);
  const char *env_socks_ver = getenv(ENV_SOCKS_VER);

  // DEFAULT_PROXY_ADDRESS
  if (env_proxy_address && env_proxy_address[0] != '\0') {
    snprintf(address, address_len, "%s", env_proxy_address);
  }
  else {
    snprintf(address, address_len, "%s", DEFAULT_PROXY_ADDRESS);
  }

  // DEFAULT_PROXY_PORT
  if (env_proxy_port && env_proxy_port[0] != '\0') {
    *port = atoi(env_proxy_port);
  }
  else {
    *port = DEFAULT_PROXY_PORT;
  }

  // DEFAULT_SOCKS_VER
  if (env_socks_ver && env_socks_ver[0] != '\0') {
    *socks_ver = atoi(env_socks_ver);
  }
  else {
    *socks_ver = DEFAULT_SOCKS_VER;
  }
}

int run_socks5_flow(const int proxy_socket, const struct sockaddr *address)
{
  int ret = 0;
  int rs = 0;

  Socks5Request1 request1 = {0};
  Socks5Response1 response1 = {0};
  Socks5Request2 request2 = {0};
  Socks5Response2 response2 = {0};

  request1.vn = 5;
  request1.num_methods = 1;
  request1.methods[0] = 0;

  write(proxy_socket, &request1, SOCKS5_REQUEST1_SIZE);

  printf("%s: waiting for response1 from the proxy ... ", LOG_PREFIX);
  fflush(stdout);
  if ((rs = read(proxy_socket, &response1, SOCKS5_RESPONSE1_SIZE)) < 1) {
    if (rs == 0) {
      fprintf(stderr, "%s: unexpected EOF\n", LOG_PREFIX);
      errno = ECONNREFUSED;
    }
    else if (rs == -1) {
      perror(LOG_PREFIX ": read() failed");
    }
    else {
      fprintf(stderr, "%s: unexpected error: %d\n", LOG_PREFIX, rs);
    }
    ret = -1;
    goto __socks5_flow_finish;
  }
  printf("ready\n");

  if (response1.vn != 5) {
    fprintf(stderr, "%s: unexpected version in response1: %d\n", LOG_PREFIX,
            response1.vn);
    ret = -1;
    goto __socks5_flow_finish;
  }

  if (response1.method_num != 0) {
    fprintf(stderr, "%s: unexpected method in response1: %d\n", LOG_PREFIX,
            response1.method_num);
    ret = -1;
    goto __socks5_flow_finish;
  }

  size_t request2_size = address->sa_family == AF_INET
                             ? SOCKS5_REQUEST2_IP4_SIZE
                             : SOCKS5_REQUEST2_IP6_SIZE;

  request2.vn = 5;
  request2.cd = 1;
  request2.rsv = 0;

  if (address->sa_family == AF_INET) {
    request2.atyp = SOCKS5_REQUEST_IP4;
    struct sockaddr_in *address_in = (struct sockaddr_in *)address;
    memcpy(&(request2.ip.ip4.addr), &(address_in->sin_addr), IPV4_SIZE);
    request2.ip.ip4.port = address_in->sin_port;
  }
  else if (address->sa_family == AF_INET6) {
    request2.atyp = SOCKS5_REQUEST_IP6;
    struct sockaddr_in6 *addr_in6 = (struct sockaddr_in6 *)address;
    memcpy(&(request2.ip.ip6.addr), &(addr_in6->sin6_addr), IPV6_SIZE);
    request2.ip.ip6.port = addr_in6->sin6_port;
  }

  write(proxy_socket, &request2, request2_size);

  size_t response2_size = address->sa_family == AF_INET
                              ? SOCKS5_RESPONSE2_IP4_SIZE
                              : SOCKS5_RESPONSE2_IP6_SIZE;

  printf("%s: waiting for response2 from the proxy ... ", LOG_PREFIX);
  fflush(stdout);
  if ((rs = read(proxy_socket, &response2, response2_size)) < 1) {
    if (rs == 0) {
      fprintf(stderr, "unexpected EOF\n");
      errno = ECONNREFUSED;
    }
    else if (rs == -1) {
      perror("read() failed");
    }
    else {
      fprintf(stderr, "unexpected error: %d\n", rs);
    }
    ret = -1;
    goto __socks5_flow_finish;
  }
  printf("ready\n");

  if (response2.vn != 5) {
    fprintf(stderr, "%s: unexpected version in response2: %d\n", LOG_PREFIX,
            response2.vn);
    ret = -1;
    goto __socks5_flow_finish;
  }

  if (response2.rep != 0) {
    fprintf(stderr, "%s: unexpected reply code: %d\n", LOG_PREFIX,
            response2.rep);
    ret = -1;
    goto __socks5_flow_finish;
  }

__socks5_flow_finish:
  return ret;
}

int run_socks4_flow(const int proxy_socket, const struct sockaddr *address)
{
  int ret = 0;

  const struct sockaddr_in *address_in = (struct sockaddr_in *)address;
  Socks4Request request = {0};
  request.vn = 4;
  request.cd = 1;
  request.dstport = address_in->sin_port;
  request.dstip = address_in->sin_addr.s_addr;
  strncpy((char *)&request.userid, USERNAME, 8);

  write(proxy_socket, &request, SOCKS4_REQUEST_SIZE);

  Socks4Response response = {0};

  printf("%s: waiting for response from the proxy ... ", LOG_PREFIX);
  fflush(stdout);
  if (read(proxy_socket, &response, SOCKS4_RESPONSE_SIZE) < 1) {
    perror("read() failed");
    ret = -1;
    goto __socks4_flow_finish;
  }
  printf("ready\n");

  if (response.cd != 90) {
    fprintf(stderr, "%s: unable to traverse the proxy, error code: %d\n",
            LOG_PREFIX, response.cd);
    ret = -1;
    goto __socks4_flow_finish;
  }

__socks4_flow_finish:
  return ret;
}

int my_connect(int original_socket, const struct sockaddr *address,
               socklen_t addrlen)
{
  (void)addrlen;  // suppress warning: unused parameter 'addrlen'

  // validation
  if (address->sa_family != AF_INET && address->sa_family != AF_INET6) {
    fprintf(stderr, "%s: validation failed: unknown address family: %d\n",
            LOG_PREFIX, address->sa_family);
    return -1;
  }
  if (address->sa_family == AF_INET6 && kSocksVer == 4) {
    fprintf(stderr,
            "%s: validation failed: incompatible SOCKS_VER (%d) "
            "with the address family (IPv6)\n",
            LOG_PREFIX, kSocksVer);
    return -1;
  }

  print_socket_and_address(original_socket, address);

  // connect to the proxy server
  int proxy_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (proxy_socket < 0) {
    perror(LOG_PREFIX ": socket");
    return -1;
  }
  printf("%s: created proxy socket %d\n", LOG_PREFIX, proxy_socket);

  struct sockaddr_in sock;
  sock.sin_family = AF_INET;
  sock.sin_port = htons(kProxyPort);
  sock.sin_addr.s_addr = inet_addr(kProxyAddress);

  printf("%s: connecting to SOCKS%d proxy server %s:%d ... ", LOG_PREFIX,
         kSocksVer, kProxyAddress, kProxyPort);

  if (connect(proxy_socket, (struct sockaddr *)&sock, sizeof(sock))) {
    perror(LOG_PREFIX ": connect() failed");
    return -1;
  }
  printf("connected\n");

  // make request through the proxy server
  int rs = 0;
  if (kSocksVer == 4) {
    rs = run_socks4_flow(proxy_socket, address);
  }
  else {
    rs = run_socks5_flow(proxy_socket, address);
  }
  if (rs) {
    return -1;
  }

  printf("%s: successfully connected through the proxy\n", LOG_PREFIX);

  printf(
      "%s: deallocate the original socket %d in favour of the proxy one %d\n",
      LOG_PREFIX, original_socket, proxy_socket);
  if (-1 == dup2(proxy_socket, original_socket)) {
    perror(LOG_PREFIX ": dup2() failed");
    close(proxy_socket);
    return -1;
  }

  return 0;
}
