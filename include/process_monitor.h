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

  void broadcast(Packet &packet);
  void receive();
  void run();
  void finish();
  void addMutex(DistributedMutex& mutex);
  void removeMutex(DistributedMutex& mutex);
  static ProcessMonitor& instance();

private:
  static ProcessMonitor* pInstance; // TODO: volatile
  std::thread monitorThread;
  int comm_size;
  int comm_rank;
  volatile bool shouldFinish = false;
  std::unordered_map<int, DistributedMutex&> resToMutex;
  std::mutex guard;
};

#endif // DM_INCLUDE_PROCESS_MONITOR_H_
