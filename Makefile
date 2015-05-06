INCDIR=include
CC=./bin/mpic++
CFLAGS=-I$(INCDIR) -Wall --std=c++11 -O3

OBJDIR=obj

SRCDIR=src

_DEPS = distributed_mutex.h
DEPS = $(patsubst %,$(INCDIR)/%,$(_DEPS))

_OBJ = distributed_mutex.o main.o
OBJ = $(patsubst %,$(OBJDIR)/%,$(_OBJ))


$(OBJDIR)/%.o: $(SRCDIR)/%.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

distr_mon: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)


.PHONY: clean

clean:
	rm -f $(OBJDIR)/*.o *~ core $(INCDIR)/*~
