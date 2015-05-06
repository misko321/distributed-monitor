# distributed-monitor

### Description

This tool aims to aid programming distributed applicatons by implementing a distributed mutex. A set of those mutexes (each corresponding to a specific resource) is managed by a lower-level process monitor. However, the distributed mutex tries to hide the presence of that monitor, to make operating on it as much transparent for the end-programmer as possible.

### TODOS

1. TODO: I had some problems with a standard installation of MPI. Using default *mpic++* and *mpirun* some errors were thrown during runtime. So currently, I use manually installed binaries in *bin/* directory which is not included. Need to fix it later.

2. TODO: Use size-fixed types in Packet class to prevent errors resulting from different size of types on different machines (e.g *long* might use 4 bytes on machine A and 8 bytes on machine B).
