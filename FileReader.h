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

#ifndef HW4_FILEREADER_H_
#define HW4_FILEREADER_H_

#include <string>

namespace hw4 {

// This class is used to read a file into memory and return its
// contents as a string.
class FileReader {
 public:
  // "base_dir" is the directory inside of which we will look for
  // files, and "file_name" is a filename relative to that directory.
  // For example, if:
  //
  //   base_dir is      "./hw4_htmldir"
  //   file_name is     "test/foo.html"
  //
  // then we would read in "./hw4_htmldir/test/foo.html"
  FileReader(const std::string& base_dir, const std::string& file_name)
    : basedir_(base_dir), fname_(file_name) { }
  virtual ~FileReader() { }

  // Attempts to reads in the file specified by the constructor arguments.
  //
  // Return false if:
  // 1. The file could not be found or opened
  // 2. The file exists above the basedir in the file system hierarchy (e.g.,
  //    filename is "../../../escape.html")
  // Otherwise, return true and use output parameter "contents" to return the
  // contents of the file.
  bool ReadFile(std::string* const contents);

 private:
  std::string basedir_;
  std::string fname_;
};

}  // namespace hw4

#endif  // HW4_FILEREADER_H_
