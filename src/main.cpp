#include <mpi.h>
#include "distributed_mutex.h"

#include <thread>
#include <chrono>

int main(int argc, char* argv[]) {
  int size, rank;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  std::cout << "size: " << size << ", rank: " << rank << std::endl;

  DistributedMutex distributedMutex(1);
  distributedMutex.acquire();
  distributedMutex.release();
  distributedMutex.acquire();
  distributedMutex.release();

  std::this_thread::sleep_for(std::chrono::seconds(60));
  MPI_Finalize();
  return 0;
}
