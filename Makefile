CXXFLAGS = -std=c++17 -Wall -Wextra

# check for if lstdc++fs is needed
CHECKSTDCPPFS=$(shell find /usr/lib -name libstdc++fs* 2>/dev/null)
ifneq (,$(CHECKSTDCPPFS))
   FSFLAG = -lstdc++fs
endif

LIBRARYFLAGS = -lpthread $(FSFLAG)
LIBRARYFILES = include/terminal/terminal.h include/terminal/tui.h include/terminal/keyboard.h

CC = g++
DIRS = build

all: build/demo

build/demo: src/demo/main.cpp $(LIBRARYFILES)
	$(CC) $(CXXFLAGS) -o build/demo src/demo/main.cpp $(LIBRARYFLAGS)

clean:
	rm build/*

$(shell mkdir -p $(DIRS))
