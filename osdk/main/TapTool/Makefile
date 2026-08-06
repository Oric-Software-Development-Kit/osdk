include ../../../rules.mk

# ------------------------------------------------------------------
# Force Homebrew GCC on macOS – never use system g++ (Apple Clang)
# ------------------------------------------------------------------
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    CXX := g++-14
    CC  := gcc-14
endif

ifeq ($(UNAME_S),Linux)
    CXX := g++-14
    CC  := gcc-14
endif

CPPFLAGS += -Wall -Iincludes
CPPFLAGS += -I ../common/includes

LDLIBS   += -L ../common -lcommon $(COMMON_EXTRA_LDFLAGS)

CXXFLAGS += -Wno-unknown-pragmas

SOURCES=sources/TapTool.cpp
 
OBJECTS    = $(SOURCES:.cpp=.o)
EXECUTABLE=TapTool$(EXE)

.PHONY: all clean

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@

sources/Link65.o: sources/Link65.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<

clean:
	rm -f $(EXECUTABLE) $(OBJECTS)
