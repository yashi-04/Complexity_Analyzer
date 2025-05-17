CC = gcc
CFLAGS = -Wall -Wextra -fPIC -I/opt/homebrew/include
LDFLAGS = -L/opt/homebrew/lib -ljson-c

TARGET = libanalyzer.dylib
SRC = analyzer.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -shared -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: all clean 