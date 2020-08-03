CC=g++-8
CFLAGS = -std=c++2a -O3
LDFLAGS = -lstdc++fs -pthread

src = $(wildcard src/*.cpp)
obj = $(src:.c=.o)

lib: $(obj)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

all: lib

clean:
	rm -f $(obj) lib