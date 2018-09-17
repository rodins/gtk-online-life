//CenterView.hpp

class CenterView {
	GtkWidget *window;
	GtkWidget *ivResults;
	GtkWidget *vbCenter;
	GtkWidget *spCenter;
    GtkWidget *swTree, *swIcon;
	GtkWidget *hbResultsError;
	
	GtkToolItem *btnSavedItems, *btnRefresh, *btnUp, *btnPrev, *btnNext;
	public:
	CenterView(GtkWidget *window, GtkWidget *ivResults,
	           GtkWidget *vbCenter, GtkWidget *spCenter, 
	           GtkWidget *swIcon, GtkWidget *swTree,
	           GtkWidget *hbResultsError,
	           GtkToolItem *btnSavedItems, GtkToolItem *btnRefresh,
	           GtkToolItem *btnUp, GtkToolItem *btnPrev, 
	           GtkToolItem *btnNext) {
		this->window = window;
		this->ivResults = ivResults;
		this->vbCenter = vbCenter;
		this->spCenter = spCenter;
		this->swIcon = swIcon;
		this->swTree = swTree;
		this->hbResultsError = hbResultsError;
		
		this->btnSavedItems = btnSavedItems;
		this->btnRefresh = btnRefresh;
		this->btnUp = btnUp;
		this->btnPrev = btnPrev;
		this->btnNext = btnNext;	   
    }
    
    void setWindowTitle(string title) {
		gtk_window_set_title(GTK_WINDOW(window), title.c_str());
	}
	
	void setResultsModel(ResultsModel &model) {
		gtk_icon_view_set_model(GTK_ICON_VIEW(ivResults),
		                        model.getTreeModel());
	}
	
	void scrollToTopOfList() {
		// Scroll to the top of the list on new results (not append to list)
	    GtkTreePath *path = gtk_tree_path_new_first();
	    gtk_icon_view_scroll_to_path(GTK_ICON_VIEW(ivResults),
	                                 path, 
	                                 FALSE, 
	                                 0, 
	                                 0);
	}
	
	void setSensitiveSavedItems(bool state=TRUE) {
		gtk_widget_set_sensitive(GTK_WIDGET(btnSavedItems), state);
	}
	
	void setSensitiveReferesh(bool state=TRUE) {
		gtk_widget_set_sensitive(GTK_WIDGET(btnRefresh), state);
	}
	
	void setSensitiveUp(bool state=TRUE) {
		gtk_widget_set_sensitive(GTK_WIDGET(btnUp), state);
	}
	
	void setSensitivePrev(bool state=TRUE) {
		gtk_widget_set_sensitive(GTK_WIDGET(btnPrev), state);
	}
	
	void setSensitiveNext(bool state=TRUE) {
		gtk_widget_set_sensitive(GTK_WIDGET(btnNext), state);
	}
    
    void showLoadingIndicator(bool isPage) {
		showToolbarLoadingIndicator();
		// Change packing params of spCenter
		gtk_box_set_child_packing(
		    GTK_BOX(vbCenter),
		    spCenter,
		    !isPage,
		    FALSE,
		    1,
		    GTK_PACK_START);
		gtk_widget_show(spCenter);
		gtk_widget_set_visible(swIcon, isPage);
		gtk_widget_hide(swTree);
		gtk_widget_hide(hbResultsError);
		gtk_spinner_start(GTK_SPINNER(spCenter));
	}
	
	void showResultsData() {
		gtk_widget_hide(swTree);
		gtk_widget_show(swIcon);
		gtk_widget_hide(spCenter);
		gtk_widget_hide(hbResultsError);
		gtk_spinner_stop(GTK_SPINNER(spCenter));
	}
	
	void showPlaylistsData() {
		gtk_widget_hide(spCenter);
		gtk_widget_hide(swIcon);
		gtk_widget_show(swTree);
		gtk_widget_hide(hbResultsError);
		gtk_spinner_stop(GTK_SPINNER(spCenter));
	}
	
	void showError(bool isPage) {
		// Change packing params of spCenter
		gtk_box_set_child_packing(
		    GTK_BOX(vbCenter),
		    hbResultsError,
		    !isPage,
		    FALSE,
		    1,
		    GTK_PACK_START);
		gtk_widget_hide(spCenter);
		// Show and hide of ivResults depends on isPage
		gtk_widget_set_visible(swIcon, isPage);
		gtk_widget_hide(swTree);
		gtk_widget_show(hbResultsError);
		gtk_spinner_stop(GTK_SPINNER(spCenter));
	}
	
	private:
	
	void showToolbarLoadingIndicator() {
		gtk_widget_set_sensitive(GTK_WIDGET(btnSavedItems), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(btnRefresh), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(btnUp), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(btnPrev), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(btnNext), FALSE);
	}
};
