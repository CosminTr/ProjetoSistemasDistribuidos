
CC = gcc
SRC_DIR = source
INC_DIR = include
BIN_DIR = binary
OBJ_DIR = object
PROTOC_DIR=/usr/
OBJ = client_stub.o data.o entry.o inet.o network_client.o network_server.o sdmessage.pb-c.o serialization.o tree_client.o tree_skel.o tree-private.o tree.o 
TESTS = test_data.o test_entry.o test_tree.o
CFLAGS = -Wall -g -O2 -I ${PROTOC_DIR} ${INC_DIR}
LDFLAGS = ${PROTOC_DIR}lib/x86_64-linux-gnu/libprotobuf-c.a
TARGET = pack_unpack
COMPILE  = $(CC) $(CFLAGS) -MD -c -g -o $(OBJ_DIR)/$@ -c $<
LINK = $(CC) $(OBJ) $(LDFLAGS) -o $@

TREESERVER = data.o entry.o message.o network_server.o sdmessage.pb-c.o tree_skel.o tree_server.o tree.o 
TREECLIENT = data.o entry.o message.o network_client.o sdmessage.pb-c.o client_stub.o tree_client.o 
CLIENT = client_stub.o network_client.o data.o entry.o message.o sdmessage.pb-c.o

vpath %.o $(OBJ_DIR) $(TESTS)

ALL: $(TARGET)

$(TARGET):$(OBJ)
	$(LINK)

all: $(OBJ)

test_data: $(OBJ) $(TESTS)
	$(CC) $(addprefix $(OBJ_DIR)/,$(OBJ)) $(OBJ_DIR)/test_data.o -g -o $(BIN_DIR)/test_data

test_entry: $(OBJ) $(TESTS)
	$(CC) $(addprefix $(OBJ_DIR)/,$(OBJ)) $(OBJ_DIR)/test_entry.o -g -o $(BIN_DIR)/test_entry

test_tree: $(OBJ) $(TESTS)
	$(CC) $(addprefix $(OBJ_DIR)/,$(OBJ)) $(OBJ_DIR)/test_tree.o -g -o $(BIN_DIR)/test_tree

%.o: $(SRC_DIR)/%.c $($@)
	$(COMPILE)
	
%.pb-c.c:%.proto
	${PROTOC_DIR}bin/protoc-c sdmessage.proto --c_out=./

-include $(OBJS:.o=.d)

clean: 
	rm -f $(addprefix $(OBJ_DIR)/,$(OBJ))
	rm -f $(addprefix $(OBJ_DIR)/,$(TESTS))
	rm -f $(BIN_DIR)/test_data
	rm -f $(BIN_DIR)/test_entry
	rm -f $(BIN_DIR)/test_tree
	rm -f $(OBJ) *~ *.d *.o $(TARGET) *pb-c.[ch]
