# Compiler settings
CC        = gcc             # Use gcc
CFLAGS    = -fopenmp       # enable OpenMP
LDFLAGS   = -fopenmp        # link OpenMP library

# Name of the program to generate
PROGRAM   = max_char       # Name of the binary (update if needed)
SRCS      = openmpmech.c     # Your source file (update to actual file name)
OBJS      = $(SRCS:.c=.o)   # Object file(s)

#
# Targets
#

default: all

all: $(PROGRAM)

$(PROGRAM): $(OBJS)
	$(CC) $(OBJS) -o $(PROGRAM) $(LDFLAGS)

clean:
	/bin/rm -f $(OBJS) $(PROGRAM)