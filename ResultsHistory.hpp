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
    
    GtkWidget *swTree, *swIcon;
    GtkWidget *spCenter;
	GtkWidget *vbCenter;
	GtkWidget *hbResultsError;
	
	GtkToolItem *btnUp;
	GtkToolItem *rbActors;
    GtkToolItem *rbPlay;
    GtkToolItem *rbDownload;
    GtkToolItem *btnRefresh;
    
    set<int> *imageIndexes;
    
    GThreadPool *resultsNewThreadPool;
    GThreadPool *resultsAppendThreadPool;
    public:
    
    ResultsHistory(GtkWidget *w,
                   GtkWidget *iv,
                   GtkToolItem *bp,
                   GtkToolItem *bn,
                   GtkWidget *sw_tree,
                   GtkWidget *sw_icon,
                   GtkWidget *sp_center,
                   GtkWidget *vb_center,
                   GtkWidget *hb_results_error,
                   GtkToolItem *btn_up,
                   GtkToolItem *rb_actors,
                   GtkToolItem *rb_play,
                   GtkToolItem *rb_download,
                   GtkToolItem *btn_refresh,
                   set<int> *ii,
                   string pn) {
		window = w;
		ivResults = iv;
		btnPrev = bp;
		btnNext = bn;
		
		swTree = sw_tree;
		swIcon = sw_icon;
		spCenter = sp_center;
		vbCenter = vb_center;
		hbResultsError = hb_results_error;
		
		btnUp = btn_up;
		rbActors = rb_actors;
		rbPlay = rb_play;
		rbDownload = rb_download;
		btnRefresh = btn_refresh;
		
		imageIndexes = ii;
		progName = pn;
		
		// GThreadPool for new results
	    resultsNewThreadPool = g_thread_pool_new(ResultsHistory::resultsNewTask,
	                                   this,
	                                   1, // Run one thread at the time
	                                   FALSE,
	                                   NULL);
	                                   
	    // GThreadPool for results pages
	    resultsAppendThreadPool = g_thread_pool_new(ResultsHistory::resultsAppendTask,
	                                   this,
	                                   1, // Run one thread at the time
	                                   FALSE,
	                                   NULL);
	}
	
	void showSpCenter(bool isPage) {
		gtk_widget_set_visible(swTree, FALSE);
		// Show and hide of ivResults depends on isPage
		gtk_widget_set_visible(swIcon, isPage);
		// Change packing params of spCenter
		gtk_box_set_child_packing(
		    GTK_BOX(vbCenter),
		    spCenter,
		    !isPage,
		    FALSE,
		    1,
		    GTK_PACK_START);
		gtk_widget_set_visible(spCenter, TRUE);
		gtk_widget_set_visible(hbResultsError, FALSE);
		gtk_spinner_start(GTK_SPINNER(spCenter));
	}
	
	void showResultsRepeat(bool isPage) {
		gtk_widget_set_visible(swTree, FALSE);
		// Show and hide of ivResults depends on isPage
		gtk_widget_set_visible(swIcon, isPage);
		// Change packing params of spCenter
		gtk_box_set_child_packing(
		    GTK_BOX(vbCenter),
		    hbResultsError,
		    !isPage,
		    FALSE,
		    1,
		    GTK_PACK_START);
		gtk_widget_set_visible(spCenter, FALSE);
		gtk_spinner_stop(GTK_SPINNER(spCenter));
		gtk_widget_set_visible(hbResultsError, TRUE);
	}
	
	void switchToIconView() {
		gtk_widget_set_visible(swTree, FALSE);
		gtk_widget_set_visible(swIcon, TRUE);
		gtk_widget_set_visible(spCenter, FALSE);
		gtk_widget_set_visible(hbResultsError, FALSE);
		gtk_spinner_stop(GTK_SPINNER(spCenter));
	}
	
	void setSensitiveItemsResults() {
		gtk_widget_set_sensitive(GTK_WIDGET(btnUp), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(btnRefresh), TRUE);
		gtk_widget_set_sensitive(GTK_WIDGET(rbPlay), TRUE);
	    gtk_widget_set_sensitive(GTK_WIDGET(rbDownload), TRUE);
	    gtk_widget_set_sensitive(GTK_WIDGET(rbActors), TRUE);
	}
	
	void disableAllItems() {
		gtk_widget_set_sensitive(GTK_WIDGET(btnRefresh), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(btnUp), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(btnPrev), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(btnNext), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(rbActors), FALSE);
	    gtk_widget_set_sensitive(GTK_WIDGET(rbPlay), FALSE);
	    gtk_widget_set_sensitive(GTK_WIDGET(rbDownload), FALSE);
	}

    void switchToTreeView() {
		gtk_widget_set_visible(swTree, TRUE);
		gtk_widget_set_visible(swIcon, FALSE);
		gtk_widget_set_visible(spCenter, FALSE);
		gtk_widget_set_visible(hbResultsError, FALSE);
		gtk_spinner_stop(GTK_SPINNER(spCenter));
	}
	
	void setSensitiveItemsPlaylists() {
		gtk_widget_set_sensitive(GTK_WIDGET(btnRefresh), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(btnUp), TRUE);
		gtk_widget_set_sensitive(GTK_WIDGET(btnPrev), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(btnNext), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(rbPlay), TRUE);
	    gtk_widget_set_sensitive(GTK_WIDGET(rbDownload), TRUE);
	    gtk_widget_set_sensitive(GTK_WIDGET(rbActors), FALSE);
	}
	
	void displayPlaylists() {
		switchToTreeView();
		setSensitiveItemsPlaylists();
	}
	
	void btnUpClicked() {
		switchToIconView();
		updateTitle();
		updatePrevNextButtons();
		setSensitiveItemsResults();
	}
	
	void btnPrevClicked() {
			// If results repeat button not displayed
		if(!gtk_widget_get_visible(hbResultsError)) {
			// Save current results to forwardResultsStack
			saveToForwardStack();
		}else {
			switchToIconView();
		}
		// Display top results from backResultsStack
		restoreFromBackStack();
		setSensitiveItemsResults();	
	}
	
	void btnNextClicked() {
		// If results repeat button not displayed
		if(!gtk_widget_get_visible(hbResultsError)) {
			// Save current results to backResultsStack
			saveToBackStack();
		}else {
			// If repeat button displayed
		    switchToIconView();
		}
	    // Display top result from forwardResultsStack
	    restoreFromForwardStack();
	    setSensitiveItemsResults();
	}
	
	string onPreExecuteNew() {
		// Display spinner for new results
	    showSpCenter(FALSE);
	    return results.getUrl();
	}
	
	string onPreExecuteAppend() {
		// Display spinner at the bottom of list
	    showSpCenter(TRUE);
	    return results.getNextLink();
	}
	
	void onPostExecuteNew(string &page) {
		if(!page.empty()) {
			//TODO: maybe I need to clear it while saving....
			// clear forward results stack on fetching new results
		    clearForwardResultsStack();
			removeBackStackDuplicate();
			
			// Scroll to the top of the list
		    GtkTreePath *path = gtk_tree_path_new_first();
		    gtk_icon_view_scroll_to_path(GTK_ICON_VIEW(ivResults), path, FALSE, 0, 0);
		    
		    // Clear results links set if not paging
		    clearThreadsLinks();
			createNewModel();
			switchToIconView();
			results.show(page);
			
			setSensitiveItemsResults();
		}else {
			switchToIconView(); // TODO: why is this here?
			showResultsRepeat(FALSE);
			setError(TRUE);
		}
		
		if(isRefresh()) {
			setRefresh(FALSE);
		}
	}
	
	void onPostExecuteAppend(string &page) {
		if(!page.empty()) {
			results.show(page);
			switchToIconView();
		}else { // error
			if(threadLinksContainNextLink()) {
				eraseNextLinkFromThreadLinks();
			}
			switchToIconView(); //TODO: why is this here?
			showResultsRepeat(TRUE);
			setError(TRUE);
		}
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
	
	void updatePrevNextButtons() {
		if(forwardResultsStack.empty()) {
			gtk_widget_set_sensitive(GTK_WIDGET(btnNext), FALSE);
		}else {
			gtk_widget_set_sensitive(GTK_WIDGET(btnNext), TRUE);
		}
		
		if(backResultsStack.empty()) {
			gtk_widget_set_sensitive(GTK_WIDGET(btnPrev), FALSE);
		}else {
			gtk_widget_set_sensitive(GTK_WIDGET(btnPrev), TRUE);
		}
	}
	
	void newThread() {
		// New images for new indexes will be downloaded
	    imageIndexes->clear();
		g_thread_pool_push(resultsNewThreadPool, (gpointer)1, NULL);
	}
	
	void appendThread() {
		if(!getNextLink().empty()) {
			// Search for the same link only once if it's not saved in set.
			if(!threadLinksContainNextLink()) {
				insertNextLinkToThreadLinks();
				g_thread_pool_push(resultsAppendThreadPool, (gpointer)1, NULL);
			}
		}
	}
	
	static void resultsNewTask(gpointer arg, gpointer arg1) {
		// On pre execute
		gdk_threads_enter();
		ResultsHistory *resultsHistory = (ResultsHistory*)arg1;
	    string link = resultsHistory->onPreExecuteNew();
	    gdk_threads_leave();
	    // async part
		string page = HtmlString::getResultsPage(link);
	    gdk_threads_enter();
	    resultsHistory->onPostExecuteNew(page);
		gdk_threads_leave();
	}
	
	static void resultsAppendTask(gpointer arg, gpointer arg1) {
		ResultsHistory *resultsHistory = (ResultsHistory *)arg1;
		// On pre execute
		gdk_threads_enter();
		string link = resultsHistory->onPreExecuteAppend();
		gdk_threads_leave();
		// async part
		string page = HtmlString::getResultsPage(link);
		// On post execute
		gdk_threads_enter();
		resultsHistory->onPostExecuteAppend(page);
		gdk_threads_leave();
	}
};
