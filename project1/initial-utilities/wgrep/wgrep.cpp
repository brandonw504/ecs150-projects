#include <iostream>

#include <fcntl.h>

#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include <string>

using namespace std;

void readFile(int fileDescriptor, string searchTerm) {
  int sz = searchTerm.size();

  if (fileDescriptor == -1) {
    cout << "wgrep: cannot open file" << endl;
    exit(1);
  }

  char buffer[1];
  string line = "";
  int searchIndex = 0;
  bool printLine = false;

  while (read(fileDescriptor, buffer, sizeof(buffer)) > 0) {
    line += buffer[0];

    if (buffer[0] == searchTerm[searchIndex]) { // The character we just took in matches the search term's next character
      searchIndex++;
      if (searchIndex > sz - 1) { // The searchIndex reached the end of the searchTerm, signifying a match
        printLine = true;
        searchIndex = 0;
      }
    } else if (buffer[0] == '\n') { // Newline encountered
      if (printLine && write(STDOUT_FILENO, line.c_str(), line.size()) < 0) { // Print if match was found
        cout << "wgrep: error writing to stdout" << endl;
        exit(1);
      }

      printLine = false;
      searchIndex = 0;
      line = "";
    } else {
      searchIndex = 0;
    }
  }

  close(fileDescriptor);
}

int main(int argc, char *argv[]) {
  if (argc == 1) {
    cout << "wgrep: searchterm [file ...]" << endl;
    return 1;
  } else if (argc == 2) {
    string searchTerm(argv[1]);
    readFile(STDIN_FILENO, searchTerm);
  } else {
    string searchTerm(argv[1]);
    for (int i = 2; i < argc; i++) {
      readFile(open(argv[i], O_RDONLY), searchTerm);
    }
  }

  return 0;
}