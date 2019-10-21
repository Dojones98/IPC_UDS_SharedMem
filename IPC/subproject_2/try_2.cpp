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
#include <pthread.h>


using std::string;

#define MSGSIZE 4

struct thread_args {
  std::vector<std::string> lines;
  std::string key;
};
struct thread_args Struct_thread_1;
struct thread_args Struct_thread_2;
struct thread_args Struct_thread_3;
struct thread_args Struct_thread_4;



void thread_function(std::vector<std::string> &work_done) {
    //std::vector<std::string>* work_done = static_cast<std::vector<std::string>*>(args);
    std::string keyword = work_done.back(); work_done.pop_back();
    std::regex r("[^A-Za-z]" + keyword + "[^A-Za-z]");
    std::smatch m;
    std::vector<std::string> result;
    for (int i = 0; i < work_done.size(); ++i) {
      if(std::regex_search(work_done.at(i),m,r)) 
          result.push_back(work_done.at(i));
    }
    work_done = result;
}

void child(std::string keyword, int p[2]) {

  long long int size;
  char newLine = 1;
  read(p[0], &size, sizeof(size));
  std::cout << "size of sent to child: " << size << std::endl;
  int shm_fd = shm_open("/fuckyoufuckyoufuckyou", O_RDWR, S_IRUSR | S_IWUSR);
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

  std::vector<std::string> v1;
  std::vector<std::string> v2;
  std::vector<std::string> v3;
  std::vector<std::string> v4;

   for (int i = 0; i < master_vec.size(); ++i) {
     if(i < (master_vec.size()/4) ) {
        v1.push_back(master_vec.at(i));
     } else if ((i >= (master_vec.size()/4)) && (i < master_vec.size()/2)) {
        v2.push_back(master_vec.at(i));
     } else if ((i >= master_vec.size()/2) && (i < 3*(master_vec.size()/4))) {
        v3.push_back(master_vec.at(i));
     }else {
        v4.push_back(master_vec.at(i));
     }
   }

   std::cout << "\n Size of cat-ed vector is: " << v1.size() << std::endl;
   v1.push_back(keyword); v2.push_back(keyword); v3.push_back(keyword); v4.push_back(keyword);


  std::thread thread1 (thread_function, std::ref(v1));
  std::thread thread2 (thread_function, std::ref(v2));
  std::thread thread3 (thread_function, std::ref(v3));
  std::thread thread4 (thread_function, std::ref(v4));

  thread1.join(); thread2.join(); thread3.join(); thread4.join();

  std::cout << "\n Size of pre cat vector is: " << v1.size() << std::endl;

  std::move(v2.begin(), v2.end(), std::back_inserter(v1));
  std::move(v3.begin(), v3.end(), std::back_inserter(v1));
  std::move(v4.begin(), v4.end(), std::back_inserter(v1));

  std::cout << "\n Size of cat-ed vector is: " << v1.size() << std::endl;
   
}


void parent(std::string file, int p[2]) {
  int fd = open(file.c_str(), O_RDONLY, S_IRUSR | S_IWUSR);
  struct stat sb;
  if (fstat(fd, &sb) == -1) {
		perror("couldn't get file size: ");
		exit(1);
}
int shm_fd = shm_open("/fuckyoufuckyoufuckyou", O_CREAT | O_RDWR, 0660);
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
