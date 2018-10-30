// ErrorDialogs.hpp

class ErrorDialogs {
    GtkWidget *window;
    public:
    ErrorDialogs(GtkWidget *window) {
		this->window = window;
	}
    
    void runBrowserErrorDialog() {
		runErrorDialog("Supported browser is not found");
	}
	
	private:
	
	void runErrorDialog(const char* message) {
		GtkWidget *dialog = gtk_message_dialog_new(
		              GTK_WINDOW(window),
		              GTK_DIALOG_DESTROY_WITH_PARENT,
		              GTK_MESSAGE_ERROR,
		              GTK_BUTTONS_OK,
		              "%s", message);
		gtk_window_set_title(GTK_WINDOW(dialog), "Error");
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
	}
};
