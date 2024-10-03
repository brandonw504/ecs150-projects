#include <iostream>

#include <fcntl.h>

#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include <string>

using namespace std;

int main(int argc, char *argv[]) {
  if (argc == 1) {
    cout << "wzip: file1 [file2 ...]" << endl;
    return 1;
  } else {
    int count = 0;
    char prevChar = '$';

    for (int i = 1; i < argc; i++) {
      int fileDescriptor = open(argv[i], O_RDONLY);

      char buffer[1];
      while (read(fileDescriptor, buffer, sizeof(buffer)) > 0) {
        if (buffer[0] == prevChar) { // character repeated
          count++;
        } else { // different character
          if (prevChar != '$') { // if we're not on the first character, try writing
            if (write(STDOUT_FILENO, &count, 4) < 0) {
              cout << "wzip: error writing to stdout" << endl;
              return 1;
            }

            if (write(STDOUT_FILENO, &prevChar, 1) < 0) {
              cout << "wzip: error writing to stdout" << endl;
              return 1;
            }
          }
          prevChar = buffer[0];
          count = 1;
        }
      }

      close(fileDescriptor);
    }

    if (prevChar != '$') {
      if (write(STDOUT_FILENO, &count, 4) < 0) {
        cout << "wzip: error writing to stdout" << endl;
        return 1;
      }

      if (write(STDOUT_FILENO, &prevChar, 1) < 0) {
        cout << "wzip: error writing to stdout" << endl;
        return 1;
      }
    }
  }

  return 0;
}