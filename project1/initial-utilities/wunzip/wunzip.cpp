#include <iostream>

#include <fcntl.h>

#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include <string>

using namespace std;

int main(int argc, char *argv[]) {
  if (argc == 1) {
    cout << "wunzip: file1 [file2 ...]" << endl;
    return 1;
  } else {
    for (int i = 1; i < argc; i++) {
      int fileDescriptor = open(argv[i], O_RDONLY);

      int count;
      char letter;
      while (read(fileDescriptor, &count, 4) > 0 && read(fileDescriptor, &letter, 1) > 0) {
        for (int i = 0; i < count; i++) {
          if (write(STDOUT_FILENO, &letter, 1) < 0) {
            cout << "wunzip: error writing to stdout" << endl;
            return 1;
          }
        }
      }

      close(fileDescriptor);
    }
  }

  return 0;
}