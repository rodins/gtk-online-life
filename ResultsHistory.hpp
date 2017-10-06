// ResultsHistory.hpp
#include "Playlists.hpp"
#include "ActorsHistory.hpp"

class ResultsHistory {
    Results *results;
    Playlists *playlists;
        
    vector<Results*> backResultsStack, forwardResultsStack;
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
    GThreadPool *playlistsThreadPool;
    
    ActorsHistory *actorsHistory;
    map<string, GdkPixbuf*> *imagesCache;
    public:
    
    ResultsHistory(GtkWidget *w,
                   GtkWidget *iv,
                   GtkWidget *tvPlaylists,
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
                   map<string, GdkPixbuf*> *cache,
                   ActorsHistory *ah,
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
		imagesCache = cache;
		results = NULL;
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
	                                   
	    // GThreadPool for playlists
	    playlistsThreadPool = g_thread_pool_new(ResultsHistory::playlistsTask,
	                                   this,
	                                   1, // Run one thread at the time
	                                   FALSE,
	                                   NULL);
	    playlists = new Playlists(
	        gtk_tree_view_get_model(GTK_TREE_VIEW(tvPlaylists))
        );
        
        actorsHistory = ah;
	}
	
	~ResultsHistory(){
		g_free(playlists);
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
		
		// New images for new indexes will be downloaded
	    imageIndexes->clear();		
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
	    
	    // New images for new indexes will be downloaded
	    imageIndexes->clear();	
	}
	
	void btnRefreshClicked() {
		results->setRefresh(TRUE);
	    newThread();
	}
	
	void btnResultsRepeatClicked() {
		if(results->isError()) {
			// Update results
		    newThread();
		    results->setError(FALSE);
		}else {
			// Update playlists
		    newThreadPlaylist();
		}
	}
	
