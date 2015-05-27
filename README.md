# distributed-monitor

### Description

This tool aims to aid programming distributed applications by implementing a distributed mutex.
A set of those mutexes (each corresponding to a specific resource) is managed by a lower-level
process monitor. However, the distributed mutex tries to hide the presence of that monitor,
to make operating on it as much transparent for the end-programmer as possible.

Under the hood it draws on Ricart-Agrawala algorithm (check [original work](http://www.cs.ucf.edu/courses/cop6614/fall2005/Ricart-Agrawala.pdf) or [Wikipedia entry](en.wikipedia.org/wiki/Ricart–Agrawala_algorithm)) for achieving distributed mutual exclusion (DME).

### Usage

The only class You need to use is **DistributedMutex**. It allows for easy *acquiring* and *releasing* the mutex:

Default constructor takes 1 parameter, which is the ID of the resource the mutex should apply to:

```
//Creates distributed mutex for resource with ID = 123
DistributedMutex distMutex(123);
```
Then a process can acquire the mutex to make sure it's the only process that enters the *distributed critical section*. After performing action in critical section the requesting process ought to release it, thus allowing processes running on other nodes to perform their actions in critical section:

```
distMutex.acquire();
/* HERE GOES THE ACTION UNDERTAKEN IN CRITICAL SECTION */
distMutex.release();
```
### Example

An example is given in `src/main.cpp`.

### TODO
Describe project file structure.
