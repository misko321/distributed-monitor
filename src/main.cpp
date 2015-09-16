#include <mpi.h>
#include <thread>
#include <chrono>

#include "distributed_resource.h"

#define RES_ID 1234

/* This example represents a sort of producer-consumer problem. All processes share one integer number.
Producers create a single unit every time they gain access to critical section. Consumers in turn,
need 5 units for their further work. To this end, consumers wait on a distributed conditional variable
implemented inside 'DistributedResource' class.
There are 3 consumers in following example, the rest of the processes are producers.
*/
int main(int argc, char* argv[]) {
  int size, rank;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  std::cout.rdbuf()->pubsetbuf(0, 0); //disable output buffering for quicker printing

  std::cout << rank << ": MPI Initialized successfully :: size: " << size << ", rank: " << rank << std::endl;

  int num = 0;
  DistributedResource resource(RES_ID, &num, sizeof(num));

    while (true) {
      if (rank < 3) { //3 consumers
        resource.lock();
        resource.wait([&num] () -> bool { //consumer need 5 units to work
          return num >= 5;
        });
        while (num > 4) //consume all available units in 5 unit batches
          num -= 5;
        std::cout << rank << ": <--- CRITICAL SECTION :: num = " << num << " CONSUMING (" << rank << ")" << std::endl;
        resource.notify();
        resource.unlock();
      }
      else { //rest of processes are producers
        resource.lock();
        num += 1; //produce 1 unit
        //a little sleep so messages from particular processed don't mix
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        std::cout << rank << ": <--- CRITICAL SECTION :: num = " << num << std::endl;
        if (num >= 5)
          resource.notify();

        resource.unlock();
      }
    }

  std::this_thread::sleep_for(std::chrono::seconds(1000));
  MPI_Finalize();
  return 0;
}
