CXX			=	g++
MPICXX		=	mpic++
#CXXFLAGS	=	-Wall -Wextra -Wfatal-errors -std=c++0x -pedantic -O3 -I.
CXXFLAGS	=	-Wall -Wextra -Wfatal-errors -I.

EXECS		=	mpi omp seq tbb qsort

INPUT		=	input/10M.txt 5

MPINP		=	4

LINK.o = $(CXX) $(TARGET_ARCH) $(LDFLAGS)


MODE		?= 	RLS

ifeq ($(MODE),DBG)
CXXFLAGS	+=	-g
else
ifeq ($(MODE),RLS)
CXXFLAGS	+=	-O3
endif
endif


.PHONY:	clean test

test-%:	%
	$< $(INPUT)

%.mpio: CXX=$(MPICXX)
%.mpio: %.cc
	$(COMPILE.cc) $(OUTPUT_OPTION) $<

default: tbb

all:	$(EXECS)

profile: CXXFLAGS+=-DTK_PROFILE
profile: $(EXECS)

test:	$(EXECS:%=test-%)

test-profile: profile test

test-mpi:	mpi
	mpirun -np $(MPINP) $< $(INPUT)

mpi: CXX=$(MPICXX)
mpi: ccut-array.mpio

mpi.o: CXX=$(MPICXX)

omp: LDFLAGS+=-fopenmp

omp.o: CXXFLAGS+=-fopenmp

tbb: LDFLAGS += -L $(HOME)/src/tbb/lib -ltbb

tbb.o: CXXFLAGS += -I $(HOME)/src/tbb/include

omp seq tbb qsort: ccut-array.o

mpi.o omp.o seq.o tbb.o qsort.o: ccut-array.hpp ccut-array-inl.hpp

clean:
	$(RM) *.o *.mpio $(EXECS)
