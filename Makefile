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
INCDIRS=-I ./include
MAKEFLAGS=-j
OBJDIR=./objs
SRCDIR=./src
SRCS=elevator.cxx \
	elevator_bank_panel.cxx \
	elevator_scheduler.cxx \
	main.cxx
OBJS=$(SRCS:%.cxx=$(OBJDIR)/%.o)
EXE=breakneck

release: CXXFLAGS+=-DNDEBUG
release: $(EXE)

debug: CXXFLAGS+=-g
debug: $(EXE)

clean:
	rm -rf $(OBJDIR) $(EXE) $(EXE).dSYM


$(EXE): $(OBJS)
	$(CXX) $(OBJS) -o $(EXE)

$(OBJS): | $(OBJDIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)

-include $(SRCS:%.cxx=$(OBJDIR)/%.d)
$(OBJDIR)/%.o: $(SRCDIR)/%.cxx
	$(CXX) $(CXXFLAGS) $(INCDIRS) -MD -c $< -o $@

.PHONY: release debug clean
