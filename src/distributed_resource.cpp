#include "distributed_resource.h"

#include "process_monitor.h"

#include <iostream>
#include <mutex>

DistributedResource::DistributedResource(unsigned int id, void* resource, size_t size) :
                                                id(id), resource(resource) {
  this->size = static_cast<int>(size);
  this->mutex = new DistributedMutex(id);
  this->condvar = new DistributedCondvar(this->mutex, id);
  ProcessMonitor::instance().addResource(*this);
  std::cout << ProcessMonitor::instance().getCommRank() << ": Resource with id = "
    << this->id << " created successfully" << std::endl;
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
  sync();
  mutex->release();
}

void DistributedResource::notify() {
  condvar->notify();
}

void DistributedResource::sync() {
  ProcessMonitor::instance().broadcastResource(id, resource, size);

  repliesNeeded = ProcessMonitor::instance().getCommSize() - 1;
  std::mutex mutex;
  std::unique_lock<std::mutex> lock(mutex);
  waitForConfirm.wait(lock, [this]()-> bool {
		return this->repliesNeeded == 0;
	});
}

void DistributedResource::onRecvConfirm() {
  --repliesNeeded;
  if (repliesNeeded == 0)
    waitForConfirm.notify_one();
}
