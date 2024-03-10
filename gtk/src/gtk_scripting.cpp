/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include "gtk_scripting.h"
#include "lua-engine.h"

static Snes9xScriptWindow *scriptWindow = nullptr;

void snes9x_scriptwindow_new()
{
    if (!scriptWindow) {
        scriptWindow = new Snes9xScriptWindow();
    }
}

void PrintToWindowConsole(int hDlgAsInt, const char* str)
{
    Snes9xScriptWindow *sw = (Snes9xScriptWindow *)hDlgAsInt;
//    fprintf(stderr, "%s: this is %p.\n", );

}

void OnStart(int hDlgAsInt)
{

}

void OnStop(int hDlgAsInt, bool statusOK)
{

}

Snes9xScriptWindow::Snes9xScriptWindow()
    : GtkBuilderWindow("script_dialog")
{
    connect_signals();

    auto dialog = Glib::RefPtr<Gtk::Dialog>::cast_static(window);

    auto chooser = get_object<Gtk::FileChooserDialog>("script_chooser");
    chooser->set_current_folder("./");

    auto filter = Gtk::FileFilter::create();
    filter->set_name("Lua Scripts");
    filter->add_pattern("*.lua");
    chooser->add_filter(filter);

    dialog->show();
}

Snes9xScriptWindow::~Snes9xScriptWindow()
{

}

void Snes9xScriptWindow::connect_signals()
{
    get_object<Gtk::Button>("script_run")->signal_clicked().connect(sigc::mem_fun(*this, &Snes9xScriptWindow::script_run));
    get_object<Gtk::Button>("script_stop")->signal_clicked().connect(sigc::mem_fun(*this, &Snes9xScriptWindow::script_stop));
}

void Snes9xScriptWindow::script_run()
{
    fprintf(stderr, "script window this is %p.\n", this);
    // Shouldn't be doing this, but should be fine.
    int uid = (int)(int64_t)this;

    auto chooser = get_object<Gtk::FileChooserDialog>("script_chooser");
    auto filename = chooser->get_filename();
    auto fn = filename.c_str();

    if (fn == NULL) {
        fprintf(stderr, "Error: No lua file to run.\n");
        return;
    }

    OpenLuaContext((int)uid, PrintToWindowConsole, OnStart, OnStop);
    RunLuaScriptFile((int)uid, fn);
}

void Snes9xScriptWindow::script_stop()
{

}

