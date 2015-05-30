#ifndef DM_INCLUDE_DISTRIBUTED_MUTEX_H_
#define DM_INCLUDE_DISTRIBUTED_MUTEX_H_

#include <condition_variable>

#include "packet.h"

template<typename R> class ProcessMonitor;

class DistributedMutex {
  friend class ProcessMonitor;
public:
  DistributedMutex();
  ~DistributedMutex();

  void acquire();
  void release();

  unsigned int getResourceId();
  long getLocalClock();
  void onReply(int sourceCommRank);
  void onRequest(int sourceCommRank, long packetClock);

private:
  unsigned int resourceId;
  long localClock = 0;
  long highestClock = 0;
  bool interestedInCriticalSection = false;
  int repliesNeeded;
  int* waitsForReply;
  std::condition_variable waitCondition;

  int rank();
};

#endif // DM_INCLUDE_DISTRIBUTED_MUTEX_H_
