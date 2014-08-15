CC=clang++
LD=clang++
CFLAGS=-O0 -Wall -Wextra -pedantic -Werror -std=c++1y -Wno-unused-parameter -Wno-unused-variable -stdlib=libc++ -MD -fPIC -pthread
SOFLAGS=-stdlib=libc++ -shared
LDFLAGS=-stdlib=libc++ -lc++abi
SOURCES=$(shell find . -name "*.cpp" ! -wholename "./tests/*" ! -wholename "*-old")
TESTSRC=$(shell find ./tests/ -name "*.cpp")
OBJECTS=$(SOURCES:.cpp=.o)
TESTOBJ=$(TESTSRC:.cpp=.o)

all: library

install: all
	@sudo cp libreaver.so /usr/local/lib/libreaver.so.1
	@sudo ln -sfn /usr/local/lib/libreaver.so.1 /usr/local/lib/libreaver.so
	@sudo mkdir -p /usr/local/include/reaver
	@sudo find -type f -name "*.h" ! -wholename "*-old" -exec cp --parents {} /usr/local/include/reaver/ \;

library: $(OBJECTS)
	$(LD) $(SOFLAGS) $(OBJECTS) -o libreaver.so -ldl -lboost_system -lboost_filesystem -pthread -lpthread

uninstall:
	@rm -rf /usr/local/include/reaver
	@rm -rf /usr/lib/libreaver.a

%.o: %.cpp
	$(CC) -c $(CFLAGS) $< -o $@ -g

clean:
	@find . -name "*.o" -delete
	@find . -name "*.d" -delete
	@find . -name "*.so" -delete

test: $(TESTOBJ)
	$(LD) $(TESTOBJ) $(LDFLAGS) -o reaverlib-test -lreaver -lboost_program_options -lboost_system -lboost_iostreams -pthread

-include $(SOURCES:.cpp=.d)
-include $(TESTSRC:.cpp=.d)
