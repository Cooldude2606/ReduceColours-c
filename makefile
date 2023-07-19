CC ?= gcc

override CFLAGS := -O3 -W -Wall -Wextra -pedantic $(CFLAGS)

LIBS := m
src := src/images.c src/octTree.c src/priorityQueue.c src/hashMap.c src/vector3.c
src_o := src/images.o src/octTree.o src/priorityQueue.o src/hashMap.o src/vector3.o

ext_libs := libs/lodepng/lodepng.o libs/nanojpeg/nanojpeg.o

all: app multi

app: src/main.o $(src_o) $(ext_libs)
	$(CC) $(CFLAGS) $^ -l$(LIBS) -o $@

multi: src/multi.o $(src_o) $(ext_libs)
	$(CC) $(CFLAGS) $^ -l$(LIBS) -lpthread -o $@

test: src/tests.c $(src) $(ext_libs)
	$(CC) $(CFLAGS) -D _TESTS $^ -l$(LIBS) -o $@

debug: src/main.c $(src) $(ext_libs)
	$(CC) -D _DEBUG -g3 $^ -l$(LIBS) -o $@

# This is a mothballed version using an unstable jpeg lib
appAlt: mothball/mainJpeg.o libs/jpeg/djpg.so $(src_o) $(ext_libs)
	$(CC) $(CFLAGS) $^ -l$(LIBS) -o $@

# The unstable jpeg lib mentioned above
libs/jpeg/djpg.so: libs/jpeg/src/djpg.c libs/jpeg/src/didct.c libs/jpeg/src/bitmap.c
	$(CC) --shared -fPIC -w -O3 $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f src/main.o src/multi.o src/tests.o $(src_o)

.PHONY: all clean