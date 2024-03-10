/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef __GTK_SCRIPTWINDOW_H
#define __GTK_SCRIPTWINDOW_H

#include "gtk_builder_window.h"

void snes9x_scriptwindow_new();

class Snes9xScriptWindow final : public GtkBuilderWindow
{
  public:
    Snes9xScriptWindow();
    ~Snes9xScriptWindow();

    void script_run();
    void script_stop();

  private:
    void connect_signals();
};

#endif
