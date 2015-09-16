#include "distributed_mutex.h"

#include "process_monitor.h"
#include "packet.h"

#include <iostream>
#include <mutex>

#define MAX(a, b) a > b ? a : b

DistributedMutex::DistributedMutex(unsigned int id) : id(id) {
  int size = ProcessMonitor::instance().getCommSize();
  waitsForReply = new int[size];
  for (int i = 0; i < size; ++i)
    waitsForReply[i] = false;
}

DistributedMutex::~DistributedMutex() {
}

void DistributedMutex::onReply(int sourceCommRank) {
  --repliesNeeded;
  if (repliesNeeded == 0)
    waitForReplies.notify_one();
}

void DistributedMutex::onRequest(int sourceCommRank, long packetClock) {
  std::lock_guard<std::mutex> lock(guard);

  highestClock = MAX(highestClock, packetClock);
  if (interestedInCriticalSection &&
    ((packetClock > localClock) ||
    (packetClock == localClock && sourceCommRank > ProcessMonitor::instance().getCommRank()))) {
      waitsForReply[sourceCommRank] = true;
  }
  else {
    Packet packet = Packet(localClock, Packet::Type::DM_REPLY, id);
    ProcessMonitor::instance().sendPacket(sourceCommRank, packet);
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
	waitForReplies.wait(lock, [this]()-> bool {
		return this->repliesNeeded == 0;
	});
}

void DistributedMutex::release() {
  std::lock_guard<std::mutex> lock(guard);
  interestedInCriticalSection = false;
  int size = ProcessMonitor::instance().getCommSize();
  for (int i = 0; i < size; ++i) {
    if (waitsForReply[i]) {
      Packet packet = Packet(localClock, Packet::Type::DM_REPLY, id);
      ProcessMonitor::instance().sendPacket(i, packet);
      waitsForReply[i] = false;
    }
  }
}
