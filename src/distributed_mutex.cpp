#include "distributed_mutex.h"

#include "process_monitor.h"
#include "packet.h"

#include <iostream>


DistributedMutex::DistributedMutex() {
  std::cout << "Creating new mutex" << std::endl;
}

DistributedMutex::~DistributedMutex() {
  std::cout << "Destroying mutex" << std::endl;
}

void DistributedMutex::acquire() {
  std::cout << "acquire" << std::endl;
  interestedInCriticalSection = true;
  localClock = highestClock + 1;
  Packet *packet = new Packet(localClock, Packet::Type::DM_REQUEST);
  // repliesNeeded = commSize - 1;

  ProcessMonitor::instance().broadcast(*packet);
  //wysylamy zadania (REQUEST) do wszystkich procesow (oprocz siebie)

  delete packet;
}

void DistributedMutex::release() {
  std::cout << "release" << std::endl;
}
