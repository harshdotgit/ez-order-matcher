#include "ipc.h"
#include <fcntl.h>
#include <iostream>
#include <string>
#include <unistd.h>

int main() {
  int fd{open(FIFO_PATH, O_WRONLY | O_NONBLOCK)};
  if (fd < 0) {
    std::cerr << "viewer not running; start ./viewer first\n";
    return 1;
  }
  fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) & ~O_NONBLOCK);

  std::string line;
  std::cout << "> " << std::flush;
  while (std::getline(std::cin, line)) {
    if (line == "quit")
      break;
    line.push_back('\n');
    write(fd, line.data(), line.size());
    std::cout << "> " << std::flush;
  }

  close(fd);
  return 0;
}
