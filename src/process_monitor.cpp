#include "process_monitor.h"

#include "packet.h"
#include "distributed_condvar.h"

#include <mutex>
#include <thread>
#include <mpi.h>

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

void ProcessMonitor::run() {
  monitorThread = std::thread([this]() -> void {
    int flag;
    MPI_Status status;

    while(!this->shouldFinish) {
      MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
      if (flag)
        this->pInstance->receive();
    }
  });
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

  /* If there are no more mutexes to watch, destroy ProcessMonitor
  WARNING: this feature allows for destroying the process monitor and deallocate used resources,
  but since the monitor will not exist anymore, other processes may block on waiting for messages
  from it. Uncomment if you know what you're doin'. */
  //if (resToMutex.size() == 0)
  //  finish();
}

void ProcessMonitor::addCondvar(DistributedCondvar& condvar) {
  std::lock_guard<std::mutex> lock(guard);

  if (resToCondvar.find(condvar.getCondvarId()) == resToCondvar.end()) {
    std::pair<unsigned int, DistributedCondvar&> pair(condvar.getCondvarId(), condvar);
    resToCondvar.insert(pair);
  }
}

void ProcessMonitor::removeCondvar(DistributedCondvar& condvar) {
  std::lock_guard<std::mutex> lock(guard);

  resToMutex.erase(condvar.getCondvarId());
}

void ProcessMonitor::finish() {
  std::cout << commRank << ": finish()" << std::endl;
  this->shouldFinish = true;
  monitorThread.join();
  delete pInstance;
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

//TODO improve structure, method for each packet type
void ProcessMonitor::receive() {
  MPI_Status status;
  Packet packet;
  MPI_Recv(&packet, sizeof(packet), MPI_BYTE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

  switch (packet.getType()) {
    case Packet::Type::DM_REQUEST:
    case Packet::Type::DM_REPLY: {
      receiveMutexPacket(packet, status.MPI_SOURCE);
      break;
    }
    case Packet::Type::DM_CONDVAR_WAIT:
    case Packet::Type::DM_CONDVAR_NOTIFY: {
      receiveCondvarPacket(packet, status.MPI_SOURCE);
      break;
    }
    default: {
      std::cout << "    !UNSUPPORTED PACKET TYPE" << std::endl;
    }
  }
}

void ProcessMonitor::receiveMutexPacket(Packet& packet, int fromRank) {
  auto mutexIter = resToMutex.find(packet.getResourceId());
  //if this monitor doesn't care about specific resourceId, immidiately send DM_REPLY
  if (mutexIter == resToMutex.end()) {
    Packet packet = Packet(-1, Packet::Type::DM_REPLY, packet.getResourceId());
    send(fromRank, packet);
  }
  else {
    switch(packet.getType()) {
      case Packet::Type::DM_REPLY: {
        mutexIter->second.onReply(fromRank);
        break;
      }
      case Packet::Type::DM_REQUEST: {
        mutexIter->second.onRequest(fromRank, packet.getClock());
        break;
      }
      default: break;
    }
  }
}

void ProcessMonitor::receiveCondvarPacket(Packet& packet, int fromRank) {
  auto condvarIter = resToCondvar.find(packet.getResourceId());
  if (condvarIter == resToCondvar.end())
    return;

  switch(packet.getType()) {
    case Packet::Type::DM_CONDVAR_WAIT: {
      condvarIter->second.onWait(fromRank);
      break;
    }
    case Packet::Type::DM_CONDVAR_NOTIFY: {
      condvarIter->second.onNotify();
      break;
    }
    default: break;
  }
}
