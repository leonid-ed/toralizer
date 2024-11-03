#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define LOG_PREFIX "CONNECT_TEST"

int main(int argc, char *argv[])
{
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <host> <port>\n", argv[0]);
    return EXIT_FAILURE;
  }

  const char *server_ip = argv[1];
  int server_port = atoi(argv[2]);

  int sockfd;
  struct sockaddr_in server_addr;

  // Create a socket
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    perror(LOG_PREFIX ": socket() failed");
    exit(EXIT_FAILURE);
  }
  printf("%s: socket created successfully\n", LOG_PREFIX);

  // Set up the server address structure
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(server_port);

  // Convert IP address from text to binary form
  if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
    perror(LOG_PREFIX ": invalid address / address not supported");
    close(sockfd);
    exit(EXIT_FAILURE);
  }

  // Attempt to connect to the server
  int rs =
      connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
  if (rs < 0) {
    perror(LOG_PREFIX ": connect() failed");
    close(sockfd);
    exit(EXIT_FAILURE);
  }
  printf("%s: connected to %s on port %d\n", LOG_PREFIX, server_ip,
         server_port);

  // Close the socket
  close(sockfd);
  printf("%s: socket closed\n", LOG_PREFIX);

  return 0;
}
