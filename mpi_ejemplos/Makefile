.PHONY  = all clean
MPICC   = mpicc
CFLAGS += -Wall

EJECUTABLES = aloha alohabis alohater amoysenior anillob

all : $(EJECUTABLES)

aloha : aloha.c
	$(MPICC) $(CFLAGS) -o aloha $(^)

alohabis : alohabis.c
	$(MPICC) $(CFLAGS) -o alohabis $(^)

alohater : alohater.c
	$(MPICC) $(CFLAGS) -o alohater $(^)

amoysenior : amoysenior.c
	$(MPICC) $(CFLAGS) -o amoysenior $(^)

anillob : anillob.c
	$(MPICC) $(CFLAGS) -o anillob $(^)

clean :
	rm -f $(EJECUTABLES)

