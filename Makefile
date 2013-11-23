CC=g++
CPPFLAGS=-c -Wall
LDFLAGS=

# To use readline
#CPPFLAGS += -DUSE_READLINE
#LDFLAGS += -lreadline

SOURCES=\
	str.cpp\
	strstream.cpp\
	redirect_output.cpp\
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
	$(CC) $(CPPFLAGS) -MM $^ > ./.depend;

include .depend

	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CPPFLAGS) $< -o $@

clean:
	rm -rf *o $(EXECUTABLE)
