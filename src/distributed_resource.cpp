#include "distributed_resource.h"
#include "process_monitor.h"

template<typename R> DistributedResource<R>::DistributedResource(unsigned int id, R& resource) :
                                                                id(id),
                                                                resource(resource) {
}

template<typename R> unsigned int DistributedResource<R>::getId() {
  return id;
}

template<typename R> void lock() {
  mutex.acquire();
}
template<typename R> void unlock() {
  mutex.release();
}

template<typename R> void notify() {
  condvar.notify();
}
