CC = gcc
OBJ_DIR = src
INC_DIR = include
BIN_DIR = binary
OBJETOS = data.o entry.o serialization.o tree.o
CFLAGS = -Wall -I $(INC_DIR)

vpath %.o $(OBJ_DIR)

grupo19: $(OBJETOS)
	$(CC) $(addprefix $(OBJ_DIR)/,$(OBJETOS)) -o $(BIN_DIR)/grupo19 -lrt -lpthread

%.o: src/%.c $($@)
	$(CC) $(CFLAGS)  -o $(OBJ_DIR)/$@ -c $<

clean: 
	rm -f $(addprefix $(OBJ_DIR)/,$(OBJETOS))
	rm -f binary/grupo19
