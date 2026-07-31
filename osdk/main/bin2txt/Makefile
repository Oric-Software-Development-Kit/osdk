include ../../../rules.mk

# ------------------------------------------------------------------
# Force Homebrew GCC on macOS – never use system g++ (Apple Clang)
# ------------------------------------------------------------------
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    CXX := g++-14
    CC  := gcc-14
endif

CPPFLAGS += -Wall -Iincludes
CPPFLAGS += -I ../common/includes

LDLIBS   += -L ../common -lcommon $(COMMON_EXTRA_LDFLAGS)

CXXFLAGS += -Wno-unknown-pragmas

SOURCES=sources/bin2txt.cpp

OBJECTS    = $(SOURCES:.cpp=.o)
EXECUTABLE = bin2txt$(EXE)

.PHONY: all clean

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@

sources/Link65.o: sources/Link65.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<

clean:
	rm -f $(EXECUTABLE) $(OBJECTS)
