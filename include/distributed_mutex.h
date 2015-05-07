#ifndef DM_INCLUDE_DISTRIBUTED_MUTEX_H_
#define DM_INCLUDE_DISTRIBUTED_MUTEX_H_

class DistributedMutex {
public:
  DistributedMutex(int resourceId);
  ~DistributedMutex();


  void acquire();
  void release();

  unsigned int getResourceId();

private:
  unsigned int resourceId;
  long localClock = 0;
  long highestClock = 0;
  bool interestedInCriticalSection = false;
};

#endif // DM_INCLUDE_DISTRIBUTED_MUTEX_H_
