// SavedItemsView.hpp

class SavedItemsView {
    GtkWidget *btnSave;
    GtkWidget *btnDelete;
    public:
    SavedItemsView(GtkWidget *btnSave, GtkWidget *btnDelete) {
		this->btnSave = btnSave;
		this->btnDelete = btnDelete;
	}
	
	void showSave() {
	    gtk_widget_show(btnSave);
	    gtk_widget_hide(btnDelete);	
	}
	
	void showDelete() {
		gtk_widget_show(btnDelete);
		gtk_widget_hide(btnSave);
	}
};
