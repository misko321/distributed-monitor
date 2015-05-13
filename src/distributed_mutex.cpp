#include "distributed_mutex.h"

#include "process_monitor.h"
#include "packet.h"

#include <iostream>
#include <mutex>

#define MAX(a, b) a > b ? a : b


DistributedMutex::DistributedMutex(int resourceId) : resourceId(resourceId) {
  // std::cout << "Creating new mutex" << std::endl;
  int size = ProcessMonitor::instance().getCommSize();
  waitsForReply = new int[ProcessMonitor::instance().getCommSize()];
  for (int i = 0; i < size; ++i)
    waitsForReply[i] = false;

  ProcessMonitor::instance().addMutex(*this);
}

DistributedMutex::~DistributedMutex() {
  // std::cout << "Destroying mutex" << std::endl;
  ProcessMonitor::instance().removeMutex(*this);
  //ProcessMonitor::instance().finish();
}

// std::condition_variable& DistributedMutex::getWaitCondition() {
//   return waitCondition;
// }

//TODO: parameterless?
void DistributedMutex::onReply(int sourceCommRank, Packet& packet) {
  --repliesNeeded;
  if (repliesNeeded == 0)
    waitCondition.notify_one();
}

void DistributedMutex::onRequest(int sourceCommRank, Packet &packet) {
  highestClock = MAX(highestClock, packet.getClock());
  if (interestedInCriticalSection &&
    ((packet.getClock() > localClock) ||
    (packet.getClock() == localClock && sourceCommRank > ProcessMonitor::instance().getCommRank()))) {
      waitsForReply[sourceCommRank] = true;
  }
  else {
    Packet packet = Packet(localClock, Packet::Type::DM_REPLY, resourceId);
    ProcessMonitor::instance().send(sourceCommRank, packet);
  }

}

void DistributedMutex::acquire() {
  //mutex waits on thread_condition, process monitor notifies (notify/notify_all?) mutex
  //corresponding to resource that is, this mutex
  interestedInCriticalSection = true;
  localClock = highestClock + 1;
  Packet packet = Packet(localClock, Packet::Type::DM_REQUEST, resourceId);
  repliesNeeded = ProcessMonitor::instance().getCommSize() - 1;

  ProcessMonitor::instance().broadcast(packet);

  std::mutex mutex;
  std::unique_lock<std::mutex> lock(mutex);
	waitCondition.wait(lock, [this]()-> bool {
		return this->repliesNeeded == 0;
	});
}

void DistributedMutex::release() {
  // std::cout << "release" << std::endl;
  interestedInCriticalSection = false;
  int size = ProcessMonitor::instance().getCommSize();
  Packet packet = Packet(localClock, Packet::Type::DM_REPLY, resourceId);
  for (int i = 0; i < size; ++i) {
    if (waitsForReply[i]) {
      ProcessMonitor::instance().send(i, packet);
      waitsForReply[i] = false;
    }
  }
}

unsigned int DistributedMutex::getResourceId() {
  return resourceId;
}
