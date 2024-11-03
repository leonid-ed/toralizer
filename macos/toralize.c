#include "toralize.h"
#include <arpa/inet.h>
#include <dlfcn.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>


char _proxy_address[INET6_ADDRSTRLEN];
int _proxy_port;

__attribute__((constructor)) static void initializer(void)
{
  printf("%s: the library has been loaded\n", LOG_PREFIX);

  // Get proxy address and port, using defaults if environment variables are unset
  get_proxy_address(_proxy_address, sizeof(_proxy_address), &_proxy_port);
}

struct proxy_request *make_request(struct sockaddr_in *address)
{
  struct proxy_request *request = malloc(REQUEST_SIZE);
  request->vn = 4;
  request->cd = 1;
  request->dstport = address->sin_port;
  request->dstip = address->sin_addr.s_addr;
  strncpy((char *)request->userid, USERNAME, 8);
  return request;
}

void print_socket_and_address(const int socket, const struct sockaddr *address)
{
  char ip_str[INET_ADDRSTRLEN];
  struct sockaddr_in *addr_in = (struct sockaddr_in *)address;
  inet_ntop(AF_INET, &(addr_in->sin_addr), ip_str, sizeof(ip_str));
  int port = ntohs(addr_in->sin_port);
  printf("%s: original socket: %d, IP: %s, port: %d\n", LOG_PREFIX, socket,
         ip_str, port);
}

void get_proxy_address(char *address, const size_t address_len, int *port)
{
  const char *env_proxy_address = getenv(ENV_PROXY_ADDRESS);
  const char *env_proxy_port = getenv(ENV_PROXY_PORT);

  // Use environment variable if set, otherwise use default
  if (env_proxy_address && env_proxy_address[0] != '\0') {
    snprintf(address, address_len, "%s", env_proxy_address);
  }
  else {
    snprintf(address, address_len, "%s", DEFAULT_PROXY_ADDRESS);
  }

  // Use environment variable if set, otherwise use default
  if (env_proxy_port && env_proxy_port[0] != '\0') {
    *port = atoi(env_proxy_port);
  }
  else {
    *port = DEFAULT_PROXY_PORT;
  }
}

int my_connect(int original_socket, const struct sockaddr *address,
               socklen_t addrlen)
{
  print_socket_and_address(original_socket, address);

  int proxy_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (proxy_socket < 0) {
    perror(LOG_PREFIX ": socket");
    return -1;
  }
  printf("%s: created proxy socket %d\n", LOG_PREFIX, proxy_socket);

  struct sockaddr_in sock;
  sock.sin_family = AF_INET;
  sock.sin_port = htons(_proxy_port);
  sock.sin_addr.s_addr = inet_addr(_proxy_address);

  printf("%s: connecting to SOCKS4 proxy server %s:%d ... ", LOG_PREFIX,
         _proxy_address, _proxy_port);

  if (connect(proxy_socket, (struct sockaddr *)&sock, sizeof(sock))) {
    perror("connect() failed");
    return -1;
  }
  printf("connected\n");

  struct proxy_request *request = make_request((struct sockaddr_in *)address);
  write(proxy_socket, request, REQUEST_SIZE);

  char buf[RESPONSE_SIZE];
  memset(buf, 0, RESPONSE_SIZE);

  printf("%s: waiting for response from the proxy ... ", LOG_PREFIX);
  if (read(proxy_socket, buf, RESPONSE_SIZE) < 1) {
    perror("read() failed");
    free(request);
    close(proxy_socket);
    return -1;
  }
  printf("ready\n");

  struct proxy_response *response = (struct proxy_response *)buf;

  if (response->cd != 90) {
    fprintf(stderr, "%s: unable to traverse the proxy, error code: %d\n",
            LOG_PREFIX, response->cd);
    close(proxy_socket);
    free(request);
    return -1;
  }

  printf("%s: successfully connected through the proxy\n", LOG_PREFIX);

  printf(
      "%s: deallocate the original socket %d in favour of the proxy one %d\n",
      LOG_PREFIX, original_socket, proxy_socket);
  if (-1 == dup2(proxy_socket, original_socket)) {
    perror(LOG_PREFIX ": dup2() failed");
    free(request);
    close(proxy_socket);
    return -1;
  }

  free(request);
  return 0;
}
