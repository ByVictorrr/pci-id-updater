CFLAGS=$(OPT) -Wall -W -Wno-parentheses -Wstrict-prototypes -Wmissing-prototypes -g -O0
LDLIBS+=-lm

all: pci-id-updater
pci-id-updater: json.o pci-id-updator.o 

pci-id-updater.o: pci-id-updater.cpp json.h
json.o: json.c json.h

%: %.o
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

