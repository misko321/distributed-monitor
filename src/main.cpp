#include <mpi.h>
#include "distributed_mutex.h"

#include <thread>
#include <chrono>

void producer() {

}

void consumer() {

}


/* main function contains example of how to use DistributedMutex. It's based on producer-consumer
problem with array used as cyclic buffer. Two separate buffers are utilized to present
the possibility of blocking multiple independent resources separately (identified by resourceId). */
int main(int argc, char* argv[]) {
  int size, rank;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  std::cout.rdbuf()->pubsetbuf(0, 0);

  std::cout << "MPI Initialized successfully :: size: " << size << ", rank: " << rank << std::endl;

  //DistributedMutex utilizes C++ RAII concept; if it's used in a block, all allocated resources
  //are released when the block reaches end and variables get out of the scope
  {
    DistributedMutex queueMutex1(1);
    DistributedMutex queueMutex2(2);

    queueMutex1.acquire();
    std::cout << rank << ": CRITICAL SECTION 1" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    queueMutex1.release();
    std::cout << rank << ": BETWEEN" << std::endl;
    queueMutex1.acquire();
    std::cout << rank << ": CRITICAL SECTION 2" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    queueMutex1.release();
  }

  MPI_Finalize();
  return 0;
}
