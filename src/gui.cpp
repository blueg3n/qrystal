#include "gui.h"

void renderGUI() {
    RefPtr<App> app = App::Create();
    RefPtr<Window> window = Window::Create(
        app->main_monitor(), 1280, 720, false,
        WindowFlags::Titled | WindowFlags::Resizable
    );
    window->SetTitle("Qrystal");

    RefPtr<Panel> panel = window->AddPanel();
    panel->view()->LoadURL("file:///index.html");

    app->Run();
}