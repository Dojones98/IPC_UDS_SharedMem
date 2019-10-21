
#include <string>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <vector>
#include <sstream>
#include <algorithm>
#include <regex>
#define BUFFER_SIZE 1



// This function takes in a vector of strings and a keyword. It then uses regular expressions to 
// search for the keyword in each string. If the keyword is found, it is then pushed to a result 
// vector which is returned.

std::vector<std::string> after_search (std::vector<std::string> sentences, std::string key_word) {
  std::string sentence;
  std::vector<std::string> result;
  std::regex r("[^A-Za-z]" + key_word + "[^A-Za-z]", std::regex_constants::icase);
  std::smatch m;

    for(int i = 0; i < sentences.size(); i++) {
      sentence = sentences.at(i);
      if (std::regex_search(sentence,m,r)) {
        result.push_back(sentences.at(i));
      }
  }
    return result;
  }


int main(void)
{
    int sv[2]; /* the pair of socket descriptors */
    char buf = 127; /* for data exchange between processes */
    char newline = 1;
    char eof = 0;

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == -1) {
        perror("socketpair");
        exit(1);
    }

    if (!fork()) {  /* child */
      
      std::string s = "";
      std::string key = "";
      
      std::vector<std::string> sentences;
      
      // Do-while loop reads from the parent to get the keyword. After the first 
      // newline character is detected, the child knows that everthing after will be 
      // the file.

      do {
        read(sv[1], &buf, BUFFER_SIZE);
        key.append(&buf, BUFFER_SIZE);
      }while(buf != newline);
      
      // While loop reads until the end-of-file character is observed in the UDS.
      // characters are appended to a string until a newline is detected, then the 
      // string is pushed back to a vector that contains the lines of the file.

      while(buf != eof) {
          read(sv[1], &buf, BUFFER_SIZE);
          s.append(&buf, BUFFER_SIZE);
        if(buf == newline){
          sentences.push_back(s);
          s = "";
          }
        }

      // the key has a carraige return character appended on the end of it, so the
      // pop_back function removes the end character  
      key.pop_back();
      
      // solution vector is set to the vector returned when the after_search function
      // is called.
      std::vector<std::string> solution = after_search(sentences, key);
      

      // lines are then sent back to the parent process in a similar way they were sent 
      // to the child.
      std::string line_to_send;
      for(int i = 0; i < solution.size(); ++i) {
        line_to_send = solution.at(i);
        for(int j = 0; j < line_to_send.size(); ++j) {
          buf = line_to_send.at(j);
          write(sv[1], &buf, BUFFER_SIZE);
        }
        write(sv[1], &newline, BUFFER_SIZE);
      }
      write(sv[1], &eof, BUFFER_SIZE);
      
      
      
// Parent process takes the filename and the keyword from standard input 
// it then creates an ifstream and opens the file. The file is then sent to the 
// child process via Unix Domain Socket, one character at a time.
      
    } else { /* parent */
      //
      std::cout << "input filename" << std::endl;
      std::string file_name;
      std::cin >> file_name;
      std::cout << "input word to be searched" << std::endl;
      std::string key_word;
      std::cin >> key_word;
    
      std::ifstream myfile;
      myfile.open(file_name);
      
      for(int i = 0; i < key_word.size(); ++i) {
        buf = key_word.at(i);
        write(sv[0], &buf, BUFFER_SIZE);
      }
      write(sv[0], &newline, BUFFER_SIZE);

      
      
      std::string line;
      while(getline(myfile, line)) {
        for(int i = 0; i < line.length(); ++i){
          buf = line.at(i);
          write(sv[0], &buf, BUFFER_SIZE);
        }
         write(sv[0], &newline, BUFFER_SIZE);
      
      }
      write(sv[0], &eof, BUFFER_SIZE);
    
///READING FROM
      
      std::vector<std::string> lines_to_sort;
      std::string final_strings;
      std::vector<std::string> lines_to_sort_v2;
      while(buf != eof) {
        read(sv[0], &buf, 1);
        final_strings.append(&buf, BUFFER_SIZE);
      if(buf == newline){
        lines_to_sort.push_back(final_strings);
        final_strings = "";
        }
      }
      
      for(int i = 0; i < lines_to_sort.size()-1; i=i+2){
        lines_to_sort_v2.push_back(lines_to_sort.at(i));
      }
      
      sort(lines_to_sort_v2.begin(), lines_to_sort_v2.end());
        
        for(int i = 0; i < lines_to_sort_v2.size(); ++i){
          std::cout << lines_to_sort_v2.at(i) << std::endl;
        }
      
        wait(NULL); /* wait for child to die */
    }

    return 0;
}




