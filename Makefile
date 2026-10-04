#-- Variables

NAME := mge

# Options: {2D, 3D}
DIMENSIONS := 3D

#-- Directories

BIN_DIR := ./bin
BLD_DIR := ./bld
INC_DIR := ./include
LIB_DIR := ./lib
SRC_DIR := ./src
TST_DIR := ./test

#-- Compilers

CC  := gcc
CXX := g++

C_FLAGS   :=
CPP_FLAGS := -O2 -std=c++20

ifeq ($(DEBUG),true)
    CPP_FLAGS += -g
else
	CPP_FLAGS += -DNDEBUG
endif

PKG_CONFIG := pkg-config

#-- OS-Specific Macros (DRY)
# We define these once to avoid repeating ifeq blocks in every single target
ifeq ($(OS),Windows_NT)
    MKDIR = @if not exist "$(subst /,\,$1)" mkdir "$(subst /,\,$1)"
    RMDIR = @if exist "$(subst /,\,$1)" rmdir /S /Q "$(subst /,\,$1)"
	COPY  = @copy /Y "$(subst /,\,$1)" "$(subst /,\,$2)" > nul
else
    MKDIR = @mkdir -p $1
    RMDIR = @rm -fr $1
	COPY  = cp $1 $2
endif

#-- Include

INCLUDE := -I$(INC_DIR)

#-- Libraries

LIBS := -L$(LIB_DIR) -lmge $(shell $(PKG_CONFIG) --cflags --libs vulkan glfw3)

#-- Objects

ifeq ($(DIMENSIONS),2D)
    PHY_SRCS  := $(wildcard $(BOX2D_DIR)/src/*.c)
    PHY_OBJS  := $(patsubst $(BOX2D_DIR)/src/%.c, $(BLD_DIR)/box2d/%.o, $(PHY_SRCS))
    PHY_FLAGS := -DDIM2D
else
    PHY_SRCS  := $(wildcard $(BOX3D_DIR)/src/*.c)
    PHY_OBJS  := $(patsubst $(BOX3D_DIR)/src/%.c, $(BLD_DIR)/box3d/%.o, $(PHY_SRCS))
    PHY_FLAGS := -DDIM3D
endif

# Automatically grab all .cpp files in src/ so you don't have to add them manually
CORE_SRCS := $(wildcard $(SRC_DIR)/*.cpp)
CORE_OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(BLD_DIR)/%.o, $(CORE_SRCS))

LIB_OBJ := $(CORE_OBJS) $(PHY_OBJS)

#-- Target-Specific Variables
# Append specific flags ONLY when compiling these exact files
$(BLD_DIR)/physics.o: CPP_FLAGS += $(PHY_FLAGS)

#-- Rules

.PHONY: all test clean

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

#   -- Executable
$(BIN_DIR)/test.exe: $(LIB_DIR)/libmge.a $(TST_DIR)/main.cpp
	$(call MKDIR,$(@D))
	$(CXX) $(CPP_FLAGS) $(INCLUDE) $(TST_DIR)/main.cpp $(LIBS) -o $@

test: $(BIN_DIR)/test.exe
	$(BIN_DIR)/test.exe

#   -- Clean
clean:
	$(call RMDIR,$(BIN_DIR))
	$(call RMDIR,$(BLD_DIR))
	$(call RMDIR,$(LIB_DIR))