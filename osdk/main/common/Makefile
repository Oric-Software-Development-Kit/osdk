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

VPATH    = sources
SOURCES := $(subst sources/,,$(wildcard sources/*.cpp))
OBJS    := $(SOURCES:.cpp=.o)

CPPFLAGS += -I sources -I includes
CPPFLAGS += -D_LIB

# macOS extras (append only)
ifeq ($(UNAME_S),Darwin)
    CXXFLAGS += -std=c++14
endif

.PHONY: all clean

all: libcommon.a

libcommon.a: $(OBJS)
	$(AR) $(ARFLAGS) $@ $^
	# $(RANLIB) $@          # uncomment if you need ranlib

clean:
	rm -f libcommon.a $(OBJS)
