#include "distributed_mutex.h"
#include <iostream>
using std::cout;
using std::endl;

DistributedMutex::DistributedMutex() {
  cout << "Creating new mutex" << endl;
}

DistributedMutex::~DistributedMutex() {
  cout << "Destroying mutex" << endl;
}

void DistributedMutex::acquire() {
  cout << "acquire" << endl;
}

void DistributedMutex::release() {
  cout << "release" << endl;
}
