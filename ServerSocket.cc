/*
 * Copyright ©2022 Hal Perkins.  All rights reserved.  Permission is
 * hereby granted to students registered for University of Washington
 * CSE 333 for use solely during Spring Quarter 2022 for purposes of
 * the course.  No other use, copying, distribution, or modification
 * is permitted without prior written consent. Copyrights for
 * third-party components of this work must be honored.  Instructors
 * interested in reusing these course materials should contact the
 * author.
 */

#include <stdio.h>       // for snprintf()
#include <unistd.h>      // for close(), fcntl()
#include <sys/types.h>   // for socket(), getaddrinfo(), etc.
#include <sys/socket.h>  // for socket(), getaddrinfo(), etc.
#include <arpa/inet.h>   // for inet_ntop()
#include <netdb.h>       // for getaddrinfo()
#include <errno.h>       // for errno, used by strerror()
#include <string.h>      // for memset, strerror()
#include <iostream>      // for std::cerr, etc.

#include "./ServerSocket.h"

extern "C" {
  #include "libhw1/CSE333.h"
}

namespace hw4 {

ServerSocket::ServerSocket(uint16_t port) {
  port_ = port;
  listen_sock_fd_ = -1;
}

ServerSocket::~ServerSocket() {
  // Close the listening socket if it's not zero.  The rest of this
  // class will make sure to zero out the socket if it is closed
  // elsewhere.
  if (listen_sock_fd_ != -1)
    close(listen_sock_fd_);
  listen_sock_fd_ = -1;
}

bool ServerSocket::BindAndListen(int ai_family, int* const listen_fd) {
  // Use "getaddrinfo," "socket," "bind," and "listen" to
  // create a listening socket on port port_.  Return the
  // listening socket through the output parameter "listen_fd"
  // and set the ServerSocket data member "listen_sock_fd_"

  // STEP 1:
  struct addrinfo hints;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = ai_family;       // IPv6 (also handles IPv4 clients)
  hints.ai_socktype = SOCK_STREAM;  // stream
  hints.ai_flags = AI_PASSIVE;      // use wildcard "in6addr_any" address
  hints.ai_flags |= AI_V4MAPPED;    // use v4-mapped v6 if no v6 found
  hints.ai_protocol = IPPROTO_TCP;  // tcp protocol
  hints.ai_canonname = nullptr;
  hints.ai_addr = nullptr;
  hints.ai_next = nullptr;

  // Use port_ as the string representation of our portnumber to
  // pass in to getaddrinfo().  getaddrinfo() returns a list of
  // address structures via the output parameter "result".
  struct addrinfo *result;
  std::string s = std::to_string(port_);
  int res = getaddrinfo(nullptr, s.c_str(), &hints, &result);

  // Did addrinfo() fail?
  if (res != 0) {
    return false;
  }

  // Loop through the returned address structures until we are able
  // to create a socket and bind to one.  The address structures are
  // linked in a list through the "ai_next" field of result.
  int listen_fd_temp = -1;
  for (struct addrinfo *rp = result; rp != nullptr; rp = rp->ai_next) {
    listen_fd_temp = socket(rp->ai_family,
                       rp->ai_socktype,
                       rp->ai_protocol);
    if (listen_fd_temp == -1) {
      // Creating this socket failed.  So, loop to the next returned
      // result and try again.
      listen_fd_temp = 0;
      continue;
    }

    // Configure the socket; we're setting a socket "option."  In
    // particular, we set "SO_REUSEADDR", which tells the TCP stack
    // so make the port we bind to available again as soon as we
    // exit, rather than waiting for a few tens of seconds to recycle it.
    int optval = 1;
    setsockopt(listen_fd_temp, SOL_SOCKET, SO_REUSEADDR,
               &optval, sizeof(optval));

    // Try binding the socket to the address and port number returned
    // by getaddrinfo().
    if (bind(listen_fd_temp, rp->ai_addr, rp->ai_addrlen) == 0) {
      // Bind worked!
      break;
    }

    // The bind failed.  Close the socket, then loop back around and
    // try the next address/port returned by getaddrinfo().
    close(listen_fd_temp);
    listen_fd_temp = -1;
  }

  // Free the structure returned by getaddrinfo().
  freeaddrinfo(result);

  // Did we succeed in binding to any addresses?
  if (listen_fd_temp == -1) {
    // No.  Quit with failure.
    return false;
  }

  // Success. Tell the OS that we want this to be a listening socket.
  if (listen(listen_fd_temp, SOMAXCONN) != 0) {
    close(listen_fd_temp);
    return false;
  }

  // Return the correct file descriptor through the output parameter
  listen_sock_fd_ = listen_fd_temp;
  sock_family_ = ai_family;
  *listen_fd = listen_fd_temp;
  return true;
}

bool ServerSocket::Accept(int* const accepted_fd,
                          std::string* const client_addr,
                          uint16_t* const client_port,
                          std::string* const client_dns_name,
                          std::string* const server_addr,
                          std::string* const server_dns_name) const {
  // Accept a new connection on the listening socket listen_sock_fd_.
  // (Block until a new connection arrives.)  Return the newly accepted
  // socket, as well as information about both ends of the new connection,
  // through the various output parameters.

  // STEP 2:
  // Accepts a connection from a client
  struct sockaddr_storage caddr;
  socklen_t caddr_len = sizeof(caddr);
  int client_fd = accept(listen_sock_fd_,
                          reinterpret_cast<struct sockaddr *>(&caddr),
                          &caddr_len);
  if (client_fd < 0) {
    return false;
  }

  // Returns client file descriptor through output parameters
  *accepted_fd = client_fd;

  // Finds and return client ip address and port number
  auto *addr = reinterpret_cast<struct sockaddr*>(&caddr);
  if (addr->sa_family == AF_INET) {
    char astring[INET_ADDRSTRLEN];
    struct sockaddr_in *in4 = reinterpret_cast<struct sockaddr_in *>(addr);
    inet_ntop(AF_INET, &(in4->sin_addr), astring, INET_ADDRSTRLEN);
    *client_addr = std::string(astring);
    *client_port = ntohs(in4->sin_port);
  } else {
    char astring[INET6_ADDRSTRLEN];
    struct sockaddr_in6 *in6 = reinterpret_cast<struct sockaddr_in6 *>(addr);
    inet_ntop(AF_INET6, &(in6->sin6_addr), astring, INET6_ADDRSTRLEN);
    *client_addr = std::string(astring);
    *client_port = ntohs(in6->sin6_port);
  }

  // Finds and return client DNS name
  char hostname[1024];
  Verify333(getnameinfo(addr, caddr_len, hostname, 1024, nullptr, 0, 0) == 0);
  *client_dns_name = std::string(hostname);

  // Finds and returns server IP address and DNS name
  char hname[1024];
  hname[0] = '\0';
  // Checks if the server address is using and IPv4 or IPv6 address
  if (sock_family_ == AF_INET) {
    // Get and returns server IP address
    struct sockaddr_in srvr;
    socklen_t srvrlen = sizeof(srvr);
    char addrbuf[INET_ADDRSTRLEN];
    getsockname(client_fd, (struct sockaddr *) &srvr, &srvrlen);
    inet_ntop(AF_INET, &srvr.sin_addr, addrbuf, INET_ADDRSTRLEN);
    *server_addr = std::string(addrbuf);
    // Get and returns server DNS name
    getnameinfo((const struct sockaddr *) &srvr,
            srvrlen, hname, 1024, nullptr, 0, 0);
    *server_dns_name = std::string(hname);
  } else {
    // Get and returns server IP address
    struct sockaddr_in6 srvr;
    socklen_t srvrlen = sizeof(srvr);
    char addrbuf[INET6_ADDRSTRLEN];
    getsockname(client_fd, (struct sockaddr *) &srvr, &srvrlen);
    inet_ntop(AF_INET6, &srvr.sin6_addr, addrbuf, INET6_ADDRSTRLEN);
    *server_addr = std::string(addrbuf);
    // Get and returns server DNS name
    getnameinfo((const struct sockaddr *) &srvr,
            srvrlen, hname, 1024, nullptr, 0, 0);
    *server_dns_name = std::string(hname);
  }


  return true;
}

}  // namespace hw4
