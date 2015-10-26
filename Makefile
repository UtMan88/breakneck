CXX=clang++
CXXFLAGS=-fexceptions \
	 -fstrict-aliasing \
	 -pedantic \
	 -Wall \
	 -Wcast-align \
	 -Wcast-qual \
	 -Wdisabled-optimization \
	 -Wextra \
	 -Wformat \
	 -Wshadow \
	 -Wsign-conversion \
	 -Wundef \
	 -Wno-unused-parameter \
	 -Wno-unused-variable \
	 -Wno-variadic-macros \
	 -std=c++11 \
	 -x c++
INCDIRS=-Isrc
SOURCES=src/elevator.cc \
	src/elevator_bank_panel.cc \
	src/elevator_scheduler.cc \
	src/main.cc
OBJECTS=$(SOURCES:.cc=.o)
EXECUTABLE=breakneck

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@

.cc.o:
	$(CXX) $(CXXFLAGS) $(INCDIRS) -c $< -o $@

clean:
	rm -f src/*.o $(EXECUTABLE)