	void onClick(string resultTitle, string href) {
		if(gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(rbActors))){
			// Fetch actors 
			actorsHistory->newThread(resultTitle, href);
		}else {
			// Fetch playlists/playItem
			string title = progName + " - " + resultTitle;
		    gtk_window_set_title(GTK_WINDOW(window), title.c_str());
		    g_thread_pool_push(playlistsThreadPool, 
		                       (gpointer)href.c_str(), 
		                       NULL);
		}
	}
	
	void newThread(string title, string url) {
		saveToBackStack();
		results = new Results(title, url, imagesCache);
		updateTitle();
		newThread();
	}
	
	void newThreadSearch(string title, string base_url) {
		saveToBackStack();
		results = new Results(title, base_url, imagesCache);
		updateTitle();
		newThread();
	}
	
	void appendThread() {
		if(!results->getNextLink().empty()) {
			// Search for the same link only once if it's not saved in set.
			if(!threadLinksContainNextLink()) {
				resultsThreadsLinks.insert(results->getNextLink());
				g_thread_pool_push(resultsAppendThreadPool, (gpointer)results, NULL);
			}
		}
	}
	
	//TODO: remove code repeat
	void playOrDownload(string file, string download) {
		if(gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(rbDownload))){
		    string command = detectTerminal() + "wget -P ~/Download -c " + download + " &";
	        system(command.c_str());
		}else if(gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(rbPlay))) {
		    string command = detectPlayer() + file + " &";
	        system(command.c_str());
		}
	}
	
	private:
	
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
		Results *resultsAppend = (Results*) arg;
		ResultsHistory *resultsHistory = (ResultsHistory *)arg1;
		// On pre execute
		gdk_threads_enter();
		resultsHistory->onPreExecuteAppend();
		gdk_threads_leave();
		// async part
		string page = HtmlString::getResultsPage(resultsAppend->getNextLink());
		// On post execute
		gdk_threads_enter();
		resultsHistory->onPostExecuteAppend(page, resultsAppend);
		gdk_threads_leave();
	}
	
	static void playlistsTask(gpointer args, gpointer args2) {
		ResultsHistory *resultsHistory = (ResultsHistory *)args2;
		Playlists *playlists = resultsHistory->getPlaylists();
		playlists->setUrl(args);
		string href = playlists->getUrl();
		string id = playlists->getHrefId();
		if(!id.empty()) {
			string url = "http://dterod.com/js.php?id=" + id;
			string referer = "http://dterod.com/player.php?newsid=" + id;
			// On pre execute
			gdk_threads_enter();
			// Show spinner fullscreen
			resultsHistory->showSpCenter(FALSE);
			gdk_threads_leave();
			
			string js = HtmlString::getPage(url, referer);
			string playlist_link = playlists->get_txt_link(js);
			if(!playlist_link.empty()) { // Playlists found
				string json = HtmlString::getPage(playlist_link);
				gdk_threads_enter();
				playlists->parse(json);
				if(playlists->getCount() > 0) {
					resultsHistory->displayPlaylists();
				}else {
					resultsHistory->showResultsRepeat(FALSE);
					resultsHistory->setSensitiveItemsPlaylists();
				}
				gdk_threads_leave();
			}else { //PlayItem found or nothing found
				gdk_threads_enter();
				PlayItem playItem = playlists->parse_play_item(js);
				if(!playItem.comment.empty()) { // PlayItem found
				    // get results list back
				    resultsHistory->switchToIconView();
				    resultsHistory->updateTitle();
					resultsHistory->processPlayItem(playItem); 
				}else {
					if(resultsHistory->results->getTitle().find("Трейлеры") != string::npos) {
						gdk_threads_leave();
						// Searching for alternative trailers links
			            string infoHtml = HtmlString::getPage(href, referer);
			            string trailerId = playlists->getTrailerId(infoHtml); 
			            url = "http://dterod.com/js.php?id=" + trailerId + "&trailer=1";
			            referer = "http://dterod.com/player.php?trailer_id=" + trailerId;
			            string json = HtmlString::getPage(url, referer);
						gdk_threads_enter();
						// get results list back
				        resultsHistory->switchToIconView();
				        resultsHistory->updateTitle();
						resultsHistory->processPlayItem(playlists->parse_play_item(json)); 
					}else {
						resultsHistory->showResultsRepeat(FALSE);
					    resultsHistory->setSensitiveItemsPlaylists();
					}
				}
				gdk_threads_leave();
			}
		}
	}
	
	Playlists* getPlaylists() {
		return playlists;
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
	
	string detectPlayer() {
		// TODO: add other players
		// TODO: add selection of players if few is installed
		if(system("which mplayer") == 0) {
			return "mplayer -cache 2048 ";
		}
		if(system("which mpv") == 0) {
			return "mpv ";
		}
		if(system("which totem") == 0) {
			return "totem ";
		}
		return "";
	}
	
	string detectTerminal() {
		if(system("which xterm") == 0) {
			return "xterm -e ";
		}
		if(system("which urxvt") == 0) {
			return "urxvt -e ";
		}
		// Not tested!
		if(system("which Terminal") == 0) {
			return "Terminal -e ";
		}
		return "";
	}
	
	void processPlayItem(PlayItem item) {
		if(!item.comment.empty()) {
		    if(gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(rbDownload))){
			    string command = detectTerminal() + "wget -P ~/Download -c " + item.download + " &";
		        system(command.c_str());
			}else if(gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(rbPlay))) {
			    string command = detectPlayer() + item.file + " &";
		        system(command.c_str());
			}	
		}
	}
	
	void setSensitiveItemsResults() {
		gtk_widget_set_sensitive(GTK_WIDGET(btnUp), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(btnRefresh), TRUE);
		gtk_widget_set_sensitive(GTK_WIDGET(rbPlay), TRUE);
	    gtk_widget_set_sensitive(GTK_WIDGET(rbDownload), TRUE);
	    gtk_widget_set_sensitive(GTK_WIDGET(rbActors), TRUE);
	}
	
	void switchToTreeView() {
		gtk_widget_set_visible(swTree, TRUE);
		gtk_widget_set_visible(swIcon, FALSE);
		gtk_widget_set_visible(spCenter, FALSE);
		gtk_widget_set_visible(hbResultsError, FALSE);
		gtk_spinner_stop(GTK_SPINNER(spCenter));
	}
	
	string onPreExecuteNew() {
		// Display spinner for new results
	    showSpCenter(FALSE);
	    return results->getUrl();
	}
	
	void onPreExecuteAppend() {
		// Display spinner at the bottom of list
	    showSpCenter(TRUE);
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
		    resultsThreadsLinks.clear();
			results->createNewModel(ivResults);
			switchToIconView();
			results->show(page);
			
			setSensitiveItemsResults();
		}else {
			switchToIconView(); // TODO: why is this here?
			showResultsRepeat(FALSE);
			results->setError(TRUE);
		}
		
		if(results->isRefresh()) {
			results->setRefresh(FALSE);
		}
	}
	
	void onPostExecuteAppend(string &page, Results *resultsAppend) {
		if(!page.empty()) {
			resultsAppend->show(page);
			switchToIconView();
		}else { // error
			if(threadLinksContainNextLink()) {
				resultsThreadsLinks.erase(resultsAppend->getNextLink());
			}
			switchToIconView(); //TODO: why is this here?
			showResultsRepeat(TRUE);
			resultsAppend->setError(TRUE);
		}
	}
	
	void clearForwardResultsStack() {
		// Do not clear if refresh button clicked
		if(!results->isRefresh()) {
			forwardResultsStack.clear();
		    gtk_tool_item_set_tooltip_text(btnNext, 
		                                   "Move forward in history");
		    gtk_widget_set_sensitive(GTK_WIDGET(btnNext), FALSE);
		}
	}
	
	void removeBackStackDuplicate() {
		// Linear search for title
		int eraseIndex = -1;
		// If back stack has title, remove results with it.
		for(unsigned i = 0; i < backResultsStack.size(); i++) {
			if(backResultsStack[i]->getTitle() == results->getTitle()) {
				eraseIndex = i;
				break;
			}
		}
		
		if(eraseIndex != -1) {
			backResultsStack.erase(backResultsStack.begin() + eraseIndex);
		}
	}
	
	void saveResultsPostion() {
		GtkTreePath *path1, *path2;
		if(gtk_icon_view_get_visible_range(GTK_ICON_VIEW(ivResults), &path1, &path2)) {
			string index(gtk_tree_path_to_string(path1));
	        results->setIndex(index);
			gtk_tree_path_free(path1);
		    gtk_tree_path_free(path2);
		}
	}
	
	void saveToBackStack() {
		// Save position and copy to save variable
		if(results != NULL && !results->getTitle().empty()) {
			saveResultsPostion();
	        backResultsStack.push_back(results);
	        // Set tooltip with results title
	        gtk_tool_item_set_tooltip_text(btnPrev, 
	                                       results->getTitle().c_str());
	        gtk_widget_set_sensitive(GTK_WIDGET(btnPrev), TRUE);
		}
	}
	
	void saveToForwardStack() {
		saveResultsPostion();
		forwardResultsStack.push_back(results); 
		// Set tooltip with results title
        gtk_tool_item_set_tooltip_text(btnNext, 
                                       results->getTitle().c_str());
        gtk_widget_set_sensitive(GTK_WIDGET(btnNext), TRUE);
	}
	
	void savedRecovery() {
		// Clear results links set if not paging
	    // (do not allow next page thread to be called twice)
	    resultsThreadsLinks.clear(); 
	    
		// Update ivResults with history results
		results->setModel(ivResults);
		// Scroll to saved position after updating model
		string index = results->getIndex();
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
											   ->getTitle().c_str());
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
		                                       ->getTitle().c_str());                                   
		}else {
			// Set tooltip with results title
		    gtk_tool_item_set_tooltip_text(btnNext, "Move forward in history");
		    gtk_widget_set_sensitive(GTK_WIDGET(btnNext), FALSE);
		}
	    
	    savedRecovery();
	    updateTitle(); 
	}
	
	bool threadLinksContainNextLink() {
		return resultsThreadsLinks.count(results->getNextLink()) > 0;
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
	
	void updateTitle() {
		string title = progName + " - " + results->getTitle();
		gtk_window_set_title(GTK_WINDOW(window), title.c_str());
	}
	
	void newThread() {
		// New images for new indexes will be downloaded
	    imageIndexes->clear();
		g_thread_pool_push(resultsNewThreadPool, (gpointer)1, NULL);
	}
	
	void newThreadPlaylist() {
		g_thread_pool_push(playlistsThreadPool, (gpointer)"", NULL);
	}
};
