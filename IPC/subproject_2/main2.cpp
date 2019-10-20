
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
#include <thread>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <mutex>
#include <condition_variable>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <sys/stat.h>


using std::string;

#define MSGSIZE 4

void child(std::string keyword, int p[2]) {

  long long int size;
  char newLine = 1;
  read(p[0], &size, sizeof(size));
  std::cout << "size of sent to child: " << size << std::endl;
  int shm_fd = shm_open("shared", O_RDWR, S_IRUSR | S_IWUSR);
  char *sha_mem = (char*)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  std::vector<std::string> master_vec;
  std::string line = "";

  for(int i = 0; i < size; i++) {
    line += sha_mem[i];
    if ((sha_mem[i] == '\n') || (sha_mem[i] == -1)) {
      master_vec.push_back(line);
      line = "";
    }
    //printf("%i\n", i);
  }
  master_vec.push_back(line);
  printf("%lld", size);
  
  std::cout << "size of master vec: " << master_vec.size() << std::endl; 
   for (int i = 0; i < master_vec.size(); ++i) {
     std::cout << master_vec.at(i);
   }

  
    
  
    
  
  
  
  std::vector<std::string> v1;
  std::vector<std::string> v2;
  std::vector<std::string> v3;
  std::vector<std::string> v4;



}





void parent(std::string file, int p[2]) {
  int fd = open(file.c_str(), O_RDONLY, S_IRUSR | S_IWUSR);
  struct stat sb;
  if (fstat(fd, &sb) == -1) {
		perror("couldn't get file size: ");
		exit(1);
}
int shm_fd = shm_open("shared", O_CREAT | O_RDWR, 0660);
// if(ftruncate(shm_fd, sb.st_size) == -1)
// 		perror("ftruncate");
char *local_mem = (char *)mmap(NULL, sb.st_size,  PROT_READ, MAP_PRIVATE, fd, 0);
char *sha_mem = (char *)mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
memcpy(sha_mem, local_mem, sb.st_size);

// for(int i = 0; i < sb.st_size; ++i) {
//   printf("%c", sha_mem[i]);
// }
write(p[1], &sb.st_size, sizeof(sb.st_size));
}





int main(int argc, char **argv) {
  if(argc =! 3) {
    std::cout << "Need to provide program execution with an input file and a keyword" << std::endl;
  }

  std::string file = argv[1];
  std::string keyword = argv[2];
  
  int p[2]; pipe(p); 

  pid_t pid = fork();

  if (pid == 0) { // Child process 
    child(keyword, p);
  } else if (pid > 0) { // Parent process
    parent(file, p);
  } else { // fork failed
printf("fork() failed!\n");
  }
 
  

    return 0;
}





