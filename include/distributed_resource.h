#ifndef DM_INCLUDE_DISTRIBUTED_RESOURCE_H_
#define DM_INCLUDE_DISTRIBUTED_RESOURCE_H_

#include "distributed_mutex.h"
#include "distributed_condvar.h"

#include <string>

template<typename R>
class DistributedResource {
public:
  DistributedResource(unsigned int id, R& resource);
  ~DistributedResource();

  void lock();
  void unlock();

  template<typename Predicate>
  void wait(Predicate pred) {
    if (condvar == NULL)
      condvar = new DistributedCondvar();

    condvar.wait(mutex, pred);
  }

  void notify();

  unsigned int getId();
private:
  unsigned int id;
  R& resource;
  DistributedMutex* mutex = NULL;
  DistributedCondvar* condvar = NULL;
};

#endif //DM_INCLUDE_DISTRIBUTED_RESOURCE_H_
