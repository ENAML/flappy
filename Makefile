# See: https://gist.github.com/maxtruxa/4b3929e118914ccef057f8a05c614b0f
# Also somewhat related:
### - http://nuclear.mutantstargoat.com/articles/make/
### - https://www.gnu.org/software/make/manual/html_node/Automatic-Variables.html
### - http://make.mad-scientist.net/papers/advanced-auto-dependency-generation/
### - https://gcc.gnu.org/onlinedocs/gcc/Preprocessor-Options.html


# define paths
# RAYLIB_PATH = ~/Projects/tests/raylib
RAYLIB_PATH = ./extern/raylib

# set web compilers
# (see: https://gist.github.com/rolandoam/5199237 for example)
EMSDK_PATH = ~/Projects/tests/emsdk
EMSCRIPTEN_VERSION = 1.37.39
EMSCRIPTEN_PATH = $(EMSDK_PATH)/emscripten/$(EMSCRIPTEN_VERSION)


# define compile target platform
# - PLATFORM_DESKTOP
# - PLATFORM_WEB
PLATFORM ?= PLATFORM_DESKTOP


# output binary
BIN := main
BIN_DIR := build

# source files
# SRCS := \
#     src/main.cpp \
# 	  src/gui.c
SRCS := $(wildcard src/*.cpp)
SRCS += $(wildcard src/*.c)
SRCS += $(wildcard src/util/*.cpp)
SRCS := $(filter-out src/tests.c, $(SRCS)) #ignore `tests.c`

# include header paths
# - -I.
# - -I./src 
# - -I./extern 
# - -L$(RAYLIB_PATH)/release/include
INCLUDES := -I.
INCLUDES += -I./src
INCLUDES += -I./extern
# INCLUDES += -I./extern/variant/include
INCLUDES += -I$(RAYLIB_PATH)/release/include

# include libraries
# - -L.
# - -L/usr/local/lib 
# - -L$(RAYLIB_PATH)/release/libs/osx
LDLIBS := -L. 
LDLIBS += -L/usr/local/lib
LDLIBS += -L$(RAYLIB_PATH)/release/libs/osx
LDLIBS += -lm
LDLIBS += -lraylib

# compiler warnings
WARNINGS := -Wall #-Wextra #-pedantic 

# optimization level (if any)
OPTIMIATION := -O1


#       ---------------------------------


# files included in the tarball generated by 'make dist' (e.g. add LICENSE file)
DISTFILES := $(BIN)

# filename of the tar archive generated by 'make dist'
DISTOUTPUT := $(BIN).tar.gz

TMP_DIR := .tmp
# intermediate directory for generated object files
OBJDIR := $(TMP_DIR)/.o
# intermediate directory for generated dependency files
DEPDIR := $(TMP_DIR)/.d

# EMSCRIPTEN / HTML5:
ifeq ($(PLATFORM),PLATFORM_WEB)
	OBJDIR := $(TMP_DIR)/.o-web
	DEPDIR := $(TMP_DIR)/.d-web
endif

# object files, auto generated from source files
OBJS := $(patsubst %,$(OBJDIR)/%.o,$(basename $(SRCS)))
# dependency files, auto generated from source files
DEPS := $(patsubst %,$(DEPDIR)/%.d,$(basename $(SRCS)))



# Create required subdirectories
# (compilers (at least gcc and clang) don't create the subdirectories automatically)
$(shell mkdir -p $(dir $(TMP_DIR)) >/dev/null)
$(shell mkdir -p $(dir $(OBJS)) >/dev/null)
$(shell mkdir -p $(dir $(DEPS)) >/dev/null)
$(shell mkdir -p $(BIN_DIR) >/dev/null)


# C compiler
CC := clang
# C++ compiler
CXX := clang++
# linker
LD := clang++
# tar
TAR := tar

# C flags
CFLAGS := -std=c11 
# C++ flags
CXXFLAGS := -std=c++11
# C/C++ flags
CPPFLAGS := -g $(WARNINGS) $(OPTIMIATION) $(INCLUDES) -D$(PLATFORM)
# linker flags
LDFLAGS := -g -Wall 


# EMSCRIPTEN / HTML5:
ifeq ($(PLATFORM),PLATFORM_WEB)

	CC = $(EMSCRIPTEN_PATH)/emcc
	CXX = $(EMSCRIPTEN_PATH)/em++
	LD = $(EMSCRIPTEN_PATH)/em++

	RAYLIB_RELEASE = $(RAYLIB_PATH)/release/libs/html5

	LDLIBS = $(RAYLIB_RELEASE)/libraylib.bc

	# Emscripten Flags; for more info, see:
	# - https://kripken.github.io/emscripten-site/docs/tools_reference/emcc.html
	# - https://hacks.mozilla.org/2018/01/shrinking-webassembly-and-javascript-code-sizes-in-emscripten/
	# - http://floooh.github.io/2016/08/27/asmjs-diet.html
	# - https://github.com/kripken/emscripten/blob/master/src/settings.js
	
	#  -D_DEFAULT_SOURCE    use with -std=c99 on Linux and PLATFORM_WEB, required for timespec
    # -O2                        # if used, also set --memory-init-file 0
    # --memory-init-file 0       # to avoid an external memory initialization code file (.mem)
    # -s ALLOW_MEMORY_GROWTH=1   # to allow memory resizing
    # -s TOTAL_MEMORY=16777216   # to specify heap memory size (default = 16MB)
    # -s USE_PTHREADS=1          # multithreading support
    EMSC_FLAGS := 
	# EMSC_FLAGS += -s WASM=1
	EMSC_FLAGS += -D_DEFAULT_SOURCE
	EMSC_FLAGS += -s USE_GLFW=3 
	EMSC_FLAGS += --preload-file resources/production
	EMSC_FLAGS += --shell-file platform/web/shell.html
	EMSC_FLAGS += --memory-init-file 0
	EMSC_FLAGS += -s TOTAL_MEMORY=16777216 
	# EMSC_FLAGS += -s ASSERTIONS=1 
	# EMSC_FLAGS += -s ELIMINATE_DUPLICATE_FUNCTIONS=1 # slow to run!
	# EMSC_FLAGS += --profiling 
	# EMSC_FLAGS += --cpuprofiler # CPU visualizer
	# EMSC_FLAGS += --memoryprofiler # memory visualizer
	EMSC_FLAGS += -O3 # optimize
	EMSC_FLAGS += -g0 # removes all debug info from JS code (function names, etc)
	# EMSC_FLAGS +=  --closure 1 # run closure compiler (NOTE: '-g0' required for this)

	CPPFLAGS += $(EMSC_FLAGS)
	LDFLAGS += $(EMSC_FLAGS)

	BIN = index
	EXT = .html
endif


# flags required for dependency generation; passed to compilers
DEPFLAGS = -MT $@ -MD -MP -MF $(DEPDIR)/$*.Td

# compile C source files
COMPILE.c = $(CC) $(DEPFLAGS) $(CFLAGS) $(CPPFLAGS) -c -o $@
# compile C++ source files
COMPILE.cc = $(CXX) $(DEPFLAGS) $(CXXFLAGS) $(CPPFLAGS) -c -o $@
# link object files to binary
LINK.o = $(LD) $(LDFLAGS) $(LDLIBS) -o $(BIN_DIR)/$@$(EXT)
# precompile step
PRECOMPILE =
# postcompile step
POSTCOMPILE = mv -f $(DEPDIR)/$*.Td $(DEPDIR)/$*.d

all: $(BIN)

dist: $(DISTFILES)
	$(TAR) -cvzf $(DISTOUTPUT) $^

.PHONY: clean
clean:
	$(RM) -r $(TMP_DIR) $(BIN_DIR)

.PHONY: distclean
distclean: clean
	$(RM) $(BIN) $(DISTOUTPUT)

.PHONY: install
install:
	@echo no install tasks configured

.PHONY: uninstall
uninstall:
	@echo no uninstall tasks configured

.PHONY: check
check:
	@echo no tests configured

# BUILD AND RUN
.PHONY: run
run: all
	@echo "\n\n\n"
	@echo [ RUNNING \'$(BIN_DIR)/$(BIN)\' ]
	./$(BIN_DIR)/$(BIN)


# WEB
.PHONY: web
web:
	make PLATFORM=PLATFORM_WEB

# WEB
.PHONY: run-web
run-web:
	$(EMSCRIPTEN_PATH)/emrun --browser chrome $(BIN_DIR)/index.html

.PHONY: help
help:
	@echo available targets: all dist clean distclean install uninstall check run

$(BIN): $(OBJS)
	$(LINK.o) $^

$(OBJDIR)/%.o: %.c
$(OBJDIR)/%.o: %.c $(DEPDIR)/%.d
	$(PRECOMPILE)
	$(COMPILE.c) $<
	$(POSTCOMPILE)

$(OBJDIR)/%.o: %.cpp
$(OBJDIR)/%.o: %.cpp $(DEPDIR)/%.d
	$(PRECOMPILE)
	$(COMPILE.cc) $<
	$(POSTCOMPILE)

$(OBJDIR)/%.o: %.cc
$(OBJDIR)/%.o: %.cc $(DEPDIR)/%.d
	$(PRECOMPILE)
	$(COMPILE.cc) $<
	$(POSTCOMPILE)

$(OBJDIR)/%.o: %.cxx
$(OBJDIR)/%.o: %.cxx $(DEPDIR)/%.d
	$(PRECOMPILE)
	$(COMPILE.cc) $<
	$(POSTCOMPILE)

.PRECIOUS = $(DEPDIR)/%.d
$(DEPDIR)/%.d: ;

-include $(DEPS)