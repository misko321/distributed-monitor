#ifndef DM_INCLUDE_DISTRIBUTED_CONDVAR_H_
#define DM_INCLUDE_DISTRIBUTED_CONDVAR_H_

#include <deque>

#include "distributed_mutex.h"
#include "process_monitor.h"


class DistributedCondvar {
  // friend class ProcessMonitor;
public:
  DistributedCondvar(unsigned int id);
  ~DistributedCondvar();

  template<typename Predicate>
  void wait(DistributedMutex& mutex, Predicate pred) {
    static std::mutex mutexLocal;

    this->distributedMutex = &mutex;
    while (!pred()) {
      //right here the process is in a critical section, so broadcast messages will be total ordered
      Packet packet = Packet(distributedMutex->getLocalClock(), Packet::Type::DM_CONDVAR_WAIT, id);
      ProcessMonitor::instance().broadcast(packet);

      distributedMutex->release();
      std::unique_lock<std::mutex> lock(mutexLocal); //TODO switch to mutex instead of condition variable?
      condVarLocal.wait(lock); //TODO spurious wakeup?

      distributedMutex->acquire();
    }
  }
  void notify();
  void onNotify();
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
