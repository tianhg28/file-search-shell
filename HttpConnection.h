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

#ifndef HW4_HTTPCONNECTION_H_
#define HW4_HTTPCONNECTION_H_

#include <stdint.h>
#include <unistd.h>
#include <map>
#include <string>

#include "./HttpRequest.h"
#include "./HttpResponse.h"

namespace hw4 {

// The HttpConnection class represents a connection to a single client
class HttpConnection {
 public:
  explicit HttpConnection(int fd) : fd_(fd) { }
  virtual ~HttpConnection() {
    close(fd_);
    fd_ = -1;
  }

  // Read and parse the next request from the file descriptor fd_,
  // storing the state in the output parameter "request".
  //
  // Returns true if a request could be parsed and read, and false otherwise
  //
  // The caller is responsible to close the connection if the function
  // returns false
  bool GetNextRequest(HttpRequest* const request);

  // Write the response to the file descriptor fd_.
  //
  // Returns true if the response was successfully written, false if the
  // connection experiences an error and should be closed.
  //
  // The caller is responsible to close the connection if the function
  // returns false
  bool WriteResponse(const HttpResponse& response) const;

 private:
  // A helper function to parse the contents of data read from
  // the HTTP connection.
  HttpRequest ParseRequest(const std::string& request) const;

  // The file descriptor associated with the client.
  int fd_;

  // A buffer storing data read from the client.
  std::string buffer_;
};

}  // namespace hw4

#endif  // HW4_HTTPCONNECTION_H_
