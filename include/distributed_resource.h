#ifndef DM_INCLUDE_DISTRIBUTED_RESOURCE_H_
#define DM_INCLUDE_DISTRIBUTED_RESOURCE_H_

#include "distributed_mutex.h"
#include "distributed_condvar.h"

#include <string>

class DistributedResource {
  friend class ProcessMonitor;

public:
  DistributedResource(unsigned int id, void* resource, int size);
  ~DistributedResource();

  void sync();

  void lock();
  void unlock();

  template<typename Predicate>
  void wait(Predicate pred) {
    if (condvar == NULL)
      condvar = new DistributedCondvar(id);

    condvar->wait(mutex, pred);
  }

  void notify();

  unsigned int getId();
private:
  unsigned int id;
  DistributedMutex* mutex = NULL;
  DistributedCondvar* condvar = NULL;
  void* resource;
  int size;
};

#endif //DM_INCLUDE_DISTRIBUTED_RESOURCE_H_
