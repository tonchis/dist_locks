MPICC   = mpicc
MPIEXEC = mpiexec
CFLAGS += -Wall

EJECUTABLE = tp3

.PHONY : all new clean run

all : $(EJECUTABLE)

tp3 : tp3.c srv.c lista.c
	$(MPICC) $(CFLAGS) -o tp3 tp3.c srv.c lista.c

run : tp3
	$(MPIEXEC) -np 2 ./$(EJECUTABLE) '.' 2 50 500 >/dev/null

clean :
	rm -f $(EJECUTABLE)

new : clean all
