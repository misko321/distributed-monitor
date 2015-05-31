#ifndef DM_INCLUDE_DISTRIBUTED_RESOURCE_H_
#define DM_INCLUDE_DISTRIBUTED_RESOURCE_H_

#include "distributed_mutex.h"
#include "distributed_condvar.h"

#include <string>
#include <condition_variable>

class DistributedResource {
  friend class ProcessMonitor;

public:
  DistributedResource(unsigned int id, void* resource, size_t size);
  ~DistributedResource();

  void lock();
  void unlock();

  template<typename Predicate>
  void wait(Predicate pred) {
    if (condvar == NULL)
      condvar = new DistributedCondvar(id);

    condvar->wait(mutex, pred);
  }

  void notify();
  void onRecvConfirm();

  unsigned int getId();
private:
  unsigned int id;
  DistributedMutex* mutex = NULL;
  DistributedCondvar* condvar = NULL;
  void* resource;
  int size;
  std::condition_variable waitForConfirm;
  int repliesNeeded;

  void sync();
};

#endif //DM_INCLUDE_DISTRIBUTED_RESOURCE_H_
