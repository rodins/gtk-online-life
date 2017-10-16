// ResultsHistory.hpp
#include "Playlists.hpp"
#include "ActorsHistory.hpp"
#include "ErrorType.hpp"

class ResultsHistory {
    Results *results;
    Results *resultsAppendError;
    Results *curlArg;
    
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
    
    ErrorType error;
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
        
        error = NONE_ERROR;
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
		// Save current results to forwardResultsStack
		saveToForwardStack();
		// Display top results from backResultsStack
		restoreFromBackStack();
		setSensitiveItemsResults();
		
		// New images for new indexes will be downloaded
	    imageIndexes->clear();		
	}
	
	void btnNextClicked() {
		// Save current results to backResultsStack
		saveToBackStack();
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
		switch(error) {
			case RESULTS_NEW_ERROR:
			    newThread();
			break;
			case RESULTS_APPEND_ERROR:
			    appendThreadOnError();
			break;
			case PLAYLISTS_ERROR:
		        newThreadPlaylist();
			break;
			case NONE_ERROR:
			    // Nothing to do. Do not want compiler warning.
			break;
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
		results = new Results(title, url, imagesCache, ivResults);
		updateTitle();
		newThread();
	}
	
	void newThreadSearch(string title, string base_url) {
		saveToBackStack();
		results = new Results(title, base_url, imagesCache, ivResults);
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
	
	void parser(Results *res, int count, string div, set<string> &titles) {
		// Prepare to show results when first result item comes
		if(count == 0) {
			if(res->isRefresh()) {
				res->clearModel();
			}
			switchToIconView();
			setSensitiveItemsResults();
		}
		
		res->parser(div, titles);
	}
	
	private:
	
	static void resultsNewTask(gpointer arg, gpointer arg1) {
		// On pre execute
		gdk_threads_enter();
		ResultsHistory *resultsHistory = (ResultsHistory*)arg1;
		// Display spinner for new results
	    resultsHistory->showSpCenter(FALSE);
	    
	    // Scroll to the top of the list
	    GtkTreePath *path = gtk_tree_path_new_first();
	    gtk_icon_view_scroll_to_path(GTK_ICON_VIEW(resultsHistory->ivResults),
	                                 path, 
	                                 FALSE, 
	                                 0, 
	                                 0);
	    
	    gdk_threads_leave();
	    // async part
		resultsHistory->getResultsFromNet(resultsHistory->results->getUrl(), 
		                                  resultsHistory->results);
	    gdk_threads_enter();
	    resultsHistory->onPostExecuteNew();
		gdk_threads_leave();
	}
	
	static void resultsAppendTask(gpointer arg, gpointer arg1) {
		Results *resultsAppend = (Results*) arg;
		ResultsHistory *resultsHistory = (ResultsHistory *)arg1;
		// On pre execute
		gdk_threads_enter();
		// Display spinner at the bottom of list
		resultsHistory->showSpCenter(TRUE);;
		gdk_threads_leave();
		// async part
		resultsHistory->getResultsFromNet(resultsAppend->getNextLink(),
		                                  resultsAppend);
		// On post execute
		gdk_threads_enter();
		resultsHistory->onPostExecuteAppend(resultsAppend);
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
					resultsHistory->error = PLAYLISTS_ERROR;
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
					    resultsHistory->error = PLAYLISTS_ERROR;
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
		if(system("which totem") == 0) {
			return "totem ";
		}
		if(system("which mpv") == 0) {
			return "mpv ";
		}
		if(system("which mplayer") == 0) {
			return "mplayer -cache 2048 ";
		}
		return "";
	}
	
	string detectTerminal() {
		// Not tested!
		if(system("which Terminal") == 0) {
			return "Terminal -e ";
		}
		if(system("which urxvt") == 0) {
			return "urxvt -e ";
		}
		if(system("which xterm") == 0) {
			return "xterm -e ";
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
	
	void onPostExecuteNew() {
		if(!results->isEmpty()) {
			//TODO: maybe I need to clear it while saving....
			// clear forward results stack on fetching new results
		    clearForwardResultsStack();
			removeBackStackDuplicate();
			
		    // Clear results links set if not paging
		    resultsThreadsLinks.clear();
		}else {
			switchToIconView(); // TODO: why is this here?
			showResultsRepeat(FALSE);
			error = RESULTS_NEW_ERROR;
		}
		
		if(results->isRefresh()) {
			results->setRefresh(FALSE);
		}
	}
	
	void onPostExecuteAppend(Results *resultsAppend) {
		if(resultsAppend->isEmpty()) { // error
			if(threadLinksContainNextLink()) {
				resultsThreadsLinks.erase(resultsAppend->getNextLink());
			}
			switchToIconView(); //TODO: why is this here?
			showResultsRepeat(TRUE);
			error = RESULTS_APPEND_ERROR;
			resultsAppendError = resultsAppend;
		}
	}
	
	void clearForwardResultsStack() {
		// Do not clear if refresh button clicked
		if(!results->isRefresh()) {
			// Free saved results first
			for(unsigned i = 0; i < forwardResultsStack.size(); i++) {
				g_free(forwardResultsStack[i]);
			}
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
		if(results != NULL) {
			if(!results->isEmpty()) {
				saveResultsPostion();
		        backResultsStack.push_back(results);
		        // Set tooltip with results title
		        gtk_tool_item_set_tooltip_text(btnPrev, 
		                                       results->getTitle().c_str());
		        gtk_widget_set_sensitive(GTK_WIDGET(btnPrev), TRUE);
			}else {
				g_free(results);
				results = NULL;
			}
		}
	}
	
	void saveToForwardStack() {
		if(!results->isEmpty()) {
			saveResultsPostion();
			forwardResultsStack.push_back(results); 
			// Set tooltip with results title
	        gtk_tool_item_set_tooltip_text(btnNext, 
	                                       results->getTitle().c_str());
	        gtk_widget_set_sensitive(GTK_WIDGET(btnNext), TRUE);
		}else {
			g_free(results);
			results = NULL;
		}
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
	    switchToIconView();
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
	    switchToIconView();
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
	    error = NONE_ERROR;
	    resultsAppendError = NULL;
		g_thread_pool_push(resultsNewThreadPool, (gpointer)1, NULL);
	}
	
	void newThreadPlaylist() {
		g_thread_pool_push(playlistsThreadPool, (gpointer)"", NULL);
	}
	
	void appendThreadOnError() {
		g_thread_pool_push(resultsAppendThreadPool,
		                   (gpointer)resultsAppendError,
		                   NULL);
	}
	
	static int results_writer(char *data, size_t size, size_t nmemb,
	                      ResultsHistory *resultsHistory)
	{
	    if (resultsHistory == NULL)
	       return 0;
	    static int count = 0;    
	    static bool isBegin = FALSE;
	    static bool pagerBeginFound = FALSE;
	    static string partial_item;
	    static set<string> titles;
	    //cout << "DATA: " << data << endl;
	    string strData(data);
	    string itemBegin = "tom-pos"; // same as line below, but shorter
	    string itemEnd = "</a>";
	    // Find begining
	    //size_t begin = strData.find("<div class=\"custom-poster\"");
	    size_t item_begin;
	    size_t item_end;
	    size_t pager_begin = strData.find("class=\"navigation\"");
	    size_t pager_end = strData.find("</div>", pager_begin+1);
	    
	    // Find end
	    string strEnd("</table>");
	    size_t end = strData.find(strEnd);
	    
	    // Find only end part of item
	    if(isBegin) {
			item_end = strData.find(itemEnd);
			if(item_end != string::npos) {
				string item_end_part = strData.substr(0, item_end + itemEnd.size());
				isBegin = FALSE;
				partial_item += item_end_part;
				//cout << "PARTIAL ITEM: " << partial_item << endl;
				gdk_threads_enter();
				resultsHistory->parser(resultsHistory->curlArg,
				                       count,
				                       partial_item, 
				                       titles);
				gdk_threads_leave();
				count++;
				item_begin = strData.find(itemBegin, item_end);
			    item_end = strData.find(itemEnd, item_begin);
			}
		}else {
			item_begin = strData.find(itemBegin);
			item_end = strData.find(itemEnd, item_begin);
		}
	    
	    // Find whole item
	    while(item_begin != string::npos && item_end != string::npos && item_begin < item_end) {
			string whole_item = strData.substr(item_begin, item_end - item_begin + itemEnd.size());
			//cout << "WHOLE ITEM: " << whole_item << endl;
			gdk_threads_enter();
			// Switch to icon view when first result item comes
			resultsHistory->parser(resultsHistory->curlArg,
			                       count,
			                       whole_item, 
			                       titles);
			gdk_threads_leave();
			count++;
			item_begin = strData.find(itemBegin, item_end);
			item_end = strData.find(itemEnd, item_begin);
		}
		
		// Find only begining part of item
		if(item_begin != string::npos && item_end == string::npos) {
			string item_begin_part = strData.substr(item_begin);
			//cout << "BEGIN PART: " << item_begin_part << endl;
			isBegin = TRUE;
			partial_item = item_begin_part;
		}
		
		static string partial_pager;
		
		// Detect pager
		// TODO: fix pager for search (find begin and end separately);
		if(pager_end != string::npos && pager_begin != string::npos) {
			size_t pager_length = pager_end - pager_begin;
			string pager = strData.substr(pager_begin+2, pager_length-2);
			cout << "Whole pager: " << pager << endl;
			resultsHistory->curlArg->parse_pager(pager);
		}else if(pager_begin != string::npos) {
			pagerBeginFound = TRUE;
			partial_pager = strData.substr(pager_begin);
		}else if(pager_end != string::npos && pagerBeginFound) {
			pagerBeginFound = FALSE;
			partial_pager += strData.substr(0, pager_end);
			cout << "Partial pager: " << partial_pager << endl;
			resultsHistory->curlArg->parse_pager(partial_pager);
		}
	    
		// Detect end
		if(end != string::npos && count > 0) {
			count = 0;
			titles.clear();
			return CURL_READFUNC_ABORT; 
		}
	
	    return size*nmemb;
	}
	
	void getResultsFromNet(string url, Results *res) {
		CURL *curl_handle;
		curlArg = res;		
		/* init the curl session */
		curl_handle = curl_easy_init();		
		if(curl_handle) {
			/* remove crash bug */
			curl_easy_setopt(curl_handle, CURLOPT_NOSIGNAL, 1);		    
		    /* set url to get here */
			curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());			
			/* send all data to this function */			
			curl_easy_setopt(curl_handle, 
			                 CURLOPT_WRITEFUNCTION, 
			                 ResultsHistory::results_writer);
			curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);			
			curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, this);			
			/* get it */
			curl_easy_perform(curl_handle);
			/* cleanup curl stuff */
			curl_easy_cleanup(curl_handle);
		}
	}
};
