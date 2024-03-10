#include <string>
#include "lua-engine.h"
#include "gtk_lua_window.h"

LuaConsole *console;

static void print_to_win_console(int id, const char* str)
{
	console->print_to_win_console(id, str);
}

static void on_start(int id)
{
	console->on_start(id);
}

static void on_stop(int id, bool status_ok)
{
	console->on_stop(id);
}

LuaWindow::LuaWindow(int id) :
	id_(id),
	running_(false)
{
	window_ = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	script_lbl_ = gtk_label_new("Script File");
	script_entry_ = gtk_entry_new();
	browse_btn_ = gtk_button_new_with_label("Browse");
	edit_btn_ = gtk_button_new_with_label("Edit");
	stop_btn_ = gtk_button_new_with_label("Stop");
	run_btn_ = gtk_button_new_with_label("Run");

	gtk_window_set_default_size(GTK_WINDOW(window_), 450, 300);

	gtk_container_set_border_width(GTK_CONTAINER(window_), 5);

	gtk_label_set_xalign(GTK_LABEL(script_lbl_), -1.0);

	GtkWidget *main_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
	gtk_box_pack_start(GTK_BOX(main_vbox), script_lbl_, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(main_vbox), script_entry_, FALSE, FALSE, 0);

	GtkWidget *button_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
	GtkWidget *button_spacer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
	gtk_widget_set_size_request(button_spacer, 100, -1);
	gtk_box_pack_start(GTK_BOX(button_hbox), browse_btn_, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(button_hbox), edit_btn_, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(button_hbox), button_spacer, TRUE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(button_hbox), stop_btn_, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(button_hbox), run_btn_, TRUE, TRUE, 0);

	gtk_box_pack_start(GTK_BOX(main_vbox), button_hbox, FALSE, FALSE, 0);

	console_out_ = gtk_text_view_new();
	console_buffer_ = gtk_text_view_get_buffer(GTK_TEXT_VIEW(console_out_));
	console_hadj_ = gtk_adjustment_new(0.0, 0.0, 0.0, 1.0, 1.0, 1.0);
	console_vadj_ = gtk_adjustment_new(0.0, 0.0, 0.0, 1.0, 1.0, 1.0);
	console_scroll_ = gtk_scrolled_window_new(console_hadj_, console_vadj_);

	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(console_scroll_), console_out_);
	gtk_widget_set_size_request(console_scroll_, -1, 200);
	gtk_box_pack_start(GTK_BOX(main_vbox), console_scroll_, TRUE, TRUE, 0);

	gtk_container_add(GTK_CONTAINER(window_), main_vbox);

	gtk_widget_show_all(window_);

	g_signal_connect(browse_btn_, "clicked", G_CALLBACK(LuaWindow::browse_thunk), this);
	g_signal_connect(edit_btn_, "clicked", G_CALLBACK(LuaWindow::edit_thunk), this);
	g_signal_connect(stop_btn_, "clicked", G_CALLBACK(LuaWindow::stop_thunk), this);
	g_signal_connect(run_btn_, "clicked", G_CALLBACK(LuaWindow::run_thunk), this);
	g_signal_connect(window_, "delete-event", G_CALLBACK(LuaWindow::delete_thunk), this);

	OpenLuaContext(id_, ::print_to_win_console, ::on_start, ::on_stop);
}

LuaWindow::~LuaWindow()
{
	CloseLuaContext(id_);
}

void LuaWindow::show()
{
	gtk_widget_show_all(window_);
}

uint32_t LuaWindow::get_id()
{
	return id_;
}

void LuaWindow::print_to_win_console(const char *str)
{
	GtkTextIter iter;
	gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(console_buffer_), &iter);
	gtk_text_buffer_insert(GTK_TEXT_BUFFER(console_buffer_), &iter, str, strlen(str));
}

void LuaWindow::on_start()
{
	GtkTextIter iter;
	gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(console_buffer_), &iter);

	std::string text = "Started.\n";
	gtk_text_buffer_insert(GTK_TEXT_BUFFER(console_buffer_), &iter, text.c_str(), text.size());

	running_ = true;

	gtk_button_set_label(GTK_BUTTON(run_btn_), "Restart");
}

void LuaWindow::on_stop()
{
	GtkTextIter iter;
	gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(console_buffer_), &iter);

	std::string text = "Stopped.\n";
	gtk_text_buffer_insert(GTK_TEXT_BUFFER(console_buffer_), &iter, text.c_str(), text.size());

	gtk_button_set_label(GTK_BUTTON(run_btn_), "Run");
}

