#pragma once

#include <map>
#include <gtk/gtk.h>

class LuaWindow
{
public:
	LuaWindow(int id);
	virtual ~LuaWindow();
	void show();
	uint32_t get_id();

	void print_to_win_console(const char *str);
	void on_start();
	void on_stop();
	void close();
private:
	GtkWidget *window_;
	GtkWidget *script_lbl_;
	GtkWidget *script_entry_;
	GtkWidget *browse_btn_;
	GtkWidget *edit_btn_;
	GtkWidget *stop_btn_;
	GtkWidget *run_btn_;
	GtkWidget *console_out_;
	GtkTextBuffer *console_buffer_;
	GtkWidget *console_scroll_;
	GtkAdjustment *console_hadj_;
	GtkAdjustment *console_vadj_;

	int id_;

	bool running_;

	static void browse_thunk(GtkWidget *widget, gpointer data);
	static void edit_thunk(GtkWidget *widget, gpointer data);
	static void stop_thunk(GtkWidget *widget, gpointer data);
	static void run_thunk(GtkWidget *widget, gpointer data);
	static gboolean delete_thunk(GtkWidget *widget, GdkEvent *event, gpointer data);

	void browse_clicked();
	void edit_clicked();
	void stop_clicked();
	void run_clicked();
	gboolean on_delete();
};

class LuaConsole
{
public:
	LuaConsole();
	virtual ~LuaConsole();

	void add_new_window();
	void close_all_windows();

	void print_to_win_console(uint32_t id, const char *str);
	void on_start(uint32_t id);
	void on_stop(uint32_t id);
private:
	std::map<uint32_t, LuaWindow *> windows_;
	int current_id_;
};
