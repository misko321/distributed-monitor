#include <mpi.h>
#include "distributed_mutex.h"
using std::cout;
using std::endl;

int main(int argc, char* argv[]) {
  int size, rank;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  cout << "size: " << size << ", rank: " << rank << endl;

  DistributedMutex distributedMutex;
  distributedMutex.acquire();
  distributedMutex.release();
  distributedMutex.acquire();
  distributedMutex.release();

  while(true);
  MPI_Finalize();
  return 0;
}
