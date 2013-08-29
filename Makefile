CC=clang++
LD=clang++
CFLAGS=-Os -Wall -Wextra -pedantic -Werror -std=c++11 -Wno-unused-parameter -Wno-unused-variable -stdlib=libc++ -MD -fPIC
SOFLAGS=-stdlib=libc++ -shared
SOURCES=$(shell find . -name "*.cpp" ! -wholename "./tests/*")
OBJECTS=$(SOURCES:.cpp=.o)

all: library

install: all
	@cp libreaver.so /usr/local/lib/libreaver.so.1
	@ln -sfn /usr/local/lib/libreaver.so.1 /usr/local/lib/libreaver.so
	@mkdir -p /usr/local/include/reaver
	@find -type f -name "*.h" -exec cp --parents {} /usr/local/include/reaver/ \;

library: $(OBJECTS)
	$(LD) $(SOFLAGS) $(OBJECTS) -o libreaver.so -ldl

uninstall:
	@rm -rf /usr/local/include/reaver
	@rm -rf /usr/lib/libreaver.a

%.o: %.cpp
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	@find . -name "*.o" -delete
	@find . -name "*.d" -delete
	@find . -name "*.so" -delete

test: all
	$(CC) $(CFLAGS) tests/main.cpp -o tests/output -lc++abi -lreaver -lboost_system -lboost_filesystem -pthread
#	$(CC) $(CFLAGS) tests/calc.cpp -o tests/calc -lc++abi -lreaver -lboost_system -lboost_filesystem -pthread

-include $(SOURCES:.cpp=.d)
