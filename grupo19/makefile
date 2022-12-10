CC = gcc
SRC_DIR = source
INC_DIR = include
BIN_DIR = binary
OBJ_DIR = object
LIB_DIR = lib
CFLAGS = -g -Wall -I 
COMPILE  = $(CC) $(CFLAGS) $(INC_DIR) -o $(OBJ_DIR)/$@ -c $<


client-lib = client_stub.o network_client.o data.o entry.o message.o sdmessage.pb-c.o
tree-client = data.o entry.o message.o network_client.o sdmessage.pb-c.o client_stub.o tree_client.o 
tree-server = data.o entry.o message.o network_server.o sdmessage.pb-c.o tree_skel.o tree_server.o tree.o 

OBJ = client-lib.o tree-client.o tree-server.o 


all: client-lib.o tree-client tree-server  

%.o: $(SRC_DIR)/%.c 
	$(COMPILE)
	
#%.pb-c.c:%.proto
#	/usr/bin/protoc-c sdmessage.proto --c_out=./


client-lib.o: $(client-lib)
	ld -r $(addprefix $(OBJ_DIR)/,$^) -o $(LIB_DIR)/$@

tree-client: $(tree-client)
	$(CC) -D THREADED $(addprefix $(OBJ_DIR)/,$^) -I/usr/local/include -L/usr/local/lib -lprotobuf-c -o $(BIN_DIR)/$@ -lzookeeper_mt

tree-server: $(tree-server)
	$(CC) -D THREADED $(addprefix $(OBJ_DIR)/,$^) -I/usr/local/include -L/usr/local/lib -pthread -lprotobuf-c -o $(BIN_DIR)/$@ -lzookeeper_mt



clean: 
	rm -f $(addprefix $(OBJ_DIR)/,$(OBJ))
	rm -f $(BIN_DIR)/*
	rm -f $(LIB_DIR)/*
	
