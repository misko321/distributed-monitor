#ifndef DM_INCLUDE_DISTRIBUTED_MUTEX_H_
#define DM_INCLUDE_DISTRIBUTED_MUTEX_H_

class DistributedMutex {
public:
  DistributedMutex();
  ~DistributedMutex();


  void acquire();
  void release();

  int getResourceId();

private:
  int resourceId;
  long localClock = 0;
  long highestClock = 0;
  bool interestedInCriticalSection = false;
};

#endif // DM_INCLUDE_DISTRIBUTED_MUTEX_H_
