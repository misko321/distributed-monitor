#include "process_monitor.h"

#include "packet.h"
#include "distributed_resource.h"

#include <mutex>
#include <thread>
#include <mpi.h>

#define PACKET_TAG 0
#define CLOCK_NO_MATTER 0

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

//Singleton design pattern
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

  if (idToRes.find(resource.getId()) == idToRes.end()) {
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
  std::lock_guard<std::mutex> lock(guard);
  for (int i = 0; i < commSize; ++i) {
      if (i != commRank) {
        MPI_Send(&packet, sizeof(packet), MPI_BYTE, i, PACKET_TAG, MPI_COMM_WORLD);
      }
  }
}

void ProcessMonitor::sendPacket(int destination, Packet &packet) {
  std::lock_guard<std::mutex> lock(guard);
  MPI_Send(&packet, sizeof(packet), MPI_BYTE, destination, PACKET_TAG, MPI_COMM_WORLD);
}

void ProcessMonitor::broadcastResource(int id, void* resource, int size) {
  std::lock_guard<std::mutex> lock(guard);
  for (int i = 0; i < commSize; ++i) {
    if (i != commRank)
      MPI_Send(resource, size, MPI_BYTE, i, id, MPI_COMM_WORLD);
  }
}

void ProcessMonitor::run() {
  monitorThread = std::thread([this]() -> void {

    while(!this->shouldFinish) {
      MPI_Status status;
      MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        if (status.MPI_TAG == PACKET_TAG)
          this->pInstance->receivePacket();
        else {
          int count;
          MPI_Get_count(&status, MPI_BYTE, &count);
          this->pInstance->receiveResource(status.MPI_TAG);
        }
    }
  });
}

void ProcessMonitor::receivePacket() {
  MPI_Status status;
  Packet packet;
  MPI_Recv(&packet, sizeof(packet), MPI_BYTE, MPI_ANY_SOURCE, PACKET_TAG, MPI_COMM_WORLD, &status);

  auto resourceIter = idToRes.find(packet.getResourceId());
  if(resourceIter == idToRes.end()) {
    Packet packet2 = Packet(packet.getClock(), Packet::Type::DM_REPLY, packet.getResourceId());
    sendPacket(status.MPI_SOURCE, packet2);
  }

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
      Packet newPacket = Packet(CLOCK_NO_MATTER, Packet::Type::DM_CONDVAR_RECV_CONFIRM, packet.getResourceId());
      sendPacket(status.MPI_SOURCE, newPacket);
      break;
    }
    case Packet::Type::DM_CONDVAR_NOTIFY: {
      resourceIter->second.condvar->onNotify(status.MPI_SOURCE);
      break;
    }
    case Packet::Type::DM_RECV_CONFIRM: {
      resourceIter->second.onRecvConfirm();
      break;
    }
    case Packet::Type::DM_CONDVAR_RECV_CONFIRM: {
      resourceIter->second.condvar->onRecvConfirm();
      break;
    }
    default: {
      std::cout << "    !UNSUPPORTED PACKET TYPE = " << packet.getType() << std::endl;
    }
  }
}

void ProcessMonitor::receiveResource(int resourceId) {
  MPI_Status status;

  auto resourceIter = idToRes.find(resourceId);
  if (resourceIter == idToRes.end())
    return;
  MPI_Recv(resourceIter->second.resource, resourceIter->second.size,
    MPI_BYTE, MPI_ANY_SOURCE, resourceId, MPI_COMM_WORLD, &status);

  Packet packet = Packet(CLOCK_NO_MATTER, Packet::Type::DM_RECV_CONFIRM, resourceId);
  sendPacket(status.MPI_SOURCE, packet);
}
