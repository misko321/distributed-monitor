INCDIR=include
CC=mpic++
CFLAGS=-I$(INCDIR) -Wall --std=c++11 -O3

OBJDIR=obj

SRCDIR=src

_DEPS = distributed_mutex.h process_monitor.h packet.h \
	distributed_condvar.h distributed_resource.h
DEPS = $(patsubst %,$(INCDIR)/%,$(_DEPS))

_OBJ = distributed_mutex.o process_monitor.o packet.o main.o \
	distributed_condvar.o distributed_resource.o
OBJ = $(patsubst %,$(OBJDIR)/%,$(_OBJ))


$(OBJDIR)/%.o: $(SRCDIR)/%.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

distr_mon: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)


.PHONY: clean

clean:
	rm -f $(OBJDIR)/*.o *~ core $(INCDIR)/*~
