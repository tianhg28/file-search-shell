
# HTTP333 Web Server

## Description
HTTP333 is a full-stack software application crafted in C/C++ for querying and searching files by keywords in specified directories. It features a multithreaded web server capable of handling and parsing HTTP requests using a custom server socket and file crawler.

## Features
- **File Querying and Searching**: Allows users to search for files by keywords within specified directories.
- **Multithreaded Web Server**: Capable of handling multiple HTTP requests concurrently.
- **Custom Server Socket and File Crawler**: Uses a bespoke implementation for socket handling and file crawling.
- **C/C++ File I/O**: Efficiently utilizes file input/output operations for storing data structures on disk and retrieving files based on user queries.

## Installation
1. Ensure you have a C/C++ compiler installed on your system.
2. Clone or download the source code from the repository.
3. Compile the source files using your C/C++ compiler. For example:
   ```
   g++ -o http333 http333.cc ServerSocket.cc HttpServer.cc -lpthread
   ```

## Usage
To run the HTTP333 web server:
```
./http333 <port_number> <static_files_directory> <index_files+>
```
- `port_number`: The port number on which the server will listen.
- `static_files_directory`: The directory containing static files for the server.
- `index_files`: One or more index files for the server to use.

## Components
- `http333.cc`: The main file that initializes the server and parses command-line arguments.
- `ServerSocket.cc`: Handles the server socket operations.
- `HttpServer.cc`: Manages HTTP request parsing and file retrieval.

## Contributing
Contributions to the HTTP333 project are welcome. Please feel free to fork the repository, make your changes, and submit a pull request.

## License
This project is licensed under the MIT License. See the LICENSE file for more details.

---

**Note**: This README is for instructional use only. Please ensure you have permission to use and distribute any third-party components included in this project.
