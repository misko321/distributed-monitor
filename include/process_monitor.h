#ifndef DM_INCLUDE_PROCESS_MONITOR_H_
#define DM_INCLUDE_PROCESS_MONITOR_H_

// #include "distributed_resource.h"
// #include "distributed_condvar.h"

#include <thread>
#include <mutex>
#include <unordered_map>

class Packet;
template<typename R> class DistributedResource;

template<template<typename> typename DistributedResource>
class ProcessMonitor {
public:
  ProcessMonitor();
  ~ProcessMonitor();

  static ProcessMonitor& instance();
  // template<typename R>
  void addResource(DistributedResource<R>& resource) {

  }
  // template<typename R>
  void removeResource(DistributedResource<R>& resource) {

  }

  void send(int destination, Packet &packet);
  void broadcast(Packet &packet);

  int getCommSize();
  int getCommRank();

private:
  void receive();
  void receiveMutexPacket(Packet& packet, int fromRank);
  void receiveCondvarPacket(Packet& packet, int fromRank);
  void run();
  void finish();

  static ProcessMonitor* pInstance;
  std::thread monitorThread;
  int commSize;
  int commRank;
  volatile bool shouldFinish = false;
  std::unordered_map<unsigned int, DistributedResource> idToRes;
  std::mutex guard;
};

#endif // DM_INCLUDE_PROCESS_MONITOR_H_
