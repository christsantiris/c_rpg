# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g
LIBS = -lncurses

# Directories
SRCDIR = src
INCDIR = include
OBJDIR = obj

# Source files
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

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

# Compile source files to object files
$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -I$(INCDIR) -c $< -o $@

# Clean up build files
clean:
	rm -rf $(OBJDIR) $(TARGET)

# Rebuild everything
rebuild: clean all

# Mark targets that don't create files
.PHONY: all clean rebuild