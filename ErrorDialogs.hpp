// ErrorDialogs.hpp

class ErrorDialogs {
    GtkWidget *window;
    public:
    ErrorDialogs(GtkWidget *window) {
		this->window = window;
	}
    
    void runLinksErrorDialog() {
		runErrorDialog("No links found");
	}
	
	void runNetErrorDialog() {
		runErrorDialog("Network problem");
	}
	
	private:
	
	void runErrorDialog(string message) {
		GtkWidget *dialog = gtk_message_dialog_new(
		              GTK_WINDOW(window),
		              GTK_DIALOG_DESTROY_WITH_PARENT,
		              GTK_MESSAGE_ERROR,
		              GTK_BUTTONS_OK,
		              message.c_str());
		gtk_window_set_title(GTK_WINDOW(dialog), "Error");
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
	}
};
