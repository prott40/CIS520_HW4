
# Compiler settings
CC        = gcc             # Use gcc for pthread program (not mpicc)
CFLAGS    = -pthread        # Add pthread flag for multi-threading support
LDFLAGS   = -pthread        # Link pthread library

# Name of the program to generate
PROGRAM   = max_char       # Name of the binary (update if needed)
SRCS      = pthreadmech.c     # Your source file (update to actual file name)
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
