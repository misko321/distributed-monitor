#ifndef DM_INCLUDE_DISTRIBUTED_COND_VAR_H_
#define DM_INCLUDE_DISTRIBUTED_COND_VAR_H_

#include <deque>

#include "distributed_mutex.h"
#include "process_monitor.h"

class DistributedCondvar {
public:
  DistributedCondvar(int condvarId);
  ~DistributedCondvar();

  template<typename Predicate>
  void wait(DistributedMutex& mutex, Predicate pred) {
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
  void notifyOne();
  void onNotify();
  void onWait(int fromRank);

  int rank();
  int getCondvarId();
private:
  DistributedMutex* distributedMutex = NULL;
  int condvarId;
  std::condition_variable condVarLocal;
  std::mutex guard;
  std::deque<int> waitersQueue;
};

#endif //DM_INCLUDE_DISTRIBUTED_COND_VAR_H_
