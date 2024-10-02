#include <iostream>

#include <fcntl.h>

#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

using namespace std;

int main(int argc, char *argv[]) {
  for (int i = 1; i < argc; i++) {
    int fileDescriptor = open(argv[i], O_RDONLY);
    if (fileDescriptor == -1) {
      cout << "wcat: cannot open file" << endl;
      return 1;
    }

    char buffer[4096];
    int ret;
    while ((ret = read(fileDescriptor, buffer, sizeof(buffer))) > 0) {
      if (write(STDOUT_FILENO, buffer, ret) < 0) {
        cout << "wcat: error writing to stdout" << endl;
        return 1;
      }
    }
  }

  return 0;
}