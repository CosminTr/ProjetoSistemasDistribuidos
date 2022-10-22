CC = gcc
PROTOC_DIR=/usr/
SRC_DIR = source
INC_DIR = include
BIN_DIR = binary
OBJ_DIR = object
OBJ = sdmessage.pb-c.o data.o entry.o tree.o serialization.o 
TESTS = test_data.o test_entry.o test_tree.o
CFLAGS = -Wall -g -O2 -I${PROTOC_DIR}include/
LDFLAGS = ${PROTOC_DIR}lib/x86_64-linux-gnu/libprotobuf-c.a
TARGET = pack_unpack
COMPILE  = $(CC) $(CFLAGS)  -MD -c -g -o $(OBJ_DIR)/$@ -c $<
LINK = $(CC) $(OBJ) $(LDFLAGS) -o $@

vpath %.o $(OBJ_DIR) $(TESTS)

ALL:$(TARGET)

$(TARGET):$(OBJ)
	$(LINK)

all : test_data test_entry test_tree

test_data: $(OBJ) $(TESTS)
	$(CC) $(addprefix $(OBJ_DIR)/,$(OBJ)) $(OBJ_DIR)/test_data.o -g -o $(BIN_DIR)/test_data

test_entry: $(OBJ) $(TESTS)
	$(CC) $(addprefix $(OBJ_DIR)/,$(OBJ)) $(OBJ_DIR)/test_entry.o -g -o $(BIN_DIR)/test_entry

test_tree: $(OBJ) $(TESTS)
	$(CC) $(addprefix $(OBJ_DIR)/,$(OBJ)) $(OBJ_DIR)/test_tree.o -g -o $(BIN_DIR)/test_tree

%.o: source/%.c $($@)
	$(COMPILE)
	
%.pb-c.c:%.proto
	${PROTOC_DIR}bin/protoc-c sdmessage.proto --c_out=./

clean: 
	rm -f $(addprefix $(OBJ_DIR)/,$(OBJ))
	rm -f $(addprefix $(OBJ_DIR)/,$(TESTS))
	rm -f $(BIN_DIR)/test_data
	rm -f $(BIN_DIR)/test_entry
	rm -f $(BIN_DIR)/test_tree
	rm -f $(OBJ) *~ *.d *.o $(TARGET) *pb-c.[ch]
