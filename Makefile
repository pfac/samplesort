CXX			=	g++
MPICXX		=	mpic++
CXXFLAGS	=	-Wall -Wextra -Wfatal-errors -std=c++0x -pedantic -O3 -I.

EXECS		=	mpi omp seq qsort

INPUT		=	input/10M.txt 5

MPINP		=	4

LINK.o = $(CXX) $(TARGET_ARCH) $(LDFLAGS)


.PHONY:	clean test

test-%:	%
	$< $(INPUT)

all:	$(EXECS)

profile: CXXFLAGS+=-DTK_PROFILE
profile: $(EXECS)

test:	$(EXECS:%=test-%)

test-profile: profile test

test-mpi:	mpi
	mpirun -np $(MPINP) $< $(INPUT)

mpi: CXX=$(MPICXX)

omp: LDFLAGS+=-fopenmp

mpi omp seq qsort: ccut-array.o

mpi.o: CXX=$(MPICXX)

omp.o: CXXFLAGS+=-fopenmp

mpi.o omp.o seq.o qsort.o: ccut-array.hpp ccut-array-inl.hpp

clean:
	$(RM) *.o $(EXECS)
