default:
	g++ src/*.cpp -o prog -Iinclude/qt6 -Iinclude $(shell pkg-config --libs Qt6Widgets) $(shell pkg-config --libs libcurl)