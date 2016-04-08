CXX = c++
LD = c++
CXXFLAGS += -O0 -fno-omit-frame-pointer -Wall -std=c++1z -MD -fPIC -Wno-unused-parameter -Wno-unused-const-variable -Wno-undefined-internal -g
SOFLAGS += -shared
LDFLAGS +=
LIBRARIES +=

SOURCES := $(shell find . -name "*.cpp" ! -wholename "./tests/*" ! -name "main.cpp" ! -wholename "./main/*")
# MAINSRC := $(shell find ./main/ -name "*.cpp") main.cpp
TESTSRC := $(shell find ./tests/ -name "*.cpp")
OBJECTS := $(SOURCES:.cpp=.o)
# MAINOBJ := $(MAINSRC:.cpp=.o)
TESTOBJ := $(TESTSRC:.cpp=.o)

PREFIX ?= /usr/local
EXEC_PREFIX ?= $(PREFIX)
BINDIR ?= $(EXEC_PREFIX)/bin
LIBDIR ?= $(EXEC_PREFIX)/lib
INCLUDEDIR ?= $(PREFIX)/include

# LIBRARY = libreaver.so
# EXECUTABLE =

all: test

#library: $(LIBRARY)

# $(EXECUTABLE): $(MAINOBJ)
#	$(LD) $(CXXFLAGS) $(LDFLAGS) $(MAINOBJ) -o $@ $(LIBRARIES)

#$(LIBRARY): $(OBJECTS)
#	$(LD) $(CXXFLAGS) $(SOFLAGS) $(OBJECTS) -o $@ $(LIBRARIES)

test: ./tests/test

./tests/test: $(TESTOBJ) # $(LIBRARY)
	$(LD) $(CXXFLAGS) $(LDFLAGS) $(TESTOBJ) -o $@ $(LIBRARIES) -lboost_system -lboost_iostreams -lboost_program_options -lboost_filesystem -pthread

install: # $(LIBRARY) # $(EXECUTABLE)
#	@cp $(EXECUTABLE) $(DESTDIR)$(BINDIR)/$(EXECUTABLE)
#	@cp $(LIBRARY) $(DESTDIR)$(LIBDIR)/$(LIBRARY).1
#	@ln -sfn $(DESTDIR)$(LIBDIR)/$(LIBRARY).1 $(DESTDIR)$(LIBDIR)/$(LIBRARY)
	@mkdir -p $(DESTDIR)$(INCLUDEDIR)/reaver
	@cp -RT include $(DESTDIR)$(INCLUDEDIR)

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@ -I./include/reaver

./tests/%.o: ./tests/%.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@ -I./include/reaver

clean:
	@find . -name "*.o" -delete
	@find . -name "*.d" -delete
	@rm -f $(LIBRARY)
#	@rm -f $(EXECUTABLE)
	@rm -f tests/test

.PHONY: install clean library test

-include $(SOURCES:.cpp=.d)
# -include $(MAINSRC:.cpp=.d)
-include $(TESTSRC:.cpp=.d)
