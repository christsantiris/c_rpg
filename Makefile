# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g
LIBS = -lncurses

# Directories
SRCDIR = src
INCDIR = include
OBJDIR = obj

# Source files - find all .c files recursively
SOURCES = $(shell find $(SRCDIR) -name '*.c')

# Object files - this is the corrected version
OBJECTS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SOURCES))

# Target executable
TARGET = conr

# Default rule
all: $(TARGET)

# Create object directory if it doesn't exist
$(OBJDIR):
	mkdir -p $(OBJDIR)

# Link object files to create executable
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LIBS)

# Compile source files to object files - auto-create subdirectories
$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I$(INCDIR) -c $< -o $@

# Clean up build files
clean:
	rm -rf $(OBJDIR) $(TARGET)

# Rebuild everything
rebuild: clean all

# Mark targets that don't create files
.PHONY: all clean rebuild