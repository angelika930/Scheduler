# Compiler and Flags
CC = gcc
CFLAGS = -fPIC -Wall -g
LDFLAGS = -shared

# Source Files
SRC = lwp.c scheduler.c
OBJ = $(SRC:.c=.o)

# Target Shared Library
TARGET = liblwp.so

# Default Target
all: $(TARGET)

# Rule to Build Shared Library
$(TARGET): $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $(OBJ)

# Object File Compilation Rule
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean Target
clean:
	rm -f $(OBJ) $(TARGET)

# Phony Targets
.PHONY: all clean
