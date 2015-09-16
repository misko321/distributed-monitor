#ifndef DM_INCLUDE_DISTRIBUTED_MUTEX_H_
#define DM_INCLUDE_DISTRIBUTED_MUTEX_H_

#include <condition_variable>

#include "packet.h"

class ProcessMonitor;

class DistributedMutex {
public:
  DistributedMutex(unsigned int id);
  ~DistributedMutex();

  void acquire();
  void release();

  unsigned int getResourceId();
  long getLocalClock();
  void onReply(int sourceCommRank);
  void onRequest(int sourceCommRank, long packetClock);

private:
  unsigned int id;
  long localClock = 0;
  long highestClock = 0;
  bool interestedInCriticalSection = false;
  int repliesNeeded;
  int* waitsForReply;
  std::condition_variable waitForReplies;
  std::mutex guard;

  int rank();
};

#endif // DM_INCLUDE_DISTRIBUTED_MUTEX_H_
