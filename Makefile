# Makefile for YAIV for mostly POSIX.
# ----------------------------------------------------------------------
# Written by Raph.K.
#

GCC = gcc
GPP = g++
AR  = ar
WRC = windres
FCG = fltk-config --use-images

# FLTK configs 
FLTKCFG_CXX := $(shell ${FCG} --cxxflags)
FLTKCFG_LFG := $(shell ${FCG} --ldflags)

# Base PATH
BASE_PATH = .
FIMG_PATH = ../fl_imgtk/lib
SRC_PATH  = $(BASE_PATH)/src

# TARGET settings
TARGET_PKG = yaiv
TARGET_DIR = ./bin
TARGET_OBJ = ./obj

# DEFINITIONS
DEFS  = -D_POSIX_THREADS -DSUPPORT_DRAGDROP -DMININI_ANSI
DEFS += -DUSE_OMP
DEFS += -DINI_ANSIONLY

# Compiler optiops 
COPTS += -ffast-math -fexceptions
LOPTS  =
LTWKS  = 
CXXOPTS =

# Are you debugging ?
DEBUG =

ifeq ($(DEBUG),1)
    COPTS += -g -DDEBUG
else
    COPTS += -O2
endif

# Automatic detecting platform -
ARCH_S = $(shell uname -s)
ARCH_M = $(shell uname -m)
ARCH_R = $(shell uname -r | cut -d . -f1)

# Architecture reconfiguration ...
ifeq ($(ARCH_S),Darwin)
    GCC = llvm-gcc
    CPP = llvm-g++
    CXXOPTS  += -std=c++11
# -- google libwebp don't support universal binary --
#    ifeq ($(shell test $(ARCH_R) -gt 19; echo $$?),0)
#        COPTS += -arch x86_64 -arch arm64
#    endif
    LOPTS += -lwebp -lwebpdemux -lwebpmux
    ifeq ($(DEBUG),0)
	    LTWKS += -s
    endif
else ifeq ($(ARCH_S),Linux)
    CXXOPTS += -std=c++11
    COPTS += -fopenmp
    LOPTS += -lwebp -lwebpdemux -lwebpmux
else
    ARCH_SS = $(shell echo $(ARCH_S) | cut -d _ -f1)
    ifeq ($(ARCH_SS),MINGW64)
        COPTS += -mms-bitfields -mwindows
        COPTS += -fopenmp
        LOPTS += -lwebp -lwebpdecoder -lwebpdemux -lwebpmux
    endif
    ifeq ($(DEBUG),0)
	    LTWKS += -s
    endif
endif

# CC FLAGS
CFLAGS  = -I$(SRC_PATH)
CFLAGS += -I$(SRC_PATH)/toolbar
CFLAGS += -I$(FIMG_PATH)
CFLAGS += $(DEFS)
CFLAGS += $(COPTS)

# CXX FLAGS
CXXFLAGS  = -I$(SRC_PATH)
CXXFLAGS += -I$(SRC_PATH)/toolbar
CXXFLAGS += -I$(FIMG_PATH)
CXXFLAGS += $(FLTKCFG_CXX)
CXXFLAGS += $(DEFS)
CXXFLAGS += $(COPTS)
CXXFLAGS += $(CXXOPTS)

# Windows Resource Flags
WFLGAS  = $(CFLAGS)

# LINK FLAG
LFLAGS  =
LFLAGS += -L$(FIMG_PATH)
LFLAGS += -lfl_imgtk
LFLAGS += ${FLTKCFG_LFG}
LFLAGS += -lpthread
LFLAGS += -lsqlite3
LFLAGS += $(LOPTS)
LFLAGS += $(LTWKS)

# Sources
SRCS_CPP = $(wildcard $(SRC_PATH)/*.cpp)
SRCS_C = $(wildcard $(SRC_PATH)/*.c)
SRCS_TOOLBAR = $(wildcard $(SRC_PATH)/toolbar/*.cxx)

# Make object targets from SRCS.
OBJS_CPP = $(SRCS_CPP:$(SRC_PATH)/%.cpp=$(TARGET_OBJ)/%.cxo)
OBJS_C = $(SRCS_C:$(SRC_PATH)/%.c=$(TARGET_OBJ)/%.co)
OBJS_TOOLBAR = $(SRCS_TOOLBAR:$(SRC_PATH)/toolbar/%.cxx=$(TARGET_OBJ)/toolbar/%.cxo)

.PHONY: prepare clean avx

all: prepare continue

continue: $(TARGET_DIR)/$(TARGET_PKG)

prepare:
	@mkdir -p $(TARGET_DIR)
	@mkdir -p $(TARGET_OBJ)
	@mkdir -p $(TARGET_OBJ)/toolbar

cleanobj:
	@echo "Cleaning objects ..."
	@rm -rf $(TARGET_INC)/*.h
	@rm -rf $(TARGET_OBJ)/*.cxo
	@rm -rf $(TARGET_OBJ)/*.co
	@rm -rf $(TARGET_OBJ)/toolbar/*.cxo
	@rm -rf $(TARGET_WROBJ)/*.res

clean: cleanobj
	@echo "Cleaning built targets ..."
	@rm -rf $(TARGET_DIR)/$(TARGET_PKG).*

packaging:
	@cp -rf pkgscripts/license.rtf ${TARGET_DIR}
	@cp -rf pkgscripts/readme.rtf ${TARGET_DIR}

$(OBJS_C): $(TARGET_OBJ)/%.co: $(SRC_PATH)/%.c
	@echo "Building $< ... "
	@$(GCC) $(CFLAGS) -c $< -o $@

$(OBJS_CPP): $(TARGET_OBJ)/%.cxo: $(SRC_PATH)/%.cpp
	@echo "Building $< ... "
	@$(GPP) $(CXXFLAGS) -c $< -o $@

$(OBJS_TOOLBAR): $(TARGET_OBJ)/toolbar/%.cxo: $(SRC_PATH)/toolbar/%.cxx
	@echo "Building $< ... "
	@$(GPP) $(CXXFLAGS) -c $< -o $@

$(TARGET_DIR)/$(TARGET_PKG): $(OBJS_CPP) $(OBJS_C) $(OBJS_TOOLBAR)
	@echo "Generating $@ ..."
	@$(GPP) $^ $(CFLAGS) $(LFLAGS) -o $@
	@echo "done."
