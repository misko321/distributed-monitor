#include "distributed_mutex.h"

#include "process_monitor.h"
#include "packet.h"

#include <iostream>


DistributedMutex::DistributedMutex(int resourceId) : resourceId(resourceId) {
  // std::cout << "Creating new mutex" << std::endl;
  ProcessMonitor::instance().addMutex(*this);
}

DistributedMutex::~DistributedMutex() {
  // std::cout << "Destroying mutex" << std::endl;
  ProcessMonitor::instance().removeMutex(*this);
  //ProcessMonitor::instance().finish();
}

void DistributedMutex::acquire() {
  // std::cout << "acquire" << std::endl;
  //mutex waits on thread_condition, process monitor notifies (notify/notify_all?) mutex
  //corresponding to resource that is, this mutex
  interestedInCriticalSection = true;
  localClock = highestClock + 1;
  Packet packet = Packet(localClock, Packet::Type::DM_REQUEST);
  // repliesNeeded = commSize - 1;

  ProcessMonitor::instance().broadcast(packet);
  //wysylamy zadania (REQUEST) do wszystkich procesow (oprocz siebie)
}

void DistributedMutex::release() {
  // std::cout << "release" << std::endl;
}

unsigned int DistributedMutex::getResourceId() {
  return resourceId;
}
