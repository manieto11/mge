#-- Variables

NAME := mge

# Options: {32, 64}
BLD_TARGET ?= 64

#-- Directories

BIN_DIR := ./bin
BLD_DIR := ./bld
INC_DIR := ./include
LIB_DIR := ./lib
SRC_DIR := ./src
TST_DIR := ./test

#-- Compiling

CC  := gcc
CXX := g++
ifeq ($(BLD_TARGET),64)
	CC  += -m64
	CXX += -m64
else
	CC  += -m32
	CXX += -m32
endif

C_FLAGS   :=
CPP_FLAGS := -O2 -std=c++20

debug: CPP_FLAGS += -g

release: CPP_FLAGS += -DNDEBUG

PKG_CONFIG := pkg-config

#-- Macros
MKDIR = @mkdir -p $1
RMDIR = @rm -fr $1
COPY  = @cp $1 $2

#-- Include

INCLUDE := -I$(INC_DIR)

#-- Libraries
LIBS := -L$(LIB_DIR) -lmge 

$(BIN_DIR)/test-window.exe: LIBS += $(shell $(PKG_CONFIG) --cflags --libs vulkan glfw3)
$(BIN_DIR)/test-physics.exe: LIBS += $(shell $(PKG_CONFIG) --cflags --libs raylib)

#-- Objects

CORE_SRCS := $(wildcard $(SRC_DIR)/*.cpp)
CORE_OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(BLD_DIR)/%.o, $(CORE_SRCS))

LIB_OBJ := $(CORE_OBJS) $(PHY_OBJS)

#-- Target-Specific Variables
# Append specific flags ONLY when compiling these exact files
$(BLD_DIR)/physics.o: CPP_FLAGS += $(PHY_FLAGS)

#-- Rules

.PHONY: all debug release tests run-tests clean

all: $(LIB_DIR)/libmge.a

#   -- Core Code (Pattern Rule)
# This one rule replaces the need to explicitly write rules for audio.o, physics.o, window.o
$(BLD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(call MKDIR,$(@D))
	$(CXX) $(CPP_FLAGS) -c $(INCLUDE) $< -o $@

#   -- External C Libraries (Pattern Rules)
$(BLD_DIR)/box2d/%.o: $(BOX2D_DIR)/src/%.c
	$(call MKDIR,$(@D))
	$(CC) $(C_FLAGS) -I$(BOX2D_DIR)/include -c $< -o $@

$(BLD_DIR)/box3d/%.o: $(BOX3D_DIR)/src/%.c
	$(call MKDIR,$(@D))
	$(CC) $(C_FLAGS) -I$(BOX3D_DIR)/include -c $< -o $@

#   -- Static Library
$(LIB_DIR)/libmge.a: $(LIB_OBJ)
	$(call MKDIR,$(LIB_DIR))
	ar rcs $@ $(LIB_OBJ)

debug: clean $(LIB_DIR)/libmge.a

release: clean $(LIB_DIR)/libmge.a

#   -- Tests
$(BIN_DIR)/test-%.exe: $(TST_DIR)/test-%.cpp $(LIB_DIR)/libmge.a
	$(call MKDIR,$(@D))
	$(CXX) $(CPP_FLAGS) $(INCLUDE) $< $(LIBS) -o $@

tests: $(BIN_DIR)/test-window.exe $(BIN_DIR)/test-physics.exe

test-physics: $(BIN_DIR)/test-physics.exe
	$(BIN_DIR)/test-physics.exe

test-window: $(BIN_DIR)/test-window.exe
	$(BIN_DIR)/test-window.exe
	
#   -- Clean
clean:
	$(call RMDIR,$(BIN_DIR))
	$(call RMDIR,$(BLD_DIR))
	$(call RMDIR,$(LIB_DIR))
