#include "gui.h"

RefPtr<App> qrystal;
RefPtr<Window> qWindow;
RefPtr<Panel> qPanel;
WindowListener qListener;

js::API app("qrystal");

void testing() {
    cout << "REGISTERED\n";
}

void attach(View* view) {
    app["testing"] = testing;

    if (app.AttachTo(view)) {
        view->LoadURL("file:///example.html");
    }
}

void render() {
    qrystal = App::Create();
    qWindow = Window::Create(qrystal->main_monitor(), 1280.0, 720.0, false, WindowFlags::Titled | WindowFlags::Resizable);
    qWindow->SetTitle("Qrystal");
    qPanel = qWindow->AddPanel();
    
    qWindow->set_listener(&qListener);
    
    app.Bind("testing", testing);
    attach(qPanel->view().get());    

    qrystal->Run();
}

void kill() {
    qWindow->Close();
    qrystal->Quit();
}