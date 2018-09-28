// DynamicLinksView.hpp

class DynamicLinksView {
    GtkWidget *frActions;
    GtkWidget *spLinks;
    GtkWidget *btnGetLinks;
    GtkWidget *btnListEpisodes;
    GtkWidget *btnLinksError;
    public:
    DynamicLinksView(GtkWidget *frActions,
                     GtkWidget *spLinks,
                     GtkWidget *btnGetLinks,
                     GtkWidget *btnListEpisodes,
                     GtkWidget *btnLinksError) {
	    this->frActions = frActions;
	    this->spLinks = spLinks;
	    this->btnGetLinks = btnGetLinks;
	    this->btnListEpisodes = btnListEpisodes;
	    this->btnLinksError = btnLinksError;					 
	}
	
	void showLoadingIndicator() {
		gtk_widget_show(frActions);
		gtk_widget_show(spLinks);
		gtk_widget_hide(btnGetLinks);
		gtk_widget_hide(btnListEpisodes);
		gtk_widget_hide(btnLinksError);
		gtk_spinner_start(GTK_SPINNER(spLinks));
	}
	
	void showFilmButton() {
		gtk_widget_hide(spLinks);
		gtk_widget_show(btnGetLinks);
		gtk_spinner_stop(GTK_SPINNER(spLinks));
	}
	
	void showSerialButton() {
		gtk_widget_hide(spLinks);
		gtk_widget_show(btnListEpisodes);
		gtk_spinner_stop(GTK_SPINNER(spLinks));
	}
	
	void showEmpty() {
		gtk_widget_hide(frActions);
		gtk_widget_hide(spLinks);
		gtk_widget_hide(btnGetLinks);
		gtk_widget_hide(btnListEpisodes);
		gtk_widget_hide(btnLinksError);
		gtk_spinner_stop(GTK_SPINNER(spLinks));
	}
	
	void showError() {
		gtk_widget_hide(spLinks);
		gtk_widget_show(btnLinksError);
		gtk_spinner_stop(GTK_SPINNER(spLinks));
	}
};
