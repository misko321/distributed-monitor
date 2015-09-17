# distributed-monitor

## Description

This tool aims to aid programming distributed applications by implementing an easy-to-use mechanism
for acquiring mutual exclusion on shared resources.
A set of those resources is managed by a lower-level process monitor. However, the distributed
resource tries to hide the presence of that monitor, to make operating on it as much transparent
for the end-programmer as possible.

Under the hood it draws on Ricart-Agrawala algorithm
(check [original work](http://www.cs.ucf.edu/courses/cop6614/fall2005/Ricart-Agrawala.pdf) or [Wikipedia entry](en.wikipedia.org/wiki/Ricart–Agrawala_algorithm)) for achieving distributed mutual exclusion (DME).

## Usage

#### Creating a distributed resource
The only class you need to use is **DistributedResource**. It allows for painless *acquiring*
and *releasing* critical section required to modify the resource safely:

DistributedResource constructor takes 3 parameters:

```
DistributedResource resource(<res_id>, <res_address>, <res_size>);

//Create distributed resource for variable 'num' with ID = 123
DistributedResource resource(123, &num, sizeof(num));
```

#### Acquiring and releasing a mutex
A process can afterwards acquire the mutex on the resource to make sure it's the only process
that enters the *distributed critical section*. After performing action in critical section the
requesting process ought to release it, thus allowing processes running on other nodes to perform
their actions in critical section. This is done with `lock()` and `unlock()` methods:

```
resource.lock();
/* HERE GOES THE ACTION UNDERTAKEN IN CRITICAL SECTION */
resource.unlock();
```
#### Wating on a condition

DistributedResource makes it also possible to acquire the mutex when specified condition holds
true with `wait()` and `notify()` methods.

```
//Consumer
resource.lock(); //resource must be first locked
resource.wait([&num] () -> bool { //consumer need 5 units to work
  return num >= 5;
});
//CRITICAL SECTION
resource.notify(); //wake others waiting on this variable, if any
resource.unlock();

---------------------------------------

//Producer
resource.lock();
num += 1; //produce 1 unit
if (num >= 5)
  resource.notify();

resource.unlock();
```

## Example

An example is given in **src/main.cpp**. Below you can find an example output from executing
this example on 8 processes along with an explanation:

```
//Every messages comes in format:
//[rank of the process]: [message]

1: MPI Initialized successfully :: size: 8, rank: 1 //[1]
0: MPI Initialized successfully :: size: 8, rank: 0
5: MPI Initialized successfully :: size: 8, rank: 5
0: Resource with id = 1234 created successfully
2: MPI Initialized successfully :: size: 8, rank: 2
6: MPI Initialized successfully :: size: 8, rank: 6
2: Resource with id = 1234 created successfully
6: Resource with id = 1234 created successfully
1: Resource with id = 1234 created successfully
5: Resource with id = 1234 created successfully
7: MPI Initialized successfully :: size: 8, rank: 7
7: Resource with id = 1234 created successfully
3: MPI Initialized successfully :: size: 8, rank: 3
3: Resource with id = 1234 created successfully
4: MPI Initialized successfully :: size: 8, rank: 4
4: Resource with id = 1234 created successfully
3: <--- CRITICAL SECTION :: num = 1 //[2]
4: <--- CRITICAL SECTION :: num = 2
5: <--- CRITICAL SECTION :: num = 3
6: <--- CRITICAL SECTION :: num = 4
7: <--- CRITICAL SECTION :: num = 5 //[3]
3: <--- CRITICAL SECTION :: num = 6
4: <--- CRITICAL SECTION :: num = 7
5: <--- CRITICAL SECTION :: num = 8
6: <--- CRITICAL SECTION :: num = 9
0: <--- CRITICAL SECTION :: num = 4 CONSUMING (0) //[4]
7: <--- CRITICAL SECTION :: num = 5
3: <--- CRITICAL SECTION :: num = 6
4: <--- CRITICAL SECTION :: num = 7
5: <--- CRITICAL SECTION :: num = 8
6: <--- CRITICAL SECTION :: num = 9
1: <--- CRITICAL SECTION :: num = 4 CONSUMING (1)
7: <--- CRITICAL SECTION :: num = 5
3: <--- CRITICAL SECTION :: num = 6
4: <--- CRITICAL SECTION :: num = 7
5: <--- CRITICAL SECTION :: num = 8
6: <--- CRITICAL SECTION :: num = 9
2: <--- CRITICAL SECTION :: num = 4 CONSUMING (2)
7: <--- CRITICAL SECTION :: num = 5
3: <--- CRITICAL SECTION :: num = 6
4: <--- CRITICAL SECTION :: num = 7
5: <--- CRITICAL SECTION :: num = 8
6: <--- CRITICAL SECTION :: num = 9
0: <--- CRITICAL SECTION :: num = 4 CONSUMING (0)
7: <--- CRITICAL SECTION :: num = 5
3: <--- CRITICAL SECTION :: num = 6
4: <--- CRITICAL SECTION :: num = 7
5: <--- CRITICAL SECTION :: num = 8
6: <--- CRITICAL SECTION :: num = 9
1: <--- CRITICAL SECTION :: num = 4 CONSUMING (1)
7: <--- CRITICAL SECTION :: num = 5
3: <--- CRITICAL SECTION :: num = 6
4: <--- CRITICAL SECTION :: num = 7
5: <--- CRITICAL SECTION :: num = 8
6: <--- CRITICAL SECTION :: num = 9
2: <--- CRITICAL SECTION :: num = 4 CONSUMING (2)
7: <--- CRITICAL SECTION :: num = 5
3: <--- CRITICAL SECTION :: num = 6
4: <--- CRITICAL SECTION :: num = 7
5: <--- CRITICAL SECTION :: num = 8
6: <--- CRITICAL SECTION :: num = 9
0: <--- CRITICAL SECTION :: num = 4 CONSUMING (0) //[5]

[1] Basic initalization messages
[2] Process no. 3 enters the critical section and increments 'num'
[3] Have in mind that although 'num' reached value allowing consumer to enter critical section, it must wait for its turn and let pass producers that have already reported their will to enter critical section
[4] When condition num >= holds true, process with rank 0 consumes 5 units from 'num'.
[5] You can see that consumers are enqueued, thus each consumer gains an exclusive access to shared resource in finite time, what prevents starvation.
```

## Project file structure
All source code is located under **src/** and **include/** directories. `ProcessMonitor` is a class that manages entire interprocess communication. It sends messages that are instances of `Packet` class and dispatches incoming packets to `DistributedResource`. Each `DistributedResource` holds instance of `DistributedMutex` (for `lock()` and `unlock()`) and `DistributedCondvar` for `wait()` and `notify()`.

## License
This project is realesed under [MIT License](http://choosealicense.com/licenses/mit/).
