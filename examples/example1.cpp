#include <mpi.h>
#include <thread>
#include <chrono>

#include "distributed_resource.h"

#define NUM1 1
#define NUM2 2

/* TODO main function contains example of how to use DistributedMutex. It's based on producer-consumer
problem with array used as cyclic buffer. Two separate buffers are utilized to present
the possibility of blocking multiple independent resources separately (identified by resourceId). */
int main(int argc, char* argv[]) {
  int size, rank;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  std::cout.rdbuf()->pubsetbuf(0, 0);

  std::cout << "MPI Initialized successfully :: size: " << size << ", rank: " << rank << std::endl;

  {
    int num1[2] = { 0, 0 };
    DistributedResource resource1(NUM1, &num1, sizeof(num1));

    while (true) {
      resource1.lock();
      ++num1[0];
      num1[1] += 10;
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
      std::cout << rank << ": <--- CRITICAL SECTION :: num1 = " << num1[0] << ", " << num1[1] << std::endl;
      resource1.unlock();
    }
  }

  std::this_thread::sleep_for(std::chrono::seconds(1000));
  MPI_Finalize();
  return 0;
}
