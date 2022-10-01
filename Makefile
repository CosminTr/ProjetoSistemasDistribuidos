CC = gcc
SRC_DIR = source
INC_DIR = include
BIN_DIR = binary
OBJETOS = data.o entry.o serialization.o tree.o
CFLAGS = -Wall -I $(INC_DIR)

vpath %.o $(SRC_DIR)

grupo19: $(OBJETOS)
	$(CC) $(addprefix $(SRC_DIR)/,$(OBJETOS)) -o $(BIN_DIR)/grupo19 -lrt -lpthread

%.o: src/%.c $($@)
	$(CC) $(CFLAGS)  -o $(SRC_DIR)/$@ -c $<

clean: 
	rm -f $(addprefix $(SRC_DIR)/,$(OBJETOS))
	rm -f binary/grupo19
