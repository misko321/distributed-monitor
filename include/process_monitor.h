#ifndef DM_INCLUDE_PROCESS_MONITOR_H_
#define DM_INCLUDE_PROCESS_MONITOR_H_

#include <thread>
#include <mutex>
#include <unordered_map>

class Packet;
class DistributedResource;
struct DistributedResource;

class ProcessMonitor {
public:
  ProcessMonitor();
  ~ProcessMonitor();

  static ProcessMonitor& instance();

  void addResource(DistributedResource& resource);
  void removeResource(DistributedResource& resource);

  void sendPacket(int destination, Packet &packet);
  void broadcastPacket(Packet &packet);
  void broadcastResource(int id, void* resource, int size);

  int getCommSize();
  int getCommRank();

private:
  void receivePacket();
  void receiveResource(int resourceId);
  void run();
  void finish();

  static ProcessMonitor* pInstance;
  std::thread monitorThread;
  int commSize;
  int commRank;
  volatile bool shouldFinish = false;
  std::unordered_map<unsigned int, DistributedResource&> idToRes;
  std::mutex guard;
};

#endif // DM_INCLUDE_PROCESS_MONITOR_H_
