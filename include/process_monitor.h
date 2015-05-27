#ifndef DM_INCLUDE_PROCESS_MONITOR_H_
#define DM_INCLUDE_PROCESS_MONITOR_H_

#include "distributed_mutex.h"
// #include "distributed_condvar.h"

#include <thread>
#include <mutex>
#include <unordered_map>

class Packet;
class DistributedCondvar;

class ProcessMonitor {
public:
  ProcessMonitor();
  ~ProcessMonitor();

  static ProcessMonitor& instance();
  void addMutex(DistributedMutex& mutex);
  void removeMutex(DistributedMutex& mutex);
  void addCondvar(DistributedCondvar& condvar);
  void removeCondvar(DistributedCondvar& condvar);

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
  std::unordered_map<unsigned int, DistributedMutex&> resToMutex;
  std::unordered_map<unsigned int, DistributedCondvar&> resToCondvar;
  std::mutex guard;
};

#endif // DM_INCLUDE_PROCESS_MONITOR_H_
