# Notes:
# MacOS:
# 	This project intentionally uses GNU libstdc++ on macOS.
#

# Quiet
#Q ?= @

RANLIB ?= ranlib
AR ?= ar
CC ?= cc
CXX ?= c++

HOSTOS := $(shell uname -s)

ifeq ($(PLATFORM),)
PLATFORM := $(HOSTOS)
endif

# Debug/release selection
ifeq ($(RELEASE),)
DEBUG = 1
CPPFLAGS += -D_DEBUG
else
CPPFLAGS += -DNDEBUG
endif

MATH_LIBS ?= -lm

# Windows cross build
ifeq ($(PLATFORM),win32)
EXE := .exe
.SUFFIXES: .exe
CROSS_COMPILE ?= i586-mingw32msvc-
CC := $(CROSS_COMPILE)$(CC)
CXX := $(CROSS_COMPILE)$(CXX)
AR := $(CROSS_COMPILE)$(AR)
RANLIB := $(CROSS_COMPILE)$(RANLIB)
WINDRES := $(CROSS_COMPILE)windres

CPPFLAGS += -DWIN32

%.exe: %.o
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@
%.exe: %.c
	$(LINK.c) $^ $(LOADLIBES) $(LDLIBS) -o $@
endif

# Haiku
ifeq ($(PLATFORM),Haiku)
CURSES_LIB := -lncurses
MATH_LIBS :=
endif

# Non-Windows, i.e. POSIX platforms
ifneq ($(PLATFORM),win32)
CURSES_LIB ?= -lcurses

ifeq ($(PLATFORM),Darwin)
# IMPORTANT:
# The project is not currently compatible with Apple clang/libc++.
# Mixing clang/libc++ with these objects causes linker failures.
CC := gcc-14
CXX := g++-14
STDCXX_LIB ?= -lstdc++
CXXSTD ?= -std=c++14
endif

ifeq ($(PLATFORM),Linux)
CC := gcc-14
CXX := g++-14
# Linux 
STDCXX_LIB ?= -lstdc++
CXXSTD ?= -std=c++11
endif

# Linker additions
COMMON_EXTRA_LDFLAGS += $(CURSES_LIB) $(STDCXX_LIB)

# Platforms share CXXFLAGS, but avoid multiple incompatible -std options.
CXXFLAGS := $(filter-out \
	-std=c++11 \
	-std=c++14 \
	-std=c++17 \
	-std=c++20 \
	-std=c++23, \
	$(CXXFLAGS))

CXXFLAGS += $(CXXSTD)

# POSIX compatibility
CPPFLAGS += -D__cdecl= -DPOSIX
CFLAGS += -Wall
endif # Non-Windows platforms

# OSDK installation support
ifneq ($(OSDK),)
.PHONY: install
install:
	$(Q)install -d $(OSDK)/bin
	$(Q)for B in $(BINS) $(EXECUTABLE); do \
		install $$B $(OSDK)/bin/; \
	done

	$(Q)for B in $(BINS) $(EXECUTABLE); do \
		b="$$(echo "$$B" | tr A-Z a-z)"; \
		if [ "$$B" != "$$b" ]; then \
			ln -sf "$$B" "$(OSDK)/bin/$$b"; \
		fi; \
	done
endif
