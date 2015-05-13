#include "process_monitor.h"

#include "packet.h"

#include <mutex>
#include <thread>
#include <mpi.h>

ProcessMonitor::ProcessMonitor() {
  MPI_Comm_size(MPI_COMM_WORLD, &commSize);
  MPI_Comm_rank(MPI_COMM_WORLD, &commRank);
}

ProcessMonitor::~ProcessMonitor() {
}

ProcessMonitor* ProcessMonitor::pInstance;

//Singleton Design Pattern
ProcessMonitor& ProcessMonitor::instance() {
  static std::mutex guard_;
  if (!pInstance) {
    std::lock_guard<std::mutex> lock(guard_);
      if (!pInstance) {
      pInstance = new ProcessMonitor();
      pInstance->run();
    }
  }

  return *pInstance;
}

int ProcessMonitor::getCommSize() {
  return commSize;
}

int ProcessMonitor::getCommRank() {
  return commRank;
}

void ProcessMonitor::run() {
  monitorThread = std::thread([this]() -> void {
    while(!this->shouldFinish)
      this->pInstance->receive();
  });
}

void ProcessMonitor::finish() {
  this->shouldFinish = true;
  monitorThread.join();
  delete pInstance;
}

void ProcessMonitor::addMutex(DistributedMutex& mutex) {
  //synchronization in case, the end-user uses multiple threads that add mutexes
  std::lock_guard<std::mutex> lock(guard);

  // std::cout << "addMutex" << std::endl;
  if (resToMutex.find(mutex.getResourceId()) == resToMutex.end()) {
    // std::cout << "addMutex inside if" << std::endl;
    std::pair<unsigned int, DistributedMutex&> pair(mutex.getResourceId(), mutex);
    resToMutex.insert(pair);
  }
}

void ProcessMonitor::removeMutex(DistributedMutex& mutex) {
  //synchronization in case, the end-user uses multiple threads that add mutexes
  std::lock_guard<std::mutex> lock(guard);

  resToMutex.erase(mutex.getResourceId());
}

void ProcessMonitor::broadcast(Packet &packet) {
  for (int i = 0; i < commSize; ++i) {
      if (i != commRank) {
          MPI_Send(&packet, sizeof(packet), MPI_BYTE, i, packet.getType(), MPI_COMM_WORLD);
      }
  }
}

void ProcessMonitor::send(int destination, Packet &packet) {
  MPI_Send(&packet, sizeof(packet), MPI_BYTE, destination, packet.getType(), MPI_COMM_WORLD);
}

void ProcessMonitor::receive() {
  MPI_Status status;
  Packet packet;
  // std::cout << "beforeMPIRecv" << std::endl;
  MPI_Recv(&packet, sizeof(packet), MPI_BYTE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
  // std::cout << commRank << ": received packet from: " << status.MPI_SOURCE
  //   << ", with clock: " << packet.getClock() << std::endl;
  //TODO
  //niekoniecznie notify
  //notify wtedy, jesli jest to REPLY -> osobna metoda onReply() w mutexie
  //jeśli REQUEST, to odpowiadamy REPLY bądź wstrzymujemy -> osobna metoda onRequest
  //jeśli ani to, ani to (czyli komunikat użytkownika) to przekazujemy jak?
  //resToMutex[packet.getResourceId()].notify();
  // std::cout << "beforeIter: " << packet.getResourceId() << ", " << resToMutex.size() << std::endl;
  std::unordered_map<unsigned int, DistributedMutex&>::iterator mutexIter = resToMutex.find(packet.getResourceId());
  if (mutexIter == resToMutex.end())
    std::cout << "not found" << std::endl;
  // std::cout << "afterIter: " << mutexIter->first << "->" << std::endl;
  switch(packet.getType()) {
    case Packet::Type::DM_REPLY: {
      mutexIter->second.onReply(status.MPI_SOURCE);
      break;
    }
    case Packet::Type::DM_REQUEST: {
      mutexIter->second.onRequest(status.MPI_SOURCE, packet.getClock());
      break;
    }
  }

  // std::this_thread::sleep_for(std::chrono::milliseconds(100));
  // this->shouldRun = false;
}
