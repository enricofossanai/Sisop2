CC=g++
CLIENT_DIR=Client
SERVER_DIR=Server
LIB_DIR=lib
BIN_DIR=bin

all: client server

client:
	$(CC) $(CLIENT_DIR)/myClient.cpp $(CLIENT_DIR)/communication.cpp $(CLIENT_DIR)/aplication.cpp -o $(BIN_DIR)/myClient -lpthread
server:
	$(CC) $(SERVER_DIR)/myServer.cpp  $(SERVER_DIR)/communication.cpp $(SERVER_DIR)/fileManager.cpp $(SERVER_DIR)/sync.cpp -o $(BIN_DIR)/myServer -lpthread

clean:
	rm -rf *.a $(BIN_DIR)/*.o
