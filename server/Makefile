CPPFLAGS = -pthread -c

BUILD_DIR = bin
OBJ = $(BUILD_DIR)/auth.o \
	  $(BUILD_DIR)/game.o \
	  $(BUILD_DIR)/main.o \
	  $(BUILD_DIR)/message.o \
	  $(BUILD_DIR)/server.o \
	  $(BUILD_DIR)/user.o

.PHONY: all

all: $(OBJ)

$(BUILD_DIR)/%.o: %.cpp
	g++ $(CPPFLAGS) -o $@ $<
