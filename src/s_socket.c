#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#define NULL ((void *)0)

#include "s_socket.h"
#include "kernel.h"

int make_service(short unsigned int port, char *my_hostname, size_t my_hostnamelen)
{
  int s, i;
  struct sockaddr_in6 sin6;
  int ipv6_only = 0; /* 0 = allow both IPv4 and IPv6 (dual-stack) */

  if (gethostname(my_hostname,my_hostnamelen) < 0) {
    return -1; /* Error in hostname? */
  }

  /* Try to create an IPv6 socket that also accepts IPv4 connections (dual-stack) */
  memset(&sin6, 0, sizeof(sin6));
  sin6.sin6_family = AF_INET6;
  sin6.sin6_port = htons(port);
  sin6.sin6_addr = in6addr_any; /* Bind to all interfaces */

  if ((s = socket(AF_INET6, SOCK_STREAM, 0)) < 0) {
    /* IPv6 not available, fall back to IPv4 */
    struct sockaddr_in sin4;
    memset(&sin4, 0, sizeof(sin4));
    sin4.sin_family = AF_INET;
    sin4.sin_port = htons(port);
    sin4.sin_addr.s_addr = htonl(INADDR_ANY);

    if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      return -3; /* Error in socket call */
    }

    /* Original SO_REUSEADDR code */
    i = 1;
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(i)) == -1) {
      progerror("setsockopt(SO_REUSEADDR, v4)");
    }

    if (bind(s, (struct sockaddr *)&sin4, sizeof(sin4)) < 0) {
      close(s);
      return -4; /* Error in bind. */
    }
  } else {
    /* IPv6 socket created - set up dual-stack to also accept IPv4 */

    /* Original SO_REUSEADDR code */
    i = 1;
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(i)) == -1) {
      progerror("setsockopt(SO_REUSEADDR, v6)");
    }

    /* Explicitly set IPV6_V6ONLY to 0 to allow both IPv4 and IPv6 connections */
    if (setsockopt(s, IPPROTO_IPV6, IPV6_V6ONLY, &ipv6_only, sizeof(ipv6_only)) == -1) {
        /* If this fails, we can still bind but it will be IPv6 only, so just continue */
    }

    if (bind(s, (struct sockaddr *)&sin6, sizeof(sin6)) < 0) {
      close(s);
      return -4; /* Error in bind. */
    }
  }

  if (listen(s, 5) != 0) {
    close(s);
    return -5; /* Error in listen */
  }

  /* Set non-blocking IO: */
  if (fcntl(s, F_SETFL, O_NDELAY) == -1) {
    close(s);
    return -6; /* Error in fcntl */
  }


  return s;
}


