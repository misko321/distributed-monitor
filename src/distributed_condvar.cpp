#include "distributed_condvar.h"
#include "process_monitor.h"

DistributedCondvar::DistributedCondvar(int condvarId) : condvarId(condvarId) {
}

DistributedCondvar::~DistributedCondvar() {
}

/* This method assumes, that [mutex] has already been acquired. */
template<class Predicate>
void DistributedCondvar::wait(DistributedMutex& mutex, Predicate pred) {
  static std::mutex mutexLocal;

  this->distributedMutex = &mutex;
  while (!pred()) {
    //right here the process is in a critical section, so broadcast messages will be total ordered
    Packet packet = Packet(distributedMutex->getLocalClock(), Packet::Type::DM_CONDVAR_WAIT, condvarId);
    ProcessMonitor::instance().broadcast(packet);

    distributedMutex->release();
    std::unique_lock<std::mutex> lock(mutexLocal); //TODO switch to mutex instead of condition variable?
    condVarLocal.wait(lock); //TODO spurious wakeup?

    distributedMutex->acquire();
  }
}

void DistributedCondvar::notifyOne() {
  std::lock_guard<std::mutex> lock(guard); //TODO check if guards needed here and in other lines

  int rankToNotify = waitersQueue.front();
  waitersQueue.pop_front();
  Packet packet = Packet(distributedMutex->getLocalClock(), Packet::Type::DM_CONDVAR_NOTIFY, condvarId);
  ProcessMonitor::instance().send(rankToNotify, packet);

  this->distributedMutex = NULL;
}

void DistributedCondvar::onNotify() {
  condVarLocal.notify_one();
}

void DistributedCondvar::onWait(int fromRank) {
  std::lock_guard<std::mutex> lock(guard);

  waitersQueue.push_back(fromRank);
}

int DistributedCondvar::getCondvarId() {
  return condvarId;
}
