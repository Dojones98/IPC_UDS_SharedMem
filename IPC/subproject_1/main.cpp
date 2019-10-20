
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


void chomp( std::string &s)
{
        int pos;
  while((pos=s.find('\n')) != std::string::npos)
                s.erase(pos);
}


std::vector<std::string> after_search (std::vector<std::string> sentences, std::string key_word) {
  
  std::string sentence;
  std::vector<std::string> result;
  std::regex r("[^A-Za-z]" + key_word + "[^A-Za-z]");
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
      
      
      do {
        read(sv[1], &buf, 1);
        key.append(&buf, 1);
      }while(buf != newline);
      
      
      while(buf != eof) {
          read(sv[1], &buf, 1);
          s.append(&buf, 1);
        if(buf == newline){
          sentences.push_back(s);
          s = "";
          }
        }
      key.pop_back();
      std::vector<std::string> solution = after_search(sentences, key);
      
      for(int i = 0; i < solution.size(); ++i) {
             std::cout << solution.at(i) << std::endl;
           }
      
      std::cout << solution.size() << std::endl;
      
      
      std::string line_to_send;
      for(int i = 0; i < solution.size(); ++i) {
        line_to_send = solution.at(i);
        for(int j = 0; j < line_to_send.size(); ++j) {
          buf = line_to_send.at(j);
          write(sv[1], &buf, 1);
        }
        write(sv[1], &newline, 1);
      }
      write(sv[1], &eof, 1);
      
      
      
      
      
    } else { /* parent */
      
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
        write(sv[0], &buf, 1);
      }
      write(sv[0], &newline, 1);

      
      
      std::string line;
      while(getline(myfile, line)) {
        int line_length = line.length();
        for(int i = 0; i < line.length(); ++i){
          buf = line.at(i);
          write(sv[0], &buf, 1);
        }
         write(sv[0], &newline, 1);
      
      }
      write(sv[0], &eof, 1);
    
///READING FROM
      
      std::vector<std::string> lines_to_sort;
      std::string final_strings;
      std::vector<std::string> lines_to_sort_v2;
      while(buf != eof) {
        read(sv[0], &buf, 1);
        final_strings.append(&buf, 1);
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
      std::cout << lines_to_sort_v2.size() << std::endl;
        wait(NULL); /* wait for child to die */
    }

    return 0;
}




