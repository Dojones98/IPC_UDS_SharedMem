
#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
#include <regex>
#include <thread>
#include <sys/shm.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pthread.h>
#define MSGSIZE 4
#define MEMORY_NAME "/shared_memory"

// This thread function takes in a reference to a vector as its only parameter. 
// From this, it reads the last element and sets it as the keyword, pops that element
// off of the back of the vector, and uses regualr expressions to find elements of the
// vector that contain the keyword. these lines are pushed back to a result vector, and 
// the vector passed in is set to the result vector.

void thread_function(std::vector<std::string> &work_done) {
    std::string keyword = work_done.back(); work_done.pop_back();
    std::regex r("[^A-Za-z]" + keyword + "[^A-Za-z]", std::regex_constants::icase);
    std::smatch m;
    std::vector<std::string> result;
    for (int i = 0; i < work_done.size(); ++i) {
      if(std::regex_search(work_done.at(i),m,r)) 
          result.push_back(work_done.at(i));
    }
    work_done = result;
}

// This is the function for the child process, it takes in the keyword, and two integer 
// arrays for IPC with pipes. This method reads in the size of memory to be allocated and 
// then opens a block of sahred memory and maps that using the size of memory to be allocated.
// It then builds a vector containing all of lines of the file, splits that into 4 smaller 
// vectors, and passes those vectors to threads to map the work into 4 equal parts.
// The vectors are then added back together after the threads complete their work, and the 
// child passes the size of vector back to the parent after placing the final vector into 
// shared memory.

void child(std::string keyword, int p[2], int q[2]) {
  long long int size;
  read(p[0], &size, sizeof(size));
  int shm_fd = shm_open(MEMORY_NAME, O_RDWR, S_IRUSR | S_IWUSR);
  char *sha_mem = (char*)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  std::vector<std::string> master_vec;
  std::string line = "";

  for(int i = 0; i < size; i++) {
    line += sha_mem[i];
    if ((sha_mem[i] == '\n') || (sha_mem[i] == -1)) {
      master_vec.push_back(line);
      line = "";
    }
  }
  master_vec.push_back(line);

  std::vector<std::string> v1, v2, v3, v4;

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
// push the keyword back onto the vector so the threads only have one parameter
    v1.push_back(keyword); v2.push_back(keyword); v3.push_back(keyword); v4.push_back(keyword);


  // Create the threads and pass the mapped vectors to them
    std::thread thread1 (thread_function, std::ref(v1));
    std::thread thread2 (thread_function, std::ref(v2));
    std::thread thread3 (thread_function, std::ref(v3));
    std::thread thread4 (thread_function, std::ref(v4));
// Join the threads back to the main thread
      thread1.join(); thread2.join(); thread3.join(); thread4.join();

// combine the vectors back together; "reduce" in map-reduce
    std::move(v2.begin(), v2.end(), std::back_inserter(v1));
    std::move(v3.begin(), v3.end(), std::back_inserter(v1));
    std::move(v4.begin(), v4.end(), std::back_inserter(v1));

  
// use a tracker variable to find out how much memory the parent should look for
  long long int tracker = 0;
  for (int i = 0; i < v1.size(); ++i) {
    for (int j = 0; j < v1.at(i).length(); ++j) {
         sha_mem[tracker] = v1.at(i).at(j);
         ++tracker;
    }
  }

  write(q[1], &tracker, sizeof(tracker));
   
}


// Parent takes in a file name, and two integer arrays for pipes. Parent opens a file descriptor,
// then uses a stat struct to find the size of the file to be used in allocatinga block of shared 
// memory. The file is mapped into local memory, and a block of shared memory is made before memcpy
// is used to copy the file from local memory to shared memory. After this, the parent uses a pipe to
// write the size of the file to the child.

// After the child does its work, the parent recieves the new size of the information in memory and 
// uses that to rebuild the strings in a vector. The parent then sorts the vector of strings alphabetically 
// and prints them.

void parent(std::string file, int p[2], int q[2]) {
  int fd = open(file.c_str(), O_RDONLY, S_IRUSR | S_IWUSR);
  struct stat sb;
  if (fstat(fd, &sb) == -1) {
		perror("couldn't get file size: ");
		exit(1);
}
int shm_fd = shm_open(MEMORY_NAME, O_CREAT | O_RDWR, 0660);
 if(ftruncate(shm_fd, sb.st_size) == -1)
 		perror("ftruncate");
char *local_mem = (char *)mmap(NULL, sb.st_size,  PROT_READ, MAP_PRIVATE, fd, 0);
char *sha_mem = (char *)mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
memcpy(sha_mem, local_mem, sb.st_size);


long long int end_size = 0;
std::string line = "";
std::vector<std::string> final;
write(p[1], &sb.st_size, sizeof(sb.st_size));
read(q[0], &end_size, sizeof(end_size));

for(int i = 0; i < end_size; i++) {
    line += sha_mem[i];
    if ((sha_mem[i] == '\n') || (sha_mem[i] == -1)) {
      final.push_back(line);
      line = "";
    }
}
sort(final.begin(), final.end());

for(int i = 0; i < final.size(); ++i) {
  std::cout << final.at(i);
}
if (shm_unlink(MEMORY_NAME) != 0) {
		perror("unlink");
		exit(1);
	}
}
// In the main method, arg 1 is set to the file, arg 2 is set to the keyword, and two pipes are created.
// Then a fork() is done, and the child and parent processes start doing their work.

int main(int argc, char **argv) {
  if(!(argc == 3)) {
    std::cout << "Program needs an inputfile and keyword for execution." << std::endl;
  }
  std::string file = argv[1];
  std::string keyword = argv[2];

  int p[2]; pipe(p); 
  int q[2]; pipe(q);

  pid_t pid = fork();
  if (pid == 0) { // Child process 
      child(keyword, p, q);
  } else if (pid > 0) { // Parent process
            parent(file, p, q);
  } else { // fork failed
            printf("fork() failed!\n");
            return 1;
  }
    return 0;
}





