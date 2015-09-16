#ifndef DM_INCLUDE_DISTRIBUTED_CONDVAR_H_
#define DM_INCLUDE_DISTRIBUTED_CONDVAR_H_

#include <deque>
#include <iostream>

#include "distributed_mutex.h"
#include "process_monitor.h"


class DistributedCondvar {
  // friend class ProcessMonitor;
public:
  DistributedCondvar(DistributedMutex* mutex, unsigned int id);
  ~DistributedCondvar();

  template<typename Predicate>
  void wait(Predicate pred) {
    static std::mutex mutexLocal;

    // std::cout << rank() << ": wait" << std::endl;
    if (pred())
      return;

    Packet packet = Packet(distributedMutex->getLocalClock(), Packet::Type::DM_CONDVAR_WAIT, id);
    ProcessMonitor::instance().broadcastPacket(packet);
    // std::cout << "waitersQueue size = " << waitersQueue.size() << std::endl;
    waitersQueue.push_back(rank());

    while (!pred()) {
      //TODO right here the process is in a critical section, so broadcast messages will be total ordered?,
      //the same as in resource?
      // std::cout << rank() << ": waitersQueue size = " << waitersQueue.size() << std::endl;
      //TODO collect replies
      distributedMutex->release();
      std::unique_lock<std::mutex> lock(mutexLocal); //TODO switch to mutex instead of condition variable?
      condVarLocal.wait(lock); //TODO spurious wakeup?
      // std::cout << rank() << ": tutaj2" << std::endl;
      distributedMutex->acquire();
    }
    // std::cout << rank() << ": out of condvar wait" << std::endl;
  }
  void notify();
  void onNotify(int fromRank);
  void onWait(int fromRank);

  int rank();
private:
  unsigned int id;
  DistributedMutex* distributedMutex = NULL;
  std::condition_variable condVarLocal;
  std::mutex guard;
  std::deque<int> waitersQueue;
};

#endif //DM_INCLUDE_DISTRIBUTED_CONDVAR_H_
