// ResultsHistory.hpp

class ResultsHistory {
    Results results;
    vector<Results> backResultsStack, forwardResultsStack;
    set<string> resultsThreadsLinks;
    
    GtkWidget *ivResults;
    GtkWidget *window;
    GtkToolItem *btnPrev;
    GtkToolItem *btnNext;
    string progName;
    public:
    
    ResultsHistory(GtkWidget *w,
                   GtkWidget *iv,
                   GtkToolItem *bp,
                   GtkToolItem *bn,
                   string pn) {
		window = w;
		ivResults = iv;
		btnPrev = bp;
		btnNext = bn;
		progName = pn;
	}
	
	string getUrl() {
		return results.getUrl();
	} 
	
	bool isRefresh() {
		return results.isRefresh();
	} 
	
	void setRefresh(bool refresh) {
		results.setRefresh(refresh);
	}
	
	void clearForwardResultsStack() {
		// Do not clear if refresh button clicked
		if(!isRefresh()) {
			forwardResultsStack.clear();
		    gtk_tool_item_set_tooltip_text(btnNext, 
		                                   "Move forward in history");
		}
	}
	
	void removeBackStackDuplicate() {
		// Linear search for title
		int eraseIndex = -1;
		// If back stack has title, remove results with it.
		for(unsigned i = 0; i < backResultsStack.size(); i++) {
			if(backResultsStack[i].getTitle() == results.getTitle()) {
				eraseIndex = i;
				break;
			}
		}
		
		if(eraseIndex != -1) {
			backResultsStack.erase(backResultsStack.begin() + eraseIndex);
		}
	}
	
	void clearThreadsLinks() {
		resultsThreadsLinks.clear();
	}
	
	void createNewModel() {
		results.createNewModel(ivResults);
	}
	
	void show(string &page) {
		results.show(page);
	}
	
	void setError(bool error) {
		results.setError(error);
	}
	
	bool isError() {
		return results.isError();
	}
	
	void saveResultsPostion() {
		GtkTreePath *path1, *path2;
		if(gtk_icon_view_get_visible_range(GTK_ICON_VIEW(ivResults), &path1, &path2)) {
			string index(gtk_tree_path_to_string(path1));
	        results.setIndex(index);
			gtk_tree_path_free(path1);
		    gtk_tree_path_free(path2);
		}
	}
	
	void saveToBackStack() {
		// Save position and copy to save variable
		if(!results.getTitle().empty()) {
			saveResultsPostion();
	        backResultsStack.push_back(results);
	        // Set tooltip with results title
	        gtk_tool_item_set_tooltip_text(btnPrev, 
	                                       results.getTitle().c_str());
	        gtk_widget_set_sensitive(GTK_WIDGET(btnPrev), TRUE);
		}
	}
	
	void setTitle(string title) {
		cout << "Setting title " << title << endl;
		results.setTitle(title);
	}
	
	string getTitle() {
		return results.getTitle();
	}
	
	void setUrl(string url) {
		results.setUrl(url);
	}
	
	void updateTitle() {
		string title = progName + " - " + results.getTitle();
		gtk_window_set_title(GTK_WINDOW(window), title.c_str());
	}
	
	void setBaseUrl(string url) {
		results.setBaseUrl(url);
	}
	
	void saveToForwardStack() {
		saveResultsPostion();
		forwardResultsStack.push_back(results); 
		// Set tooltip with results title
        gtk_tool_item_set_tooltip_text(btnNext, 
                                       results.getTitle().c_str());
        gtk_widget_set_sensitive(GTK_WIDGET(btnNext), TRUE);
	}
	
	void savedRecovery() {
		// Clear results links set if not paging
	    // (do not allow next page thread to be called twice)
	    resultsThreadsLinks.clear(); 
	    
		// Update ivResults with history results
		results.setModel(ivResults);
		// Scroll to saved position after updating model
		string index = results.getIndex();
		GtkTreePath *path1 = gtk_tree_path_new_from_string(index.c_str());
		gtk_icon_view_scroll_to_path(GTK_ICON_VIEW(ivResults), path1, FALSE, 0, 0);
	    gtk_tree_path_free(path1);
	}
	
	void restoreFromBackStack() {
		results = backResultsStack.back();
	    backResultsStack.pop_back();
	    
	    if(!backResultsStack.empty()) {
			// Set tooltip with results title
			gtk_tool_item_set_tooltip_text(btnPrev, 
										   backResultsStack
											   .back()
											   .getTitle().c_str());
		}else {
			// Set tooltip with results title
		    gtk_tool_item_set_tooltip_text(btnPrev, "Move back in history");
		    gtk_widget_set_sensitive(GTK_WIDGET(btnPrev), FALSE);
		}
		
		savedRecovery();
	    updateTitle();
	}
	
	void restoreFromForwardStack() {
		results = forwardResultsStack.back();
	    forwardResultsStack.pop_back();
	    if(!forwardResultsStack.empty()) {
			// Set tooltip with results title
		    gtk_tool_item_set_tooltip_text(btnNext, 
		                                   forwardResultsStack
		                                       .back()
		                                       .getTitle().c_str());                                   
		}else {
			// Set tooltip with results title
		    gtk_tool_item_set_tooltip_text(btnNext, "Move forward in history");
		    gtk_widget_set_sensitive(GTK_WIDGET(btnNext), FALSE);
		}
	    
	    savedRecovery();
	    updateTitle(); 
	}
	
	string getNextLink() {
		return results.getNextLink();
	}
	
	bool threadLinksContainNextLink() {
		return resultsThreadsLinks.count(results.getNextLink()) > 0;
	}
	
	void insertNextLinkToThreadLinks() {
		resultsThreadsLinks.insert(results.getNextLink());
	}
	
	void eraseNextLinkFromThreadLinks() {
		resultsThreadsLinks.erase(results.getNextLink());
	}
	
};
