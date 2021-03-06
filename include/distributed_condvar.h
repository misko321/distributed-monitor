#ifndef DM_INCLUDE_DISTRIBUTED_CONDVAR_H_
#define DM_INCLUDE_DISTRIBUTED_CONDVAR_H_

#include <deque>
#include <iostream>

#include "distributed_mutex.h"
#include "process_monitor.h"


class DistributedCondvar {
public:
  DistributedCondvar(DistributedMutex* mutex, unsigned int id);
  ~DistributedCondvar();

  template<typename Predicate>
  void wait(Predicate pred) {
    static std::mutex mutexLocal;

    if (pred())
      return;

    Packet packet = Packet(distributedMutex->getLocalClock(), Packet::Type::DM_CONDVAR_WAIT, id);
    ProcessMonitor::instance().broadcastPacket(packet);
    waitersQueue.push_back(rank());

    std::mutex mutex;
    std::unique_lock<std::mutex> lock(mutex);
    this->repliesNeeded = ProcessMonitor::instance().getCommSize() - 1;
    waitForConfirm.wait(lock, [this]()-> bool {
      return this->repliesNeeded == 0;
    });

    while (!pred()) {
      distributedMutex->release();
      std::unique_lock<std::mutex> lock(mutexLocal); //TODO switch to mutex instead of condition variable?
      condVarLocal.wait(lock); //TODO spurious wakeup?
      distributedMutex->acquire();
    }
  }
  void notify();
  void onNotify(int fromRank);
  void onWait(int fromRank);
  void onRecvConfirm();

  int rank();
private:
  unsigned int id;
  int repliesNeeded;
  DistributedMutex* distributedMutex = NULL;
  std::condition_variable condVarLocal;
  std::condition_variable waitForConfirm;
  std::mutex guard;
  std::deque<int> waitersQueue;
};

#endif //DM_INCLUDE_DISTRIBUTED_CONDVAR_H_
