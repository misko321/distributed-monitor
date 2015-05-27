#ifndef DM_INCLUDE_DISTRIBUTED_COND_VAR_H_
#define DM_INCLUDE_DISTRIBUTED_COND_VAR_H_

#include <deque>

#include "distributed_mutex.h"

class DistributedCondvar {
public:
  DistributedCondvar(int condvarId);
  ~DistributedCondvar();

  template<class Predicate>
  void wait(DistributedMutex& mutex, Predicate pred);
  void notifyOne();
  void onNotify();
  void onWait(int fromRank);

  int getCondvarId();
private:
  DistributedMutex* distributedMutex = NULL;
  int condvarId;
  std::condition_variable condVarLocal;
  std::mutex guard;
  std::deque<int> waitersQueue;
};

#endif //DM_INCLUDE_DISTRIBUTED_COND_VAR_H_
