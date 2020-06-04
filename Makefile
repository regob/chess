CPPFLAGS= -pthread

SERVER_BUILD_DIR = server/bin
CLIENT_BUILD_DIR = client/bin
CLIENT_COMPILE = compile.sh

OBJ = $(SERVER_BUILD_DIR)/board.o \
      $(SERVER_BUILD_DIR)/global_message.o

.PHONY: all

all: $(SERVER_BUILD_DIR) $(OBJ)
	make -C server
	g++ $(CPPFLAGS) -o $(SERVER_BUILD_DIR)/server $(SERVER_BUILD_DIR)/*.o
	sh -c "cd $(CLIENT_BUILD_DIR); ./$(CLIENT_COMPILE)"

$(SERVER_BUILD_DIR)/%.o: %.cpp 
	g++ -c $(CPPFLAGS) -o $@ $<

$(SERVER_BUILD_DIR): 
	mkdir -p $(SERVER_BUILD_DIR)
