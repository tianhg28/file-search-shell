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

#include <stdint.h>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <map>
#include <string>
#include <vector>

extern "C" {
  #include "libhw1/CSE333.h"
}

#include "./HttpRequest.h"
#include "./HttpUtils.h"
#include "./HttpConnection.h"

using std::map;
using std::string;
using std::vector;
using boost::algorithm::split;
using boost::algorithm::to_lower;
using boost::algorithm::is_any_of;
using boost::algorithm::token_compress_on;

namespace hw4 {

static const char* kHeaderEnd = "\r\n\r\n";
static const int kHeaderEndLen = 4;

bool HttpConnection::GetNextRequest(HttpRequest* const request) {
  // Use WrappedRead from HttpUtils.cc to read bytes from the files into
  // private buffer_ variable. Keep reading until:
  // 1. The connection drops
  // 2. You see a "\r\n\r\n" indicating the end of the request header.
  //
  // Hint: Try and read in a large amount of bytes each time you call
  // WrappedRead.
  //
  // After reading complete request header, use ParseRequest() to parse into
  // an HttpRequest and save to the output parameter request.
  //
  // Important note: Clients may send back-to-back requests on the same socket.
  // This means WrappedRead may also end up reading more than one request.
  // Make sure to save anything you read after "\r\n\r\n" in buffer_ for the
  // next time the caller invokes GetNextRequest()!

  // STEP 1:
  int buf_size = 8;
  char temp_buf[8];
  while (1) {
    int res = WrappedRead(fd_, reinterpret_cast<unsigned char*>(temp_buf),
                          buf_size);
    if (res == -1) {
      return false;
    }
    if (res == 0) {
      break;
    }
    std::string temp_str(temp_buf, res);
    buffer_ = buffer_ + temp_str;
    if (buffer_.find(kHeaderEnd) != std::string::npos) {
      break;
    }
  }

  // Accounts for "\r\n\r\n"  and updates buffer and gets request accordingly
  HttpRequest req;
  size_t pos = buffer_.find(kHeaderEnd);
  if (pos == std::string::npos) {
    req = ParseRequest(buffer_);
    buffer_.clear();
  } else {
    req = ParseRequest(buffer_.substr(0, pos));
    buffer_ = buffer_.substr(pos + kHeaderEndLen);
  }

  *request = req;
  return true;
}

bool HttpConnection::WriteResponse(const HttpResponse& response) const {
  string str = response.GenerateResponseString();
  int res = WrappedWrite(fd_,
                         reinterpret_cast<const unsigned char*>(str.c_str()),
                         str.length());
  if (res != static_cast<int>(str.length()))
    return false;
  return true;
}

HttpRequest HttpConnection::ParseRequest(const string& request) const {
  HttpRequest req("/");  // by default, get "/".

  // Plan for STEP 2:
  // 1. Split the request into different lines (split on "\r\n").
  // 2. Extract the URI from the first line and store it in req.URI.
  // 3. For the rest of the lines in the request, track the header name and
  //    value and store them in req.headers_ (e.g. HttpRequest::AddHeader).
  //
  // Hint: Take a look at HttpRequest.h for details about the HTTP header
  // format that you need to parse.
  //
  // You'll probably want to look up boost functions for:
  // - Splitting a string into lines on a "\r\n" delimiter
  // - Trimming whitespace from the end of a string
  // - Converting a string to lowercase.
  //
  // Note: If a header is malformed, skip that line.

  // STEP 2:
  // Splits the request string into different lines using \r\n
  std::vector<string> request_vec;
  split(request_vec, request, is_any_of("\r\n"), token_compress_on);

  if (request_vec.size() < 2) {
    return req;
  }

  // Extract and sets the URI for the HttpRequest
  std::vector<string> first_line;
  split(first_line, request_vec[0], is_any_of(" "), token_compress_on);
  request_vec.erase(request_vec.begin());

  // Not the right format, just return the default request with "\"
  if (first_line.size() < 2 || first_line.size() > 3) {
    return req;
  }
  req.set_uri(first_line[1]);

  // Adds the headers for the HttpRequest
  for (string line : request_vec) {
    size_t pos = line.find(": ");
    if (pos != std::string::npos) {
      string header = line.substr(0, pos);
      string value = line.substr(pos + 2);
      to_lower(header);
      req.AddHeader(header, value);
    }
  }

  return req;
}

}  // namespace hw4
