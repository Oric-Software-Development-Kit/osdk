#NO_LIB=1

SRC=main.c world.c asm_data.s asm_helpers.s
BIN=debug_type_zoo
OBJ=$(SRC:.c=.o)
OBJ+=$(SRC:.s=.o)
include ../../rules.mk

all: $(DATA) $(SRC) $(BIN)
