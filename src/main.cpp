#include <mpi.h>
#include <thread>
#include <chrono>

#include "distributed_resource.h"

#define NUM1 41313
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

  std::cout << rank << ": MPI Initialized successfully :: size: " << size << ", rank: " << rank << std::endl;

  //DistributedMutex utilizes C++ RAII concept; if it's used in a block, all allocated resources
  //are released when the block reaches end and variables get out of the scope

  //TODO remove everything at out of scope
  {
    int num = 0;
    DistributedResource resource(NUM1, &num, sizeof(num));

      while (true) {
      // for (int i = 0; i < 5; ++i)
        // std::cout << rank << ": ---> CRITICAL SECTION :: num1 = " << num1 << std::endl;
        if (rank < 3) {
          resource.lock();
          // std::cout << rank << " lock\n";
          resource.wait([&num] () -> bool {
            return num >= 5;
          });
          // std::cout << rank << " locked\n";
          while (num > 4)
            num -= 5;
          std::cout << rank << ": <--- CRITICAL SECTION :: num = " << num << " CONSUMING (" << rank << ")" << std::endl;
          resource.notify();
          resource.unlock();
        }
        else {
          resource.lock();
          num += 1;
          std::this_thread::sleep_for(std::chrono::milliseconds(100));
          std::cout << rank << ": <--- CRITICAL SECTION :: num = " << num << std::endl;
          if (num >= 5)
            resource.notify();

          // std::cout << rank << ": " << reinterpret_cast<long>(&num1) << std::endl;
          resource.unlock();
        }
      }
  }

  std::this_thread::sleep_for(std::chrono::seconds(1000));
  MPI_Finalize();
  return 0;
}
