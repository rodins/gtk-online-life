// DynamicLinksView.hpp

class DynamicLinksView {
    GtkWidget *spLinks;
    GtkWidget *btnGetLinks;
    GtkWidget *btnLinksError;
    public:
    DynamicLinksView(GtkWidget *spLinks,
                     GtkWidget *btnGetLinks,
                     GtkWidget *btnLinksError) {
	    this->spLinks = spLinks;
	    this->btnGetLinks = btnGetLinks;
	    this->btnLinksError = btnLinksError;					 
	}
	
	void showLoadingIndicator() {
		gtk_widget_show(spLinks);
		gtk_widget_hide(btnGetLinks);
		gtk_widget_hide(btnLinksError);
		gtk_spinner_start(GTK_SPINNER(spLinks));
	}
	
	void showFilmButton() {
		gtk_widget_hide(spLinks);
		gtk_widget_show(btnGetLinks);
		gtk_widget_hide(btnLinksError);
		gtk_spinner_stop(GTK_SPINNER(spLinks));
	}
	
	void showSerialButton() {
		gtk_widget_hide(spLinks);
		gtk_widget_hide(btnGetLinks);
		gtk_widget_hide(btnLinksError);
		gtk_spinner_stop(GTK_SPINNER(spLinks));
	}
	
	void showEmpty() {
		gtk_widget_hide(spLinks);
		gtk_widget_hide(btnGetLinks);
		gtk_widget_hide(btnLinksError);
		gtk_spinner_stop(GTK_SPINNER(spLinks));
	}
	
	void showError() {
		gtk_widget_hide(spLinks);
		gtk_widget_hide(btnGetLinks);
		gtk_widget_show(btnLinksError);
		gtk_spinner_stop(GTK_SPINNER(spLinks));
	}
};
