#include "distributed_resource.h"
#include "process_monitor.h"

DistributedResource::DistributedResource(unsigned int id, void* resource, int size) :
                                                id(id), resource(resource), size(size) {
  mutex = new DistributedMutex(id);
}

unsigned int DistributedResource::getId() {
  return id;
}

void DistributedResource::lock() {
  mutex->acquire();
}

void DistributedResource::unlock() {
  mutex->release();
}

void DistributedResource::notify() {
  condvar->notify();
}

void DistributedResource::sync() {
  ProcessMonitor::instance().sendResource(resource, size);
}
