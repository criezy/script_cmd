CC=g++
CFLAGS=-c -Wall
LDFLAGS=

SOURCES=\
	str.cpp\
	strstream.cpp\
	parser_operators.cpp\
	equation_parser.cpp\
	script_parser.cpp\
	equation_module.cpp\
	script_module.cpp\
	main.cpp
 
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=script_cmd

all: depend $(SOURCES) $(EXECUTABLE)

depend: .depend

.depend: $(SOURCES)
	rm -f ./.depend
	$(CC) $(CFLAGS) -MM $^ > ./.depend;

include .depend

	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *o $(EXECUTABLE)
