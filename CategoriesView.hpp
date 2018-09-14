// CategoriesView.hpp

class CategoriesView {
	GtkWidget *vbLeft;
    GtkWidget *spCategories;
    GtkWidget *swCategories;
    GtkWidget *hbCategoriesError;
    public:
    CategoriesView(GtkWidget* vbLeft,
                   GtkWidget *spCategories, 
                   GtkWidget *swCategories,
                   GtkWidget *hbCategoriesError) {
		this->vbLeft = vbLeft;			   
	    this->spCategories = spCategories;
	    this->swCategories = swCategories;
	    this->hbCategoriesError = hbCategoriesError;		   
	}
	
	void btnCategoriesClicked() {
		gtk_widget_set_visible(vbLeft, !gtk_widget_get_visible(vbLeft));
	}
	
	void showLoadingIndicator() {
		gtk_widget_show(spCategories);
		gtk_spinner_start(GTK_SPINNER(spCategories));
		gtk_widget_hide(swCategories);
		gtk_widget_hide(hbCategoriesError);
	}
	
	void showData() {
		gtk_widget_hide(spCategories);
		gtk_spinner_stop(GTK_SPINNER(spCategories));
		gtk_widget_show(swCategories);
	}
	
	void showError() {
		gtk_widget_hide(spCategories);
		gtk_spinner_stop(GTK_SPINNER(spCategories));
		gtk_widget_show(hbCategoriesError);
	}
};
