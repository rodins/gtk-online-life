//CenterView.hpp

class CenterView {
	GtkWidget *window;
	GtkWidget *ivResults;
	GtkWidget *vbCenter;
	GtkWidget *spCenter;
    GtkWidget *swTree, *swIcon;
	GtkWidget *hbResultsError;
	
	GtkToolItem *btnSavedItems, *btnRefresh, *btnUp, *btnPrev, *btnNext;
	string progName, modelTitle;
	SavedItemsModel *savedItemsModel;
	public:
	CenterView(GtkWidget *window, string progName, GtkWidget *ivResults,
	           GtkWidget *vbCenter, GtkWidget *spCenter, 
	           GtkWidget *swIcon, GtkWidget *swTree,
	           GtkWidget *hbResultsError,
	           GtkToolItem *btnSavedItems, GtkToolItem *btnRefresh,
	           GtkToolItem *btnUp, GtkToolItem *btnPrev, 
	           GtkToolItem *btnNext, SavedItemsModel *savedItemsModel) {
		this->window = window;
		this->progName = progName;
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
		
		this->savedItemsModel = savedItemsModel;	   
    }
    
    bool isSavedItemsPressed() {
		return gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(btnSavedItems));
	}
	
	void setSavedItemsActive(bool state) {
		gtk_toggle_tool_button_set_active(
		                  GTK_TOGGLE_TOOL_BUTTON(btnSavedItems), state);
	}
	
	void showSavedItems() {
		if(!savedItemsModel->isEmpty()) {
			gtk_icon_view_set_model(GTK_ICON_VIEW(ivResults),
		                                 savedItemsModel->getTreeModel());
		    scrollToTopOfList();
	        showToolbarSavedItems();
	        setTitle("Saved items");
		}
	}
    
    string getPosition() {
		GtkTreePath *path1, *path2;
		if(gtk_icon_view_get_visible_range(GTK_ICON_VIEW(ivResults), &path1, &path2)) {
			string index(gtk_tree_path_to_string(path2));
			gtk_tree_path_free(path1);
		    gtk_tree_path_free(path2);
		    return index;
		}
		return "";
	}
    
    void setTitle(string title) {
		string winTitle;
		if(title.empty()) {
			winTitle = progName;
		}else {
			winTitle = progName + " - " + title;
		}
		gtk_window_set_title(GTK_WINDOW(window), winTitle.c_str());
	}
	
	void setResultsModel(ResultsModel &model) {
		modelTitle = model.getTitle();
		setTitle(modelTitle);
		gtk_icon_view_set_model(GTK_ICON_VIEW(ivResults),
		                        model.getTreeModel());
		setPosition(model.getPosition());
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
	
	void setPosition(string index) {
		if(!index.empty()) {
			GtkTreePath *path = gtk_tree_path_new_from_string(index.c_str());
			gtk_icon_view_scroll_to_path(GTK_ICON_VIEW(ivResults), 
			                             path, 
			                             FALSE, 
			                             0, 
			                             0);
		    gtk_tree_path_free(path);
		}
	}
	
	void setSensitiveSavedItems() {
		gtk_widget_set_sensitive(GTK_WIDGET(btnSavedItems),
		                         !savedItemsModel->isEmpty());
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
	
	void setTooltipPrev(string text) {
		gtk_tool_item_set_tooltip_text(btnPrev, text.c_str());
	}
    
    void setTooltipNext(string text) {
		gtk_tool_item_set_tooltip_text(btnNext, text.c_str());
	}
    
    void showLoadingIndicator(bool isPage) {
		if(!isPage) {
			setTitle("Loading...");
		}
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
		setTitle(modelTitle);
		setSensitiveSavedItems();
		gtk_widget_hide(swTree);
		gtk_widget_show(swIcon);
		gtk_widget_hide(spCenter);
		gtk_widget_hide(hbResultsError);
		gtk_spinner_stop(GTK_SPINNER(spCenter));
	}
	
	void showPlaylistsData() {
		setSensitiveUp();
		gtk_widget_hide(spCenter);
		gtk_widget_hide(swIcon);
		gtk_widget_show(swTree);
		gtk_widget_hide(hbResultsError);
		gtk_spinner_stop(GTK_SPINNER(spCenter));
	}
	
	void showError(bool isPage) {
		if(!isPage) {
			setTitle("Error");
		}
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
	
	void showToolbarSavedItems() {
		gtk_widget_set_sensitive(GTK_WIDGET(btnRefresh), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(btnUp), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(btnPrev), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(btnNext), FALSE);
	}
	
	void showToolbarLoadingIndicator() {
		gtk_widget_set_sensitive(GTK_WIDGET(btnSavedItems), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(btnRefresh), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(btnUp), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(btnPrev), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(btnNext), FALSE);
	}
};