void LuaWindow::close()
{
	if (running_)
	{
		StopLuaScript(id_);
	}

	gtk_window_close(GTK_WINDOW(window_));
	gtk_widget_destroy(window_);
}

void LuaWindow::browse_thunk(GtkWidget *widget, gpointer data)
{
	LuaWindow *window = (LuaWindow *)data;
	window->browse_clicked();
}

void LuaWindow::edit_thunk(GtkWidget *widget, gpointer data)
{
	LuaWindow *window = (LuaWindow *)data;
	window->edit_clicked();
}

void LuaWindow::stop_thunk(GtkWidget *widget, gpointer data)
{
	LuaWindow *window = (LuaWindow *)data;
	window->stop_clicked();
}

void LuaWindow::run_thunk(GtkWidget *widget, gpointer data)
{
	LuaWindow *window = (LuaWindow *)data;
	window->run_clicked();
}

gboolean LuaWindow::delete_thunk(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	LuaWindow *window = (LuaWindow *)data;
	return window->on_delete();
}

void LuaWindow::browse_clicked()
{
	GtkWidget     *dialog;
	GtkFileFilter *filter;
	char          *filename;
	gint          result;
	const char    *extensions[] =
	{
			"*.lua", "*.LUA",
			NULL
	};


	dialog = gtk_file_chooser_dialog_new ("Load Lua Script",
										  GTK_WINDOW (window_),
										  GTK_FILE_CHOOSER_ACTION_SAVE,
										  "gtk-cancel", GTK_RESPONSE_CANCEL,
										  "gtk-open", GTK_RESPONSE_ACCEPT,
										  NULL);

	filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter, "Lua Scripts");
	for (int i = 0; extensions[i]; i++)
	{
		gtk_file_filter_add_pattern (filter, extensions[i]);
	}
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);

	filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter, "All Files");
	gtk_file_filter_add_pattern (filter, "*.*");
	gtk_file_filter_add_pattern (filter, "*");
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);

	result = gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_hide (dialog);

	if (result == GTK_RESPONSE_ACCEPT)
	{
		filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
	}
	else
	{
		filename = strdup ("");
	}

	gtk_entry_set_text(GTK_ENTRY(script_entry_), filename);

	if (strlen(filename) > 0)
	{
		RunLuaScriptFile(id_, filename);
	}

	gtk_widget_destroy (dialog);
}

void LuaWindow::edit_clicked()
{
	const gchar *text =	gtk_entry_get_text(GTK_ENTRY(script_entry_));
	if (strlen(text) == 0)
	{
		return;
	}
	std::string f = "file://" + std::string(text);
	g_app_info_launch_default_for_uri(f.c_str(), NULL, NULL);
}

void LuaWindow::stop_clicked()
{
	StopLuaScript(id_);
}

void LuaWindow::run_clicked()
{
	if (running_)
	{
		StopLuaScript(id_);
	}

	const gchar *text =	gtk_entry_get_text(GTK_ENTRY(script_entry_));
	if (strlen(text) == 0)
	{
		return;
	}

	RunLuaScriptFile(id_, text);
}

gboolean LuaWindow::on_delete()
{
	if (running_)
	{
		StopLuaScript(id_);
	}
	return FALSE;
}

// -----------------------------------------------------

LuaConsole::LuaConsole() :
	current_id_(0)
{
	console = this;
}

LuaConsole::~LuaConsole()
{

}

void LuaConsole::add_new_window()
{
	int cid = ++current_id_;
	windows_[cid] = new LuaWindow(cid);
}

void LuaConsole::close_all_windows()
{
	for (std::pair<uint32_t, LuaWindow *> pair : windows_)
	{
		pair.second->close();
	}
}

void LuaConsole::print_to_win_console(uint32_t id, const char *str)
{
	LuaWindow *win = windows_[id];
	if (!win)
	{
		return;
	}

	win->print_to_win_console(str);
}

void LuaConsole::on_start(uint32_t id)
{
	LuaWindow *win = windows_[id];
	if (!win)
	{
		return;
	}

	win->on_start();
}

void LuaConsole::on_stop(uint32_t id)
{
	LuaWindow *win = windows_[id];
	if (!win)
	{
		return;
	}

	win->on_stop();
}
