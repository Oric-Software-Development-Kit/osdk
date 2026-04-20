include ../../../rules.mk

CPPFLAGS += -I ../common/includes

FI_PATH := ../../../shared_libraries/freeimage/v3.12.0
NS_PATH := ../../../shared_libraries/nanosvg
CPPFLAGS += -DFREEIMAGE_LIB
ifeq ($(PLATFORM),win32)
CPPFLAGS += -I libs
LDFLAGS += -L libs -lFreeImage
else
CPPFLAGS += -I $(FI_PATH)/ -I$(FI_PATH)/Source/
LDFLAGS += -L $(FI_PATH)/ -lfreeimage $(MATH_LIBS)
endif

CPPFLAGS += -I $(NS_PATH)/src

LDFLAGS += -lstdc++
LDFLAGS += -L ../common -lcommon $(COMMON_EXTRA_LDFLAGS)

SOURCES := $(notdir $(wildcard sources/*.cpp))
# filter out some test sources...
SOURCES := $(filter-out %video.cpp,$(SOURCES))
VPATH = sources
CPPFLAGS += -I sources -I includes
OBJS := $(subst .cpp,.o,$(SOURCES))
#OBJS := $(SOURCES:.cpp,.o)

EXECUTABLE=pictconv$(EXE)

all: $(EXECUTABLE)

pictconv$(EXE): $(OBJS)
	$(LINK.c) -o $@ $(OBJS) $(LDFLAGS)

clean:
	rm -f pictconv$(EXE) $(OBJS)
