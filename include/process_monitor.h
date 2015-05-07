#ifndef DM_INCLUDE_PROCESS_MONITOR_H_
#define DM_INCLUDE_PROCESS_MONITOR_H_

#include <thread>

class Packet;

class ProcessMonitor {
public:
  ProcessMonitor();
  ~ProcessMonitor();

  void broadcast(Packet &packet);
  void receive();
  void run();
  void finish();
  static ProcessMonitor& instance();

private:
  static ProcessMonitor* pInstance; // TODO: volatile
  std::thread monitorThread;
  int comm_size;
  int comm_rank;
  volatile bool shouldFinish = false;
};

#endif // DM_INCLUDE_PROCESS_MONITOR_H_
