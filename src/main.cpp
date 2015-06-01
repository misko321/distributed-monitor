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
  // {
  //   DistributedMutex queueMutex1(1);
  //   DistributedMutex queueMutex2(2);
  //
  //   queueMutex1.acquire();
  //   std::cout << rank << ": CRITICAL SECTION 1" << std::endl;
  //   std::this_thread::sleep_for(std::chrono::milliseconds(500));
  //   queueMutex1.release();
  //   queueMutex1.acquire();
  //   std::cout << rank << ": CRITICAL SECTION 2" << std::endl;
  //   std::this_thread::sleep_for(std::chrono::milliseconds(500));
  //   queueMutex1.release();
  // }

  // {
  //   DistributedMutex mutex(1);
  //   DistributedCondvar condvar(1);
  //   if (rank % 2)
  //     condvar.wait(mutex, [] () -> bool { return true; });
  // }
  // std::cout << rank << ": DONE" << std::endl;
  //TODO remove everything at out of scope
  {
    int num1[2] = { -100, 1000 };
    DistributedResource resource1(NUM1, &num1, sizeof(num1));
    // std::cout << rank << ": " << reinterpret_cast<long>(&num1) << std::endl;
    // MPI_Barrier(MPI_COMM_WORLD);
    // std::this_thread::sleep_for(std::chrono::milliseconds(500));
    // DistributedResource resource2(NUM2, &num, sizeof(num));

      while (true) {
      // for (int i = 0; i < 5; ++i) {
        resource1.lock();
        // std::cout << rank << ": ---> CRITICAL SECTION :: num1 = " << num1 << std::endl;
        --num1[0];
        num1[1] += 10;
        // if (rank != 0)
          std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << rank << ": <--- CRITICAL SECTION :: num1 = " << num1[0] << ", " << num1[1] << std::endl;
        // std::cout << rank << ": " << reinterpret_cast<long>(&num1) << std::endl;
        resource1.unlock();
        // break;
        // resource2.lock();
        // // std::cout << rank << ": ---> CRITICAL SECTION :: num2 = " << num2 << std::endl;
        // num2 += 10;
        // std::this_thread::sleep_for(std::chrono::milliseconds(500));
        // std::cout << rank << ": <--- CRITICAL SECTION :: num2 = " << num2 << std::endl;
        // resource2.unlock();
      }
  }

  std::this_thread::sleep_for(std::chrono::seconds(1000));
  MPI_Finalize();
  return 0;
}
