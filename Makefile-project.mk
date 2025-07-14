# This is common Makefile for Empirical that sets up a common structure for standard projects.

.PHONY: default native web all debug debug-web web-debug clean

default: native

include ../../Makefile-base.mk

native:	FLAGS := $(FLAGS_OPT)
native: $(PROJECT)

web:	FLAGS_WEB := $(FLAGS_WEB_OPT)
web: $(PROJECT).js

all: native
all: web

debug:	FLAGS := $(FLAGS_DEBUG)
debug:	$(PROJECT)

debug-web:	FLAGS_WEB := $(FLAGS_WEB_DEBUG)
debug-web:	$(PROJECT).js

web-debug:	debug-web

web-threaded: FLAGS_WEB := $(FLAGS_WEB_THREADS)
web-threaded: $(PROJECT).js

$(PROJECT):	source/native/$(PROJECT).cpp
	$(CXX) $(FLAGS) source/native/$(PROJECT).cpp -o $(PROJECT)
	@echo To build the web version use: make web

$(PROJECT).js: source/web/$(PROJECT)-web.cpp
	$(CXX_web) $(FLAGS_WEB) source/web/$(PROJECT)-web.cpp -o web/$(PROJECT).js

CLEAN_EXTRA = $(PROJECT) web/$(PROJECT).js source/*.o source/*~

help:
	@echo "Makefile for $(PROJECT):"
	@echo "  make native        Build optimized native binary"
	@echo "  make debug         Build debug native binary"
	@echo "  make web           Build optimized WebAssembly"
	@echo "  make debug-web     Build debug WebAssembly"
	@echo "  make all           Build both native and web"
	@echo "  make clean         Remove generated files"