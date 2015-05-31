#include "distributed_resource.h"

#include <iostream>
#include "process_monitor.h"

//TODO fair parameter
DistributedResource::DistributedResource(unsigned int id, void* resource, size_t size) :
                                                id(id), resource(resource) {
  this->size = static_cast<int>(size);
  this->mutex = new DistributedMutex(id);
  ProcessMonitor::instance().addResource(*this);
  std::cout << "Resource with id = " << this->id << " created successfully" << std::endl;
}

DistributedResource::~DistributedResource() {
}

unsigned int DistributedResource::getId() {
  return id;
}

void DistributedResource::lock() {
  mutex->acquire();
}

void DistributedResource::unlock() {
  sync(); //TODO what if due to unfulfilled condition, resource remains unchanged?
  mutex->release();
}

void DistributedResource::notify() {
  condvar->notify();
}

//TODO obtain responses from everybody
void DistributedResource::sync() {
  ProcessMonitor::instance().broadcastResource(id, resource, size);
}
