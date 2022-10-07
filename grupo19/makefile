CC = gcc
SRC_DIR = source
INC_DIR = include
BIN_DIR = binary
OBJ_DIR = object
OBJ = data.o entry.o tree.o serialization.o 
TESTS = test_data.o test_entry.o test_tree.o
CFLAGS = -I $(INC_DIR)

vpath %.o $(OBJ_DIR) $(TESTS)

all : test_data test_entry test_tree

test_data: $(OBJ) $(TESTS)
	$(CC) $(addprefix $(OBJ_DIR)/,$(OBJ)) $(OBJ_DIR)/test_data.o -g -o $(BIN_DIR)/test_data

test_entry: $(OBJ) $(TESTS)
	$(CC) $(addprefix $(OBJ_DIR)/,$(OBJ)) $(OBJ_DIR)/test_entry.o -g -o $(BIN_DIR)/test_entry

test_tree: $(OBJ) $(TESTS)
	$(CC) $(addprefix $(OBJ_DIR)/,$(OBJ)) $(OBJ_DIR)/test_tree.o -g -o $(BIN_DIR)/test_tree

%.o: source/%.c $($@)
	$(CC) $(CFLAGS) -g -o $(OBJ_DIR)/$@ -c $<

clean: 
	rm -f $(addprefix $(OBJ_DIR)/,$(OBJ))
	rm -f $(addprefix $(OBJ_DIR)/,$(TESTS))
	rm -f $(BIN_DIR)/test_data
	rm -f $(BIN_DIR)/test_entry
	rm -f $(BIN_DIR)/test_tree