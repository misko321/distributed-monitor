#include "process_monitor.h"

#include "packet.h"

#include <mutex>
#include <thread>
#include <mpi.h>

int globalvar = 1;

ProcessMonitor::ProcessMonitor() {
  MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
  MPI_Comm_rank(MPI_COMM_WORLD, &comm_rank);
}

ProcessMonitor::~ProcessMonitor() {
}

ProcessMonitor* ProcessMonitor::pInstance;

ProcessMonitor& ProcessMonitor::instance() {
  static std::mutex mutex;

  if (!pInstance) {
    std::lock_guard<std::mutex> lock(mutex);
    if (!pInstance) {
      pInstance = new ProcessMonitor();
      pInstance->run();
    }
  }

  return *pInstance;
}

void ProcessMonitor::run() {
  monitorThread = std::thread([this]() -> void {
    while(!this->shouldFinish)
      pInstance->receive();
  });
}

void ProcessMonitor::finish() {
  this->shouldFinish = true;
  monitorThread.join();
}

void ProcessMonitor::broadcast(Packet &packet) {
  for (int i = 0; i < comm_size; ++i) {
      if (i != comm_rank) {
          MPI_Send(&packet, sizeof(packet), MPI_BYTE, i, packet.getType(), MPI_COMM_WORLD);
      }
  }
}

void ProcessMonitor::receive() {
  MPI_Status status;
  Packet packet;
  MPI_Recv(&packet, sizeof(packet), MPI_BYTE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
  std::cout << comm_rank << ": received, " << packet.getClock() << std::endl;
  // std::this_thread::sleep_for(std::chrono::milliseconds(100));
  // this->shouldRun = false;
}
