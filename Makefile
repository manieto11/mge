#-- Variables

NAME := mge

# Options: {2D, 3D}
DIMENSIONS := 3D

#-- Directories

BIN_DIR := ./bin
BLD_DIR := ./bld
EXT_DIR := ./external
INC_DIR := ./include
LIB_DIR := ./lib
SRC_DIR := ./src
TST_DIR := ./test

BOX2D_DIR := $(EXT_DIR)/box2d
BOX3D_DIR := $(EXT_DIR)/box3d
FT_DIR    := $(EXT_DIR)/freetype
GLM_DIR   := $(EXT_DIR)/glm
JSON_DIR  := $(EXT_DIR)/json
KTX_DIR   := $(EXT_DIR)/KTX-Software
MA_DIR    := $(EXT_DIR)/miniaudio
SDL_DIR   := $(EXT_DIR)/SDL
TOL_DIR   := $(EXT_DIR)/tinyobjloader
VK_DIR    := $(EXT_DIR)/Vulkan-Headers
VMA_DIR   := $(EXT_DIR)/VulkanMemoryAllocator

SDL_LIB_DIR := "C:\SDL3\3.4.14\lib\x64"
VK_LIB_DIR := "C:\VulkanSDK\1.4.341.1\Lib"

#-- Compilers

CC  := gcc
CXX := g++

C_FLAGS   :=
CPP_FLAGS := -O2 -std=c++20

ifdef DEBUG
    C_FLAGS   += -g -DDEBUG
    CPP_FLAGS += -g -DDEBUG
endif

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
WIN_INC := -I$(SDL_DIR)/include -I$(VK_DIR)/include -I$(VMA_DIR)/include

#-- Libraries

LIBS := -L$(LIB_DIR) -lmge

ifeq ($(OS),Windows_NT)
    LIBS += -L$(SDL_LIB_DIR) -lSDL3 -L$(VK_LIB_DIR) -lvulkan-1
else
    LIBS += -lSDL3 -lvulkan
endif

#-- Objects

ifeq ($(DIMENSIONS),2D)
    PHY_SRCS  := $(wildcard $(BOX2D_DIR)/src/*.c)
    PHY_OBJS  := $(patsubst $(BOX2D_DIR)/src/%.c, $(BLD_DIR)/box2d/%.o, $(PHY_SRCS))
    PHY_FLAGS := -DDIM2D
    PHY_INC   := -I$(BOX2D_DIR)/include
else
    PHY_SRCS  := $(wildcard $(BOX3D_DIR)/src/*.c)
    PHY_OBJS  := $(patsubst $(BOX3D_DIR)/src/%.c, $(BLD_DIR)/box3d/%.o, $(PHY_SRCS))
    PHY_FLAGS :=
    PHY_INC   := -I$(BOX3D_DIR)/include
endif

# Automatically grab all .cpp files in src/ so you don't have to add them manually
CORE_SRCS := $(wildcard $(SRC_DIR)/*.cpp)
CORE_OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(BLD_DIR)/%.o, $(CORE_SRCS))

LIB_OBJ := $(CORE_OBJS) $(PHY_OBJS)

#-- Target-Specific Variables
# Append specific flags ONLY when compiling these exact files
$(BLD_DIR)/physics.o: CPP_FLAGS += $(PHY_INC) $(PHY_FLAGS)
$(BLD_DIR)/window.o:  CPP_FLAGS += $(WIN_INC)

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
ifeq ($(OS),Windows_NT)
	$(call COPY,$(SDL_LIB_DIR)/SDL3.dll,$(BIN_DIR))
endif
	$(BIN_DIR)/test.exe

#   -- Clean
clean:
	$(call RMDIR,$(BIN_DIR))
	$(call RMDIR,$(BLD_DIR))
	$(call RMDIR,$(LIB_DIR))