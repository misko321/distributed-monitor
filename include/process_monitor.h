#ifndef DM_INCLUDE_PROCESS_MONITOR_H_
#define DM_INCLUDE_PROCESS_MONITOR_H_

#include "distributed_mutex.h"

#include <thread>
#include <mutex>
#include <unordered_map>

class Packet;

class ProcessMonitor {
public:
  ProcessMonitor();
  ~ProcessMonitor();

  void send(int destination, Packet &packet);
  void broadcast(Packet &packet);
  void addMutex(DistributedMutex& mutex);
  void removeMutex(DistributedMutex& mutex);
  static ProcessMonitor& instance();
  int getCommSize();
  int getCommRank();

private:
  void receive();
  void run();
  void finish();
  
  static ProcessMonitor* pInstance;
  std::thread monitorThread;
  int commSize;
  int commRank;
  volatile bool shouldFinish = false;
  std::unordered_map<unsigned int, DistributedMutex&> resToMutex;
  std::mutex guard;
};

#endif // DM_INCLUDE_PROCESS_MONITOR_H_
