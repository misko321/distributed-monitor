#include <iostream>
#include <algorithm>

#include "distributed_condvar.h"

DistributedCondvar::DistributedCondvar(DistributedMutex* mutex, unsigned int id) : id(id) {
  this->distributedMutex = mutex;
}

DistributedCondvar::~DistributedCondvar() {
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

void DistributedCondvar::notify() {
  std::lock_guard<std::mutex> lock(guard); //TODO check if guards needed here and in other lines

  // std::cout << rank() << ": sending notify" << std::endl;
  //TODO zakładam, że pierwszy z kolejki dostanie dostep, ale on go dostanie dopiero moze po kilku tych notifyiach
  //az do spelnienia warunku
  if (waitersQueue.size() > 0 && waitersQueue.front() == ProcessMonitor::instance().getCommRank())
    waitersQueue.pop_front(); //remove yourself from queue if leaving condvar
  // std::cout << rank() << ": after sending notify2" << std::endl;
  Packet packet = Packet(distributedMutex->getLocalClock(), Packet::Type::DM_CONDVAR_NOTIFY, id);
  //all processes must be informed, so they can update thier waitersQueue
  // std::cout << rank() << ": after sending notify3" << std::endl;
  ProcessMonitor::instance().broadcastPacket(packet);
  // std::cout << rank() << ": after sending notify" << std::endl;
  // this->distributedMutex = NULL;
}

void DistributedCondvar::onNotify(int fromRank) {
  std::lock_guard<std::mutex> lock(guard);

  // std::cout << rank() << ": onNotify" << std::endl;
  // int firstWaiter = waitersQueue.front();
  // std::cout << rank() << ": onNotify, front = " << firstWaiter << ", size = " << waitersQueue.size() << ", fromRank = " << fromRank << std::endl;
  // std::cout << rank() << ": onNotify2" << std::endl;
  //if this process is the first on the list, wake it up
    // if (firstWaiter == nie ja, tylko ja lub from)
  if (waitersQueue.size() > 0 && waitersQueue.front() == fromRank)
    waitersQueue.pop_front();
  if (waitersQueue.size() > 0 && waitersQueue.front() == ProcessMonitor::instance().getCommRank())
    condVarLocal.notify_one();
  // std::cout << rank() << ": 2onNotify, front = " << firstWaiter << ", size = " << waitersQueue.size() << ", fromRank = " << fromRank << std::endl;
}

void DistributedCondvar::onWait(int fromRank) {
  std::lock_guard<std::mutex> lock(guard);

  // std::cout << rank() << ": onWait from: " << fromRank << std::endl;
  waitersQueue.push_back(fromRank);
}

int DistributedCondvar::rank() {
  return ProcessMonitor::instance().getCommRank();
}

void DistributedCondvar::onRecvConfirm() {
// std::cout << ProcessMonitor::instance().getCommRank() << ": onRecvConfirm" << std::endl;
  --repliesNeeded;
  if (repliesNeeded == 0)
    waitForConfirm.notify_one();
}
