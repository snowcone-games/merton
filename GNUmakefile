UNAME_S = $(shell uname -s)
ARCH = $(shell uname -m)
NAME = merton

OBJS = \
	src/csync.o \
	src/rcore.o \
	src/loader.o \
	src/main.o

INCLUDES = \
	-I. \
	-Isrc \
	-I../libmatoya/src

FLAGS = \
	-Wall \
	-Wextra \
	-Wshadow \
	-Wno-unused-parameter \
	-std=c99 \
	-fPIC

DEFS = \
	-D_POSIX_C_SOURCE=200112L

ifdef DEBUG
FLAGS := $(FLAGS) -O0 -g3
else
FLAGS := $(FLAGS) -O3 -g0 -flto -fvisibility=hidden
LD_FLAGS = -flto
endif

############
### WASM ###
############

# github.com/WebAssembly/wasi-sdk/releases -> ~/wasi-sdk

ifdef WASM

WASI_SDK = $(HOME)/wasi-sdk

CC = $(WASI_SDK)/bin/clang

TARGET = web
ARCH := wasm32

LD_FLAGS := $(LD_FLAGS) \
	-Wl,--allow-undefined \
	-Wl,--export-table \
	-Wl,--import-memory,--export-memory,--max-memory=1073741824 \
	-Wl,-z,stack-size=$$((8 * 1024 * 1024))

FLAGS := $(FLAGS) \
	--sysroot=$(WASI_SDK)/share/wasi-sysroot \
	--target=wasm32-wasi-threads \
	-pthread

else
#############
### LINUX ###
#############
ifeq ($(UNAME_S), Linux)

TARGET = linux

LIBS = \
	-lgcc_s \
	-lc \
	-lm

LD_FLAGS := $(LD_FLAGS) \
	-nodefaultlibs

endif

#############
### APPLE ###
#############
ifeq ($(UNAME_S), Darwin)

ifndef TARGET
TARGET = macosx
endif

ifndef ARCH
ARCH = x86_64
endif

ifeq ($(TARGET), macosx)
MIN_VER = 10.15
else
MIN_VER = 13.0
FLAGS := $(FLAGS) -fembed-bitcode
endif

LIBS = \
	-framework AppKit \
	-framework QuartzCore \
	-framework Carbon \
	-framework GameController \
	-framework Metal \
	-framework IOKit \
	-framework AudioToolbox \
	-framework WebKit

FLAGS := $(FLAGS) \
	-m$(TARGET)-version-min=$(MIN_VER) \
	-isysroot $(shell xcrun --sdk $(TARGET) --show-sdk-path) \
	-arch $(ARCH)

LD_FLAGS := $(LD_FLAGS) \
	-arch $(ARCH)

endif
endif

STATIC_LIBS = \
	../libmatoya/bin/$(TARGET)/$(ARCH)/libmatoya.a

CFLAGS = $(INCLUDES) $(FLAGS) $(DEFS)

all: clean clear zip
	make objs -j4

zip:
	compress/$(TARGET)/$(ARCH)/mcompress ui-zip.h UI_ZIP src/ui

objs: $(OBJS)
	$(CC) -o $(NAME) $(OBJS) $(STATIC_LIBS) $(LIBS) $(LD_FLAGS)

###############
### ANDROID ###
###############

# developer.android.com/ndk/downloads -> ~/android-ndk

ifndef ANDROID_NDK_ROOT
ANDROID_NDK_ROOT = $(HOME)/android-ndk
endif

ifndef ABI
ABI = all
endif

android: clean clear $(SHADERS)
	@$(ANDROID_NDK_ROOT)/ndk-build -j4 \
		APP_BUILD_SCRIPT=Android.mk \
		APP_PLATFORM=android-28 \
		APP_ABI=$(ABI) \
		NDK_PROJECT_PATH=. \
		NDK_OUT=android/build \
		NDK_LIBS_OUT=android/app/libs \
		--no-print-directory

clean:
	@rm -rf android/build
	@rm -rf android/app/libs
	@rm -rf $(NAME)
	@rm -rf $(OBJS)
	@rm -rf ui-zip.h

clear:
	@clear
