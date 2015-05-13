#ifndef DM_INCLUDE_DISTRIBUTED_MUTEX_H_
#define DM_INCLUDE_DISTRIBUTED_MUTEX_H_

#include <condition_variable>

#include "packet.h"

class DistributedMutex {
public:
  DistributedMutex(int resourceId);
  ~DistributedMutex();


  void acquire();
  void release();

  unsigned int getResourceId();
  // std::condition_variable& getWaitCondition();
  // void notify();
  void onReply(int source, Packet& packet);
  void onRequest(int source, Packet& packet);

private:
  unsigned int resourceId;
  long localClock = 0;
  long highestClock = 0;
  bool interestedInCriticalSection = false;
  int repliesNeeded;
  int* waitsForReply;

  std::condition_variable waitCondition;
};

#endif // DM_INCLUDE_DISTRIBUTED_MUTEX_H_
