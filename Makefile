CFLAGS= -g -O0 -std=c++11
LDLIBS+=-lm
LDFLAGS=-std=c++11
CC=g++

all: pci-id-updater
pci-id-updater: json.o pci-id-updater.o 


pci-id-updater.o: pci-id-updater.cpp  json.h
	$(CC) -std=c++11 -Wfatal-errors pci-id-updater.cpp $(LDLIBS) -o $@

json.o: json.c  json.h
	$(CC) -std=c++11 json.c $(LDLIBS) -o $@

%: %.o
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

clean:
	rm *.o pci-id-updater