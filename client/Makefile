DNS_SERVER ?= 127.0.0.1
DNS_POLL_INTERVAL ?= 1000

CXX = x86_64-w64-mingw32-g++
FLAGS = -Wall -Wextra -Werror -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -Wno-unused-but-set-variable -Wno-unused-value -Wno-unused-label -Wno-unused-result -Wno-unused-local-typedefs \
		-DDNS_SERVER=\"$(DNS_SERVER)\" -DDNS_POLL_INTERVAL=$(DNS_POLL_INTERVAL)
LIBS = -ldnsapi -lws2_32
CXXFLAGS = $(FLAGS) $(LIBS)
SRC = $(wildcard src/*.cpp)
OBJ = $(SRC:.cpp=.o)

TARGET = pwnsh.exe

.PHONY: all
all: $(TARGET)

.PHONY: clean
clean:
	rm -f $(OBJ) $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) -o $@ $^ $(CXXFLAGS)

src/%.o: src/%.cpp
	$(CXX) -c -o $@ $< $(CXXFLAGS)

.PHONY: compiledb
compiledb: clean
	compiledb make
