#-- Variables

NAME ?= mge

# Options: {32, 64}
BLD_TARGET ?= 64

#-- Directories

BIN_DIR ?= ./bin
BLD_DIR ?= ./bld
INC_DIR ?= ./include
INS_DIR ?= /usr/local
LIB_DIR ?= ./lib
SRC_DIR ?= ./src
TST_DIR ?= ./test

#-- Extensions

BIN_EXT ?=
LIB_EXT ?=.so

#-- Compiling

AR ?= ar

CC  ?= gcc
CXX ?= g++
ifeq ($(BLD_TARGET),64)
	CC  += -m64
	CXX += -m64
else
	CC  += -m32
	CXX += -m32
endif

C_FLAGS   ?=
CPP_FLAGS ?= -O2 -std=c++20

debug: CPP_FLAGS += -g

release: CPP_FLAGS += -DNDEBUG

PKG_CONFIG ?= pkg-config

#-- Macros
MKDIR = @mkdir -p $1
RMDIR = @rm -fr $1
COPY  = @cp $1 $2

#-- Include

INCLUDE := -I$(INC_DIR)

#-- Libraries
LIBS := -L$(LIB_DIR) -lmge 

$(BIN_DIR)/test-input$(BIN_EXT): LIBS += $(shell $(PKG_CONFIG) --cflags --libs glfw3)
$(BIN_DIR)/test-physics$(BIN_EXT): LIBS += $(shell $(PKG_CONFIG) --cflags --libs raylib)
$(BIN_DIR)/test-window$(BIN_EXT): LIBS += $(shell $(PKG_CONFIG) --cflags --libs vulkan glfw3)

#-- Objects

CORE_SRCS := $(wildcard $(SRC_DIR)/*.cpp)
CORE_OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(BLD_DIR)/%.o, $(CORE_SRCS))

LIB_OBJ := $(CORE_OBJS) $(PHY_OBJS)

#-- Target-Specific Variables
# Append specific flags ONLY when compiling these exact files
$(BLD_DIR)/physics.o: CPP_FLAGS += $(PHY_FLAGS)

#-- Rules

.PHONY: all debug release tests run-tests install uninstall clean

all: $(LIB_DIR)/lib$(NAME).a

#   -- Core Code (Pattern Rule)
# This one rule replaces the need to explicitly write rules for audio.o, physics.o, window.o
$(BLD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(call MKDIR,$(@D))
	$(CXX) $(CPP_FLAGS) -c $(INCLUDE) $< -o $@

#   -- Static Library
$(LIB_DIR)/lib$(NAME).a: $(LIB_OBJ)
	$(call MKDIR,$(LIB_DIR))
	$(AR) rcs $@ $(LIB_OBJ)

debug: clean $(LIB_DIR)/lib$(NAME).a

release: clean $(LIB_DIR)/lib$(NAME).a

#   -- Tests
$(BIN_DIR)/test-%$(BIN_EXT): $(TST_DIR)/test-%.cpp $(LIB_DIR)/lib$(NAME).a
	$(call MKDIR,$(@D))
	$(CXX) $(CPP_FLAGS) $(INCLUDE) $< $(LIBS) -o $@

tests: $(BIN_DIR)/test-input$(BIN_EXT) $(BIN_DIR)/test-physics$(BIN_EXT) $(BIN_DIR)/test-window$(BIN_EXT)

test-all: tests
	$(BIN_DIR)/test-input$(BIN_EXT)
	$(BIN_DIR)/test-physics$(BIN_EXT)
	$(BIN_DIR)/test-window$(BIN_EXT)

test-%: $(BIN_DIR)/test-%$(BIN_EXT)
	$<

test-window: $(BIN_DIR)/test-window$(BIN_EXT)
	$(BIN_DIR)/test-window$(BIN_EXT)

#-- Installation

install: $(LIB_DIR)/lib$(NAME).a
	mkdir -p $(INS_DIR)/lib
	mkdir -p $(INS_DIR)/include/$(NAME)
	cp $(LIB_DIR)/lib$(NAME).a $(INS_DIR)/lib/
	cp -r include/$(NAME)/* $(INS_DIR)/include/$(NAME)/

uninstall:
	rm $(INS_DIR)/lib/lib$(NAME).a
	rm -r $(INS_DIR)/include/$(NAME)
	
#   -- Clean
clean:
	$(call RMDIR,$(BIN_DIR))
	$(call RMDIR,$(BLD_DIR))
	$(call RMDIR,$(LIB_DIR))
	$(call RMDIR,./result)
