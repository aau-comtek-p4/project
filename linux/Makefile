PROJECT_NAME = COMTEK4
CPP = g++

LOCAL_INC_DIR= include
GENERAL_INC_DIR= ../common/include

CPP_FLAGS = -std=c++23 -Wall -g -I$(LOCAL_INC_DIR) -I$(GENERAL_INC_DIR)

SRC_DIR = src

SERVER_SRC_DIR= $(SRC_DIR)/server
NODE_SRC_DIR= $(SRC_DIR)/node
SIMULATOR_SRC_DIR= $(SRC_DIR)/simulator

COMMON_SRC_DIR= $(SRC_DIR)/common


BUILD_DIR = build

SERVER_BUILD_DIR = $(BUILD_DIR)/server
NODE_BUILD_DIR = $(BUILD_DIR)/node
SIMULATOR_BUILD_DIR = $(BUILD_DIR)/simulator


SERVER_OBJ_DIR = $(SERVER_BUILD_DIR)/objs
NODE_OBJ_DIR = $(NODE_BUILD_DIR)/objs
SIMULATOR_OBJ_DIR = $(SIMULATOR_BUILD_DIR)/objs

SERVER_TARGET = $(SERVER_BUILD_DIR)/$(PROJECT_NAME)_SERVER
NODE_TARGET = $(NODE_BUILD_DIR)/$(PROJECT_NAME)_NODE
SIMULATOR_TARGET= $(SIMULATOR_BUILD_DIR)/$(PROJECT_NAME)_SIMULATOR

COMMON_CPP_SRCS := $(shell find $(COMMON_SRC_DIR) -name '*.cpp') 
SERVER_CPP_SRCS := $(shell find $(SERVER_SRC_DIR) -name '*.cpp') 
NODE_CPP_SRCS := $(shell find $(NODE_SRC_DIR) -name '*.cpp') 
SIMULATOR_CPP_SRCS := $(shell find $(SIMULATOR_SRC_DIR) -name '*.cpp') 


SERVER_CPP_OBJS := $(patsubst $(SERVER_SRC_DIR)/%.cpp, $(SERVER_OBJ_DIR)/%.o,$(SERVER_CPP_SRCS)) $(patsubst $(COMMON_SRC_DIR)/%.cpp, $(SERVER_OBJ_DIR)/%.o,$(COMMON_CPP_SRCS)) 
NODE_CPP_OBJS := $(patsubst $(NODE_SRC_DIR)/%.cpp, $(NODE_OBJ_DIR)/%.o,$(NODE_CPP_SRCS)) $(patsubst $(COMMON_SRC_DIR)/%.cpp, $(NODE_OBJ_DIR)/%.o,$(COMMON_CPP_SRCS))
SIMULATOR_CPP_OBJS := $(patsubst $(SIMULATOR_SRC_DIR)/%.cpp, $(SIMULATOR_OBJ_DIR)/%.o,$(SIMULATOR_CPP_SRCS)) $(patsubst $(COMMON_SRC_DIR)/%.cpp, $(SIMULATOR_OBJ_DIR)/%.o,$(COMMON_CPP_SRCS)) 

$(foreach dir,$(sort $(dir $(SERVER_CPP_OBJS))),$(shell mkdir -p $(dir)))
$(foreach dir,$(sort $(dir $(NODE_CPP_OBJS))),$(shell mkdir -p $(dir)))
$(foreach dir,$(sort $(dir $(SIMULATOR_CPP_OBJS))),$(shell mkdir -p $(dir)))

all: $(NODE_TARGET) $(SERVER_TARGET) $(SIMULATOR_TARGET)

$(SERVER_TARGET): $(SERVER_CPP_OBJS)
	@echo "Linking : $@"
	$(CPP) $(CPP_FLAGS) -o $@ $(SERVER_CPP_OBJS)

$(NODE_TARGET): $(NODE_CPP_OBJS)
	@echo "Linking : $@"
	$(CPP) $(CPP_FLAGS) -o $@ $(NODE_CPP_OBJS)

$(SIMULATOR_TARGET): $(SIMULATOR_CPP_OBJS)
	@echo "Linking : $@"
	$(CPP) $(CPP_FLAGS) -o $@ $(SIMULATOR_CPP_OBJS)


$(SERVER_OBJ_DIR)/%.o: $(SERVER_SRC_DIR)/%.cpp | $(SERVER_OBJ_DIR)
	@echo "Compiling C: $<"
	$(CPP) $(CPP_FLAGS) -c $< -o $@

$(NODE_OBJ_DIR)/%.o: $(NODE_SRC_DIR)/%.cpp | $(NODE_OBJ_DIR)
	@echo "Compiling C: $<"
	$(CPP) $(CPP_FLAGS) -c $< -o $@

$(SIMULATOR_OBJ_DIR)/%.o: $(SIMULATOR_SRC_DIR)/%.cpp | $(SIMULATOR_OBJ_DIR)
	@echo "Compiling C: $<"
	$(CPP) $(CPP_FLAGS) -c $< -o $@

$(SERVER_OBJ_DIR)/%.o: $(COMMON_SRC_DIR)/%.cpp | $(SERVER_OBJ_DIR)
	@echo "Compiling C: $<"
	$(CPP) $(CPP_FLAGS) -c $< -o $@

$(NODE_OBJ_DIR)/%.o: $(COMMON_SRC_DIR)/%.cpp | $(NODE_OBJ_DIR)
	@echo "Compiling C: $<"
	$(CPP) $(CPP_FLAGS) -c $< -o $@

$(SIMULATOR_OBJ_DIR)/%.o: $(COMMON_SRC_DIR)/%.cpp | $(SIMULATOR_OBJ_DIR)
	@echo "Compiling C: $<"
	$(CPP) $(CPP_FLAGS) -c $< -o $@

$(SERVER_OBJ_DIR):  | $(SERVER_BUILD_DIR)
	@mkdir -p $(SERVER_OBJ_DIR)

$(NODE_OBJ_DIR):  | $(NODE_BUILD_DIR)
	@mkdir -p $(NODE_OBJ_DIR)

$(SIMULATOR_OBJ_DIR):  | $(SIMULATOR_BUILD_DIR)
	@mkdir -p $(SIMULATOR_OBJ_DIR)


$(SERVER_BUILD_DIR): | $(BUILD_DIR)
	@mkdir -p $(SERVER_BUILD_DIR)

$(NODE_BUILD_DIR): | $(BUILD_DIR)
	@mkdir -p $(NODE_BUILD_DIR)

$(SIMULATOR_BUILD_DIR): | $(BUILD_DIR)
	@mkdir -p $(SIMULATOR_BUILD_DIR)

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)



clean:
	rm -rf build



