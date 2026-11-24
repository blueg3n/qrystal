CXX = g++

CXXFLAGS = -std=c++20 -Wall -Wextra -Iinclude

LDFLAGS = -Llib -Wl,-rpath,'$$ORIGIN/lib' -Wl,--as-needed

LIBS = $(shell pkg-config --libs libcurl) -lAppCore -lUltralight -lUltralightCore -lWebCore -lpthread -ldl

SRC = $(wildcard src/*.cpp)

TARGET = prog

default:
	$(CXX) $(SRC) -o $(TARGET) $(CXXFLAGS) $(LDFLAGS) $(LIBS)

clean:
	rm -f $(TARGET)