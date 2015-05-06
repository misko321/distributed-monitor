#include "distributed_mutex.h"

int main(int argc, char* argv[]) {
  DistributedMutex distributedMutex;
  distributedMutex.acquire();
  distributedMutex.release();
  
  return 0;
}
