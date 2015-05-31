#include "process_monitor.h"

#include "packet.h"
#include "distributed_resource.h"

#include <mutex>
#include <thread>
#include <mpi.h>

#define PACKET_TAG 0

ProcessMonitor* ProcessMonitor::pInstance;

ProcessMonitor::ProcessMonitor() {
  MPI_Comm_size(MPI_COMM_WORLD, &commSize);
  MPI_Comm_rank(MPI_COMM_WORLD, &commRank);
}

ProcessMonitor::~ProcessMonitor() {
  std::cout << commRank << ": ~ProcessMonitor()" << std::endl;
}

int ProcessMonitor::getCommSize() {
  return commSize;
}

int ProcessMonitor::getCommRank() {
  return commRank;
}

//Singleton Design Pattern
ProcessMonitor& ProcessMonitor::instance() {
  static std::mutex guard_;

  if (!pInstance) {
    std::lock_guard<std::mutex> lock(guard_);
      if (!pInstance) {
        ProcessMonitor* inst = new ProcessMonitor();
        inst->run();
        pInstance = inst;
    }
  }

  return *pInstance;
}

void ProcessMonitor::addResource(DistributedResource& resource) {
  //synchronization in case, the end-user uses multiple threads that add mutexes
  std::lock_guard<std::mutex> lock(guard);

  // std::cout << "addMutex" << std::endl;
  if (idToRes.find(resource.getId()) == idToRes.end()) {
    // std::cout << "addMutex inside if" << std::endl;
    idToRes.emplace(resource.getId(), resource);
  }
}

void ProcessMonitor::removeResource(DistributedResource& resource) {
  //synchronization in case, the end-user uses multiple threads that add mutexes
  std::lock_guard<std::mutex> lock(guard);

  idToRes.erase(resource.getId());

  /* If there are no more mutexes to watch, destroy ProcessMonitor
  WARNING: this feature allows for destroying the process monitor and deallocate used resources,
  but since the monitor will not exist anymore, other processes may block on waiting for messages
  from it. Uncomment if you know what you're doin'. */
  //if (resToMutex.size() == 0)
  //  finish();
}

void ProcessMonitor::finish() {
  std::cout << commRank << ": finish()" << std::endl;
  this->shouldFinish = true;
  monitorThread.join();
  delete pInstance;
}

void ProcessMonitor::broadcastPacket(Packet &packet) {
  for (int i = 0; i < commSize; ++i) {
      if (i != commRank)
        MPI_Send(&packet, sizeof(packet), MPI_BYTE, i, PACKET_TAG, MPI_COMM_WORLD);
  }
}

void ProcessMonitor::sendPacket(int destination, Packet &packet) {
  MPI_Send(&packet, sizeof(packet), MPI_BYTE, destination, PACKET_TAG, MPI_COMM_WORLD);
}

void ProcessMonitor::broadcastResource(int id, void* resource, int size) {
  // std::cout << commRank << ": " << id << " " << size << std::endl;
  for (int i = 0; i < commSize; ++i) {
    if (i != commRank)
      MPI_Send(resource, size, MPI_BYTE, i, id, MPI_COMM_WORLD);
  }
}

void ProcessMonitor::run() {
  monitorThread = std::thread([this]() -> void {
    int flag;
    MPI_Status status;

    while(!this->shouldFinish) {
      MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
      if (flag) {
        if (status.MPI_TAG == PACKET_TAG)
          this->pInstance->receivePacket();
        else
          this->pInstance->receiveResource(status.MPI_TAG);
      }
    }
  });
}

void ProcessMonitor::receivePacket() {
  // std::cout << commRank << ": receivePacket" << std::endl;
  MPI_Status status;
  Packet packet;
  MPI_Recv(&packet, sizeof(packet), MPI_BYTE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
  auto resourceIter = idToRes.find(packet.getResourceId());
  //TODO send immidiate reply if no resource found

  switch (packet.getType()) {
    case Packet::Type::DM_REQUEST: {
      resourceIter->second.mutex->onRequest(status.MPI_SOURCE, packet.getClock());
      break;
    }
    case Packet::Type::DM_REPLY: {
      resourceIter->second.mutex->onReply(status.MPI_SOURCE);
      break;
    }
    case Packet::Type::DM_CONDVAR_WAIT: {
      resourceIter->second.condvar->onWait(status.MPI_SOURCE);
      break;
    }
    case Packet::Type::DM_CONDVAR_NOTIFY: {
      resourceIter->second.condvar->onNotify(); //TODO here ->, above .
      break;
    }
    default: {
      std::cout << "    !UNSUPPORTED PACKET TYPE" << std::endl;
    }
  }
}

void ProcessMonitor::receiveResource(int resourceId) {
  // std::cout << commRank << ": receiveResource" << std::endl;
  MPI_Status status;

  auto resourceIter = idToRes.find(resourceId);
  if (resourceIter == idToRes.end()) {
    // std::cout << "ret" << std::endl;
    return;
  }
  // std::cout << "a" << std::endl;
  MPI_Recv(resourceIter->second.resource, resourceIter->second.size, MPI_BYTE,
     MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    //  std::cout << "b" << std::endl;
}
