include ../../../rules.mk

# ------------------------------------------------------------------
# Force Homebrew GCC on macOS so we never fall back to system g++
# (Apple Clang → libc++). This must be consistent with libcommon.
# ------------------------------------------------------------------
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    CXX := g++-14
    CC  := gcc-14
endif

CPPFLAGS += -I ../common/includes
CPPFLAGS += -DFREEIMAGE_LIB

FI_PATH := ../../../shared_libraries/freeimage/v3.12.0
NS_PATH := ../../../shared_libraries/nanosvg

ifeq ($(PLATFORM),win32)
    CPPFLAGS += -I libs
    LDFLAGS  += -L libs -lFreeImage
else
    # Prefer Homebrew FreeImage on macOS when available
    ifeq ($(UNAME_S),Darwin)
        FI_PREFIX := $(shell brew --prefix freeimage 2>/dev/null)
        ifneq ($(FI_PREFIX),)
            CPPFLAGS += -I $(FI_PREFIX)/include
            LDFLAGS  += -L $(FI_PREFIX)/lib -lfreeimage $(MATH_LIBS)
        else
            # Fallback to the bundled copy
            CPPFLAGS += -I $(FI_PATH)/ -I $(FI_PATH)/Source/
            LDFLAGS  += -L $(FI_PATH)/ -lfreeimage $(MATH_LIBS)
        endif
    else
        # Non-macOS Unix: use the bundled FreeImage
        CPPFLAGS += -I $(FI_PATH)/ -I $(FI_PATH)/Source/
        LDFLAGS  += -L $(FI_PATH)/ -lfreeimage $(MATH_LIBS)
    endif
endif

# macOS-specific extras (append, do not overwrite flags from rules.mk)
ifeq ($(UNAME_S),Darwin)
    CXXFLAGS += -std=c++14
    CFLAGS   += -Wno-implicit-function-declaration
    # CFLAGS += -std=gnu99   # uncomment if C23 keyword problems appear
endif

CPPFLAGS += -I $(NS_PATH)/src
CPPFLAGS += -I sources -I includes

# common library + curses / C++ runtime (from rules.mk)
LDFLAGS += -L ../common -lcommon $(COMMON_EXTRA_LDFLAGS)

SOURCES := $(notdir $(wildcard sources/*.cpp))
# filter out some test sources...
SOURCES := $(filter-out %video.cpp,$(SOURCES))
VPATH    = sources
OBJS     := $(SOURCES:.cpp=.o)

EXECUTABLE = pictconv$(EXE)

.PHONY: all clean

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJS)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ $(OBJS) $(LDFLAGS)

clean:
	rm -f $(EXECUTABLE) $(OBJS)
