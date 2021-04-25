CFLAGS= -g -O0 -std=c++11
LDLIBS+=-lm
LDFLAGS=-std=c++11
CC=g++

all: pci-id-updater
pci-id-updater: json.o pci-id-updater.o utils.o


pci-id-updater.o: pci-id-updater.cpp json.h pci-id-updater.h 

json.o: json.c  json.h

utils.o: utils.cpp pci-id-updater.h

%: %.o
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

clean:
	rm *.o pci-id-updater