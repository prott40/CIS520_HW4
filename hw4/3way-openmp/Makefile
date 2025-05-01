# Compiler and flags
CC        = gcc
CFLAGS    = -fopenmp
LDFLAGS   = -fopenmp

# Build mode: 'release' (with -O2) or 'debug' (with -g -O0)
BUILD     ?= release

ifeq ($(BUILD),release)
	CFLAGS += -O2
else ifeq ($(BUILD),debug)
	CFLAGS += -g -O0
endif

# Source files and target binaries
SRCS      = openmpmechV1.c openmpmechV2.c openmpmechV3.c
PROGS     = max_charV1 max_charV2 max_charV3
OBJS      = $(SRCS:.c=.o)

# Default target
default: all

# Build all programs
all: $(PROGS)

# Rule to build each binary from its object
max_charV%: openmpmechV%.o
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

# Rule to compile each .c into .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up
clean:
	/bin/rm -f $(OBJS) $(PROGS)