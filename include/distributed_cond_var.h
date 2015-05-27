#ifndef DM_INCLUDE_DISTRIBUTED_COND_VAR_H_
#define DM_INCLUDE_DISTRIBUTED_COND_VAR_H_

#include "distributed_mutex.h"

class DistributedCondvar {
public:
  DistributedCondvar(int condvarId);
  ~DistributedCondvar();

  template< class Predicate >
  void wait(DistributedMutex& mutex, Predicate pred);
  void notifyOne();
  int getCondvarId();
  void onNotify();
private:
  int condvarId;
  std::condition_variable condVarLocal;
  std::mutex mutexLocal;
};

#endif //DM_INCLUDE_DISTRIBUTED_COND_VAR_H_
