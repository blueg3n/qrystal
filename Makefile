CXX = g++

CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude

LDFLAGS = -Llib

LIBS = $(shell pkg-config --libs libcurl) \
       -lfltk \
       -lXrender \
       -lXcursor \
       -lXfixes \
       -lXft \
       -lfontconfig \
       -lXinerama \
       -lX11 \
       -pthread \
       -lm

SRC = $(wildcard src/*.cpp)

TARGET = prog

default:
	$(CXX) $(SRC) -o $(TARGET) $(CXXFLAGS) $(LDFLAGS) $(LIBS)

clean:
	rm -f $(TARGET)