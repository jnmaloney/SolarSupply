CC=emcc
WEBCORE_DIR:=../../WebCore
SOURCES:=main.cpp $(wildcard src/*.cpp)
LDFLAGS=-O2 --llvm-opts 2
OUTPUT=out/provis.js
#OUTPUT=out/index.html
USE_IMGUI=-I$(WEBCORE_DIR)/imgui/
USE_LIBPNG=-s USE_LIBPNG=1 -s USE_ZLIB=1
USE_WEBGL=-s FULL_ES3=1 -s USE_GLFW=3 -s USE_WEBGL2=1 -lGL
USE_WEBGL_MOB=-s FULL_ES2=1 -s USE_GLFW=3 -lGL
USE_ENGINE_WEBCORE=-I$(WEBCORE_DIR)/include/  $(wildcard $(WEBCORE_DIR)/src/*.cpp) $(wildcard $(WEBCORE_DIR)/imgui/*.cpp)
USE_ASYNC_FILES=-s EMTERPRETIFY=1 -s EMTERPRETIFY_ASYNC=1
USE_FETCH=-s FETCH=1

all: $(SOURCES) $(OUTPUT)

$(OUTPUT): $(SOURCES)
	$(CC) -Isrc -I/usr/local/include/ $(SOURCES) $(USE_IMGUI) -std=c++1z $(USE_LIBPNG) $(USE_WEBGL) $(USE_ENGINE_WEBCORE) $(LDFLAGS) -o $(OUTPUT) -s WASM=1 -s ASSERTIONS=1 --preload-file data $(USE_FETCH) --emrun -s ALLOW_MEMORY_GROWTH=1

clean:
	rm $(OUTPUT)
