CXX			=	g++
MPICXX		=	mpic++
CXXFLAGS	=	-Wall -Wextra -Wfatal-errors -std=c++0x -pedantic -O3 -I.

EXECS		=	mpi qsort

INPUT		=	int.txt 3

LINK.o = $(CXX) $(TARGET_ARCH) $(LDFLAGS)

.PHONY:	clean

test-%:	%
	mpirun -np 4 $< $(INPUT)

all:	$(EXECS)

profile: CXXFLAGS+=-DTK_PROFILE
profile: $(EXECS)

mpi: CXX=$(MPICXX)
mpi: mpi.o ccut-array.o

qsort: ccut-array.o

mpi.o: CXX=$(MPICXX)
mpi.o: ccut-array.hpp ccut-array-inl.hpp

qsort.o: ccut-array.hpp ccut-array-inl.hpp


clean:
	$(RM) *.o $(EXECS)
