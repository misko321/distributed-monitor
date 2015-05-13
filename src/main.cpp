#include <mpi.h>
#include "distributed_mutex.h"

#include <thread>
#include <chrono>

int main(int argc, char* argv[]) {
  int size, rank;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  std::cout.rdbuf()->pubsetbuf(0, 0);

  std::cout << "size: " << size << ", rank: " << rank << std::endl;

  DistributedMutex distributedMutex(1);
  distributedMutex.acquire();
  std::cout << rank << ": CRITICAL SECTION 1" << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(1));
  distributedMutex.release();
  std::cout << rank << ": BETWEEN" << std::endl;
  distributedMutex.acquire();
  std::cout << rank << ": CRITICAL SECTION 2" << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(1));
  distributedMutex.release();
  // distributedMutex.acquire();
  // std::cout << rank << ": CRITICAL SECTION 3" << std::endl;
  // distributedMutex.release();
  // distributedMutex.acquire();
  // distributedMutex.release();

  std::this_thread::sleep_for(std::chrono::seconds(60));
  MPI_Finalize();
  return 0;
}
