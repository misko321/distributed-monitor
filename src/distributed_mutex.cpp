#include "distributed_mutex.h"

#include "process_monitor.h"
#include "packet.h"

#include <iostream>
#include <mutex>

#define MAX(a, b) a > b ? a : b

DistributedMutex::DistributedMutex(unsigned int id) : id(id) {
  int size = ProcessMonitor::instance().getCommSize();
  waitsForReply = new int[ProcessMonitor::instance().getCommSize()];
  for (int i = 0; i < size; ++i)
    waitsForReply[i] = false;
}

DistributedMutex::~DistributedMutex() {
}

void DistributedMutex::onReply(int sourceCommRank) {
  // std::cout << rank() << ": onReply from: " << sourceCommRank << std::endl;
  --repliesNeeded;
  if (repliesNeeded == 0)
    waitCondition.notify_one();
}

void DistributedMutex::onRequest(int sourceCommRank, long packetClock) {
  // std::cout << rank() << ": onRequest from: " << sourceCommRank << std::endl;
  highestClock = MAX(highestClock, packetClock);
  if (interestedInCriticalSection &&
    ((packetClock > localClock) ||
    (packetClock == localClock && sourceCommRank > ProcessMonitor::instance().getCommRank()))) {
      waitsForReply[sourceCommRank] = true;
      // std::cout << rank() << ": deferring reply to: " << sourceCommRank << std::endl;
  }
  else {
    Packet packet = Packet(localClock, Packet::Type::DM_REPLY, id);
    ProcessMonitor::instance().sendPacket(sourceCommRank, packet);
    // std::cout << rank() << ": sending immediate reply to: " << sourceCommRank << std::endl;
  }

}

int DistributedMutex::rank() {
  return ProcessMonitor::instance().getCommRank();
}

long DistributedMutex::getLocalClock() {
  return localClock;
}

void DistributedMutex::acquire() {
  //mutex waits on thread_condition, process monitor notifies (notify/notify_all?) mutex
  //corresponding to resource that is, this mutex
  interestedInCriticalSection = true;
  localClock = highestClock + 1;
  Packet packet = Packet(localClock, Packet::Type::DM_REQUEST, id);
  repliesNeeded = ProcessMonitor::instance().getCommSize() - 1;

  ProcessMonitor::instance().broadcastPacket(packet);

  std::mutex mutex;
  std::unique_lock<std::mutex> lock(mutex);
	waitCondition.wait(lock, [this]()-> bool {
		return this->repliesNeeded == 0;
	});
}

//TODO mutex, something may arrive while sending replies here
void DistributedMutex::release() {
  // std::cout << "release\n";
  interestedInCriticalSection = false;
  int size = ProcessMonitor::instance().getCommSize();
  Packet packet = Packet(localClock, Packet::Type::DM_REPLY, id);
  for (int i = 0; i < size; ++i) {
    if (waitsForReply[i]) {
      ProcessMonitor::instance().sendPacket(i, packet);
      waitsForReply[i] = false;
      // std::cout << rank() << ": sending reply after deferring to: " << i << std::endl;
    }
  }
}
