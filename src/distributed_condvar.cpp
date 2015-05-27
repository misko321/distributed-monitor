#include <iostream>

#include "distributed_condvar.h"
#include "process_monitor.h"

DistributedCondvar::DistributedCondvar(int condvarId) : condvarId(condvarId) {
  ProcessMonitor::instance().addCondvar(*this);
}

DistributedCondvar::~DistributedCondvar() {
  ProcessMonitor::instance().removeCondvar(*this);
}

/* This method assumes, that [mutex] has already been acquired. */
// template<typename Predicate>
// void DistributedCondvar::wait(DistributedMutex& mutex, Predicate pred) {
//   static std::mutex mutexLocal;
//
//   this->distributedMutex = &mutex;
//   while (!pred()) {
//     //right here the process is in a critical section, so broadcast messages will be total ordered
//     Packet packet = Packet(distributedMutex->getLocalClock(), Packet::Type::DM_CONDVAR_WAIT, condvarId);
//     ProcessMonitor::instance().broadcast(packet);
//
//     distributedMutex->release();
//     std::unique_lock<std::mutex> lock(mutexLocal); //TODO switch to mutex instead of condition variable?
//     condVarLocal.wait(lock); //TODO spurious wakeup?
//
//     distributedMutex->acquire();
//   }
// }

void DistributedCondvar::notifyOne() {
  std::lock_guard<std::mutex> lock(guard); //TODO check if guards needed here and in other lines

  waitersQueue.pop_front();
  Packet packet = Packet(distributedMutex->getLocalClock(), Packet::Type::DM_CONDVAR_NOTIFY, condvarId);
  //all processes must be informed, so they can update thier waitersQueue
  ProcessMonitor::instance().broadcast(packet);

  this->distributedMutex = NULL;
}

void DistributedCondvar::onNotify() {
  int firstWaiter = waitersQueue.front();
  waitersQueue.pop_front();

  //if this process is the first on the list, wake it up
  if (firstWaiter == ProcessMonitor::instance().getCommRank())
    condVarLocal.notify_one();
}

void DistributedCondvar::onWait(int fromRank) {
  std::lock_guard<std::mutex> lock(guard);

  std::cout << rank() << ": onWait from: " << fromRank << std::endl;
  waitersQueue.push_back(fromRank);
}

int DistributedCondvar::rank() {
  return ProcessMonitor::instance().getCommRank();
}

int DistributedCondvar::getCondvarId() {
  return condvarId;
}
