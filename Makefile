# ---------------------------------------------------------------
# Compiler selection (prefer Homebrew GCC if available)
# ---------------------------------------------------------------
CXX      ?= g++-14          # or g++-15 / g++-16 if you prefer
CC       ?= gcc-14

# ---------------------------------------------------------------
# Base flags
# ---------------------------------------------------------------
CFLAGS   = -Wall -D_DEBUG -D__cdecl= -DPOSIX -Iincludes
CXXFLAGS = -Wall -D_DEBUG -D__cdecl= -DPOSIX -Iincludes
LDFLAGS  =

# ---------------------------------------------------------------
# macOS-specific adjustments
# ---------------------------------------------------------------
UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Darwin)

ifeq (, $(shell which g++-14))
$(error g++-14 not found. Run: brew install gcc@14 freeimage)
endif

    # Force C++14 (required for multi-statement constexpr)
    CXXFLAGS += -std=c++14

    # Optional but useful for older codebases on modern macOS
    CFLAGS   += -Wno-implicit-function-declaration
    CXXFLAGS += -Wno-implicit-function-declaration

    # Uncomment if you still hit C23 keyword problems in C files
    # CFLAGS += -std=gnu99
endif

# Linux also:
ifeq ($(UNAME_S),Linux)
    # Force C++14 (required for multi-statement constexpr)
    CXXFLAGS += -std=c++14

    # Optional but useful for older codebases on modern macOS
    CFLAGS   += -Wno-implicit-function-declaration
    CXXFLAGS += -Wno-implicit-function-declaration
endif

# Make sure the sub-makes see the flags
export CC CXX CFLAGS CXXFLAGS LDFLAGS

SUBDIRS := shared_libraries euphoric_tools osdk

all install clean:
	@for d in $(SUBDIRS); do \
		$(MAKE) -C "$$d" $(MAKECMDGOALS) || exit $$?; \
	done
