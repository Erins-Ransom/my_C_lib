IDIR=include
CC=gcc
CFLAGS=-g -I$(IDIR)

SDIR=src

ODIR=$(SDIR)/obj
LIBS=-lpthread

_DEPS = utils.h avl_tree.h format.h hash.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = utils.o avl_tree.o tests.o hash.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))



$(ODIR)/%.o: $(SDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

tests: $(OBJ)
	$(CC) -o  $@ $^ $(CFLAGS) $(LIBS)

pthread_practice: $(ODIR)/pthread_practice.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o tests test_file pthread_practice