#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <string.h>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

using namespace std;

void printError() {
  char error_message[30] = "An error has occurred\n";
  write(STDERR_FILENO, error_message, strlen(error_message));
}

void trim(string& s) {
  const string ws = " \t";
  s.erase(0, s.find_first_not_of(ws));
  s.erase(s.find_last_not_of(ws) + 1);
}

vector<string> generateTokensWhitespace(string& line) {
  vector<string> tokens;
  string token = "";
  for (char& c : line) {
    if ((c == ' ' || c == '\t') && token != "") {
      tokens.push_back(token);
      token = "";
    } else if (c != ' ' && c != '\t') {
      token += c;
    }
  }
  tokens.push_back(token);

  return tokens;
}

pid_t executeCommand(string command, vector<string>& paths) {
  vector<string> tokens = generateTokensWhitespace(command);
  
  if (tokens[0] == "exit") {
    if (tokens.size() > 1) {
      printError();
      return -1;
    }
    exit(0);
  } else if (tokens[0] == "cd") {
    if (tokens.size() != 2) {
      printError();
      return -1;
    }

    if (isalnum(tokens[1][0])) tokens[1].insert(0, "./");
    if (chdir(tokens[1].c_str()) == -1) {
      printError();
      return -1;
    }
  } else if (tokens[0] == "path") {
    paths.clear();
    for (size_t i = 1; i < tokens.size(); i++) {
      paths.push_back(tokens[i]);
    }
  } else if (tokens[0] == "echo") {
    pid_t pid = fork();
    if (pid == 0) {
      if (paths.empty()) {
        printError();
        exit(1);
      }

      for (string& path : paths) {
        tokens[0] = path + "/" + tokens[0];
        if (access(tokens[0].c_str(), X_OK) == 0) {
          command.erase(0, 4);
          trim(command);

          char *args[3];
          args[0] = (char*) tokens[0].c_str();
          args[1] = (char*) command.c_str();
          args[2] = NULL;

          if (execv(args[0], args) == -1) {
            printError();
            return -1;
          }
        } else {
          printError();
          return -1;
        }
      }

      exit(0);
    } else {
      return pid;
    }
  } else {
    pid_t pid = fork();
    if (pid == 0) {
      if (paths.empty()) {
        printError();
        exit(1);
      }

      bool ranCommand = false;
      for (string path : paths) {
        if (tokens[0] == "rm" && tokens[1][0] == '-') {
          tokens.erase(tokens.begin() + 1);
        }

        path += "/" + tokens[0];
        if (isalnum(path[0])) path.insert(0, "./");

        if (access(path.c_str(), X_OK) == 0) {
          char **args = new char*[tokens.size()];
          for(size_t i = 0; i < tokens.size(); i++){
            args[i] = new char[tokens[i].size() + 1];
            strcpy(args[i], tokens[i].c_str());
          }

          ranCommand = true;
          if (execv(path.c_str(), args) == -1) {
            printError();
            exit(1);
          }
        }
      }

      if (!ranCommand) {
        printError();
        exit(1);
      }

      exit(0);
    } else {
      return pid;
    }
  }

  return -1;
}

vector<string> generateTokens(string& line, char delim) {
  vector<string> tokens;
  string token = "";
  for (char& c : line) {
    if (c == delim) {
      tokens.push_back(token);
      token = "";
    } else {
      token += c;
    }
  }
  tokens.push_back(token);

  return tokens;
}

pid_t executeChunk(string chunk, vector<string>& paths) {
  int redirectCount = 0;
  for (char& c : chunk) {
    if (c == '>') {
      redirectCount++;
    }
  }

  pid_t pid;
  vector<string> tokens = generateTokens(chunk, '>');
  if (redirectCount == 0 && tokens.size() == 1) {
    trim(tokens[0]);
    pid = executeCommand(tokens[0], paths);
  } else if (redirectCount == 1 && tokens.size() == 2) {
    trim(tokens[0]);
    trim(tokens[1]);

    if (tokens[0] == "" || tokens[1] == "") {
      printError();
      return -1;
    }

    if (tokens[1].find(" ") != string::npos) {
      printError();
      return -1;
    }

    int fileDescriptor = open(tokens[1].c_str(), O_WRONLY | O_CREAT);
    if (fileDescriptor == -1) {
      printError();
      return -1;
    }

    int savedStdout = dup(STDOUT_FILENO);
    if (savedStdout == -1) {
      printError();
      return -1;
    }

    if (dup2(fileDescriptor, STDOUT_FILENO) == -1) {
      printError();
      return -1;
    }

    close(fileDescriptor);
    pid = executeCommand(tokens[0], paths);

    if (dup2(savedStdout, STDOUT_FILENO) == -1) {
      printError();
      return pid;
    }
    close(savedStdout);
  } else {
    printError();
    return -1;
  }

  return pid;
}

void processLine(string line, vector<string>& paths) {
  vector<string> chunks = generateTokens(line, '&');
  vector<pid_t> children;

  for (string& chunk : chunks) {
    trim(chunk);
    if (chunk == "") continue;
    
    children.push_back(executeChunk(chunk, paths));
  }

  for (pid_t pid : children) {
    if (pid != -1 && waitpid(pid, NULL, 0) == -1) {
      printError();
      return;
    }
  }
}

int main(int argc, char *argv[]) {
  vector<string> paths = { "/bin" };

  if (argc == 2) {
    ifstream inFile;
    inFile.open(argv[1]);

    if (inFile.is_open()) {
      string line;
      while (getline(inFile, line)) {
        processLine(line, paths);
      }
    } else {
      printError();
      exit(1);
    }

    inFile.close();
  } else if (argc == 1) {
    while (true) {
      cout << "wish> ";

      string line;
      getline(cin, line);
      processLine(line, paths);
    }
  } else {
    printError();
    exit(1);
  }

  return 0;
}