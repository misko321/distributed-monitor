#ifndef DISTRIBUTED_MUTEX
#define DISTRIBUTED_MUTEX

class DistributedMutex {
public:
  DistributedMutex();
  ~DistributedMutex();

  void acquire();
  void release();

private:
  int resourceId;
};

#endif
