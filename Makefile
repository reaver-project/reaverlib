CC=clang++
AR=ar
CFLAGS=-Os -Wall -Wextra -pedantic -Werror -std=c++11 -Wno-unused-parameter -Wno-unused-variable -stdlib=libc++ -MD
SOURCES=$(shell find . -name "*.cpp" ! -wholename "./tests/*")
OBJECTS=$(SOURCES:.cpp=.o)

all: library

install: all
	@cp libreaver.a /usr/local/lib/
	@mkdir -p /usr/local/include/reaver
	@find -type f -name "*.h" -exec cp --parents {} /usr/local/include/reaver/ \;

library: $(OBJECTS)
	$(AR) crv libreaver.a $(OBJECTS)

uninstall:
	@rm -rf /usr/local/include/reaver
	@rm -rf /usr/lib/libreaver.a

%.o: %.cpp
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	@rm -rf *.o
	@rm -rf *.a
	@rm -rf */*.o

test: all
	$(CC) $(CFLAGS) tests/main.cpp -o tests/output -lc++abi -lreaver
	$(CC) $(CFLAGS) tests/calc.cpp -o tests/calc -lc++abi -lreaver

-include $(SOURCES:.cpp=.d)
