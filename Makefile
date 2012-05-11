CXX			=	mpic++
CXXFLAGS	=	-Wall -Wextra -Wfatal-errors -std=c++0x -pedantic -g

EXECS		=	main

INPUT		=	int.txt 3

LINK.o = $(CXX) $(TARGET_ARCH) $(LDFLAGS)

.PHONY:	clean

test-%:	%
	mpirun -np 4 $< $(INPUT)

all:	$(EXECS)

main:	ccut-array.o

clean:
	$(RM) *.o $(EXECS)
