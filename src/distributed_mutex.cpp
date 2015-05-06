#include "distributed_mutex.h"
#include <iostream>
using namespace std;

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