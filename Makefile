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
MAKEFLAGS=-j
SOURCES=src/elevator.cxx \
	src/elevator_bank_panel.cxx \
	src/elevator_scheduler.cxx \
	src/main.cxx
OBJECTS=$(SOURCES:.cxx=.o)
EXECUTABLE=breakneck

all: $(SOURCES) release

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(EXECUTABLE)

debug: CXXFLAGS+=-g
debug: $(EXECUTABLE)

release: CXXFLAGS+=-DNDEBUG
release: $(EXECUTABLE)

-include $(SOURCES:.cxx=.d)
%.o: %.cxx
	$(CXX) $(CXXFLAGS) $(INCDIRS) -MD -c $< -o $@

clean:
	rm -rf src/*.o $(EXECUTABLE) $(EXECUTABLE).dSYM

cleandepend:
	rm -rf src/*.d

.PHONY: all debug release clean cleandepend
