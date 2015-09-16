#include <iostream>
#include <algorithm>

#include "distributed_condvar.h"

DistributedCondvar::DistributedCondvar(DistributedMutex* mutex, unsigned int id) : id(id) {
  this->distributedMutex = mutex;
}

DistributedCondvar::~DistributedCondvar() {
}

void DistributedCondvar::notify() {
  std::lock_guard<std::mutex> lock(guard);

  if (waitersQueue.size() > 0 && waitersQueue.front() == ProcessMonitor::instance().getCommRank())
    waitersQueue.pop_front(); //remove yourself from queue if leaving condvar
  Packet packet = Packet(distributedMutex->getLocalClock(), Packet::Type::DM_CONDVAR_NOTIFY, id);
  ProcessMonitor::instance().broadcastPacket(packet);
}

void DistributedCondvar::onNotify(int fromRank) {
  std::lock_guard<std::mutex> lock(guard);

  if (waitersQueue.size() > 0 && waitersQueue.front() == fromRank)
    waitersQueue.pop_front();
  if (waitersQueue.size() > 0 && waitersQueue.front() == ProcessMonitor::instance().getCommRank())
    condVarLocal.notify_one();
}

void DistributedCondvar::onWait(int fromRank) {
  std::lock_guard<std::mutex> lock(guard);

  waitersQueue.push_back(fromRank);
}

int DistributedCondvar::rank() {
  return ProcessMonitor::instance().getCommRank();
}

void DistributedCondvar::onRecvConfirm() {
  --repliesNeeded;
  if (repliesNeeded == 0)
    waitForConfirm.notify_one();
}
