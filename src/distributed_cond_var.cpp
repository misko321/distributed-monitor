#include "distributed_cond_var.h"
#include "process_monitor.h"

DistributedCondvar::DistributedCondvar(int condvarId) : condvarId(condvarId) {
}

DistributedCondvar::~DistributedCondvar() {
}

/* This method assumes, that [mutex] has already been acquired. */
template<class Predicate>
void DistributedCondvar::wait(DistributedMutex& mutex, Predicate pred) {
  //here the process is in a critical section, so broadcast messages will be total ordered
  while (!pred()) {
    mutex.release();
    Packet packet = Packet(mutex.getLocalClock(), Packet::Type::DM_CONDVAR_WAIT, condvarId);
    ProcessMonitor::instance().broadcast(packet);

    std::unique_lock<std::mutex> lock(mutex);
    condVarLocal.wait(lock); //TODO spurious wakeup?
    
    mutex.acquire();
  }
}

void DistributedCondvar::notifyOne() {

}

void onNotify() {

}

int DistributedCondvar::getCondvarId() {
  return condvarId;
}
