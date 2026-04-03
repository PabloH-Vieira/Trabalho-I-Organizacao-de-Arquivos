# Variables for compiler and flags
CC = gcc
CFLAGS = -Wall -g

# The final executable name
TARGET = main

# Build the executable from source files
$(TARGET): main.c
	$(CC) $(CFLAGS) -o $(TARGET) main.c file.c

# Clean up build files
clean:
	rm -f $(TARGET)
