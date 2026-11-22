#include "gui.h"

int renderGUI(int argc, char** argv) {

    Fl_Window window(400, 300, "My FLTK Application");

    Fl_Box box(100, 120, 200, 60, "Hello, FLTK!");

    window.end();
    window.show();

    return Fl::run();
}