COMPILER=gcc
CFLAGS=-Wall -O3 -s
OBJECTS=src/filetrack.o
SOURCES=src/filetrack.c
APP_NAME=bin/filetrack

all: $(OBJECTS)
	$(COMPILER) $(CFLAGS) $(OBJECTS) -o $(APP_NAME)

filetrack: $(SOURCES)
	$(COMPILER) $(CFLAGS) $(SOURCES) -o $(APP_NAME)

depend:
	makedepend $(CFLAGS) $(SOURCES)

clean:
	rm ./*.o *.~ $(APP_NAME)
