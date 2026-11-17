default:
	g++ src/*.cpp -o prog -Iinclude/qt6 $(shell pkg-config --libs Qt6Widgets)