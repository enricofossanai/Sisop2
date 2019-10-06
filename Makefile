CC=g++
CLIENT_DIR=Client
SERVER_DIR=Server
LIB_DIR=lib
BIN_DIR=bin

all: client server

client:
	$(CC) $(CLIENT_DIR)/myClient.cpp $(CLIENT_DIR)/commClient.cpp $(CLIENT_DIR)/aplication.cpp -o $(BIN_DIR)/myClient -lpthread -fpermissive
server:
	$(CC) $(SERVER_DIR)/myServer.cpp  $(SERVER_DIR)/commServer.cpp $(SERVER_DIR)/fileManager.cpp -o $(BIN_DIR)/myServer -lpthread -fpermissive

clean:
	rm -rf *.a $(BIN_DIR)/*.o
