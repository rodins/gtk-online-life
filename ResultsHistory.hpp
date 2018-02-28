// ResultsHistory.hpp
#include "PlayItem.hpp"
#include "PlaylistsUtils.hpp"
#include "Playlists.hpp"
#include "ActorsHistory.hpp"
#include "ErrorType.hpp"

class ResultsHistory;

struct ResultsArgs {
	string title;
	string link;
	Results *results;
	ResultsHistory *resultsHistory;
	
	// For new task
	ResultsArgs(string title, string link, ResultsHistory *resultsHistory) {
		this->title = title;
		this->link = link;
		this->resultsHistory = resultsHistory;
		results = NULL;
	} 
	
	// For append task or refresh
	ResultsArgs(Results *results, ResultsHistory *resultsHistory) {
		this->title = results->getTitle();
		if(results->isRefresh()) {
			this->link = results->getUrl();
		}else {
			this->link = results->getNextLink();
		}
		this->results = results;
		this->resultsHistory = resultsHistory;
	}
};

class ResultsHistory {
    Results *results;
    Results *resultsAppendError;
    
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
    GtkToolItem *btnRefresh;
    
    set<int> *imageIndexes;
    
    GThreadPool *resultsNewThreadPool;
    GThreadPool *resultsAppendThreadPool;
    GThreadPool *listEpisodesThreadPool;
    
    ActorsHistory *actorsHistory;
    map<string, GdkPixbuf*> *imagesCache;
    
    ErrorType error;
    
    string repeatTitle, repeatLink;
    
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
	                                   
	    // GThreadPool for listEpisodes
	    listEpisodesThreadPool = g_thread_pool_new(ResultsHistory::listEpisodesTask,
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
		//removeBackStackDuplicate();
		// Save current results to forwardResultsStack
		saveToForwardStack();
		// Display top results from backResultsStack
		restoreFromBackStack();
		setSensitiveItemsResults();
		
		// New images for new indexes will be downloaded
	    imageIndexes->clear();		
	}
	
	void btnNextClicked() {
		//removeBackStackDuplicate();
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
		ResultsArgs *resultsArgs = new ResultsArgs(results, this);
	    newThread(resultsArgs);
	}
	
	void btnResultsRepeatClicked() {
		switch(error) {
			case RESULTS_NEW_ERROR:
			    newThread(repeatTitle, repeatLink);
			break;
			case RESULTS_APPEND_ERROR:
			    appendThreadOnError();
			break;
			case PLAYLISTS_ERROR:
		        btnListEpisodesClicked();
			break;
			case NONE_ERROR:
			    // Nothing to do. Do not want compiler warning.
			break;
		}
	}
	
	void newThread(string title, string url) {
		saveToBackStack();
		ResultsArgs *resultsArgs = new ResultsArgs(title, url, this);
		updateTitle(title);
		newThread(resultsArgs);
	}
	
	void newThreadSearch(string title, string base_url) {
		saveToBackStack();
		ResultsArgs *resultsArgs = new ResultsArgs(title, base_url, this);
		updateTitle(title);
		newThread(resultsArgs);
	}
	
	void appendThread() {
		if(!results->getNextLink().empty()) {
			// Search for the same link only once if it's not saved in set.
			if(resultsThreadsLinks.count(results->getNextLink()) == 0) {
				resultsThreadsLinks.insert(results->getNextLink());
				g_thread_pool_push(resultsAppendThreadPool, (gpointer)results, NULL);
			}
		}
	}
	
	void parser(ResultsArgs *resultsArgs, int count, string div, set<string> &titles) {
		// Prepare to show results when first result item comes
		if(count == 0) {
			switchToIconView();
			setSensitiveItemsResults();
			
			if(resultsArgs->results == NULL) {
				// Create new results object
				results = new Results(resultsArgs->title,
				                      resultsArgs->link,
				                      imagesCache, 
				                      ivResults);
				// Save link to results also in resultsArgs                      
				resultsArgs->results = results;
				scrollToTopOfList();
			}else if(resultsArgs->results->isRefresh()) {
				// Clear existing model on refresh
				resultsArgs->results->clearModel();	
				scrollToTopOfList();
			}
		}
		
		resultsArgs->results->parser(div, titles);
	}
	
	void btnListEpisodesClicked() {
		ListEpisodesArgs listEpisodesArgs = actorsHistory->getCurrentActorsListEpisodesArgs();
		ListEpisodesArgs *listEpisodesArgsPtr = new ListEpisodesArgs();
		listEpisodesArgsPtr->title = listEpisodesArgs.title;
		listEpisodesArgsPtr->playlist_link = listEpisodesArgs.playlist_link;
	    g_thread_pool_push(listEpisodesThreadPool, 
		                   (gpointer)listEpisodesArgsPtr, 
		                   NULL);
	}
	
	private:
	
	void scrollToTopOfList() {
		// Scroll to the top of the list on new results (not append to list)
	    GtkTreePath *path = gtk_tree_path_new_first();
	    gtk_icon_view_scroll_to_path(GTK_ICON_VIEW(ivResults),
	                                 path, 
	                                 FALSE, 
	                                 0, 
	                                 0);
	}
	
	static void resultsNewTask(gpointer arg, gpointer arg1) {
		ResultsArgs *resultsArgs = (ResultsArgs*)arg;
		// On pre execute
		gdk_threads_enter();
		ResultsHistory *resultsHistory = (ResultsHistory*)arg1;
		// Display spinner for new results
	    resultsHistory->showSpCenter(FALSE);
	    // Save title and link to be able to repeat the call 
	    resultsHistory->repeatTitle = resultsArgs->title;
	    resultsHistory->repeatLink = resultsArgs->link;
	    gdk_threads_leave();
	    // async part
		CURLcode res = resultsHistory->getResultsFromNet(resultsArgs);
		
		gdk_threads_enter();
		resultsHistory->onPostExecuteNew(res);                                 
		gdk_threads_leave();
	}
	
	static void resultsAppendTask(gpointer arg, gpointer arg1) {
		Results *resultsAppend = (Results*) arg;
		ResultsHistory *resultsHistory = (ResultsHistory *)arg1;
		ResultsArgs *resultsArgs = new ResultsArgs(resultsAppend, 
		                                           resultsHistory);
		// On pre execute
		gdk_threads_enter();
		// Display spinner at the bottom of list
		resultsHistory->showSpCenter(TRUE);;
		gdk_threads_leave();
		// async part
		CURLcode res = resultsHistory->getResultsFromNet(resultsArgs);
		// On post execute
		gdk_threads_enter();
		resultsHistory->onPostExecuteAppend(resultsAppend, res);
		gdk_threads_leave();
	}
	
	static void listEpisodesTask(gpointer args, gpointer args2) {
		ResultsHistory *resultsHistory = (ResultsHistory *)args2;
		Playlists *playlists = resultsHistory->getPlaylists();
		ListEpisodesArgs *listEpisodesArgs = (ListEpisodesArgs *)args;
		
		// On pre execute
		gdk_threads_enter();
		// Set title
		resultsHistory->updateTitle(listEpisodesArgs->title);
		// Show spinner fullscreen
		resultsHistory->showSpCenter(FALSE);
		gdk_threads_leave();
		
		string json = HtmlString::getPage(listEpisodesArgs->playlist_link);
		
		gdk_threads_enter();
		playlists->parse(json);
		if(playlists->getCount() > 0) {
			resultsHistory->displayPlaylists();
		}else {
			resultsHistory->showResultsRepeat(FALSE);
			resultsHistory->setSensitiveItemsPlaylists();
			resultsHistory->error = PLAYLISTS_ERROR;
		}
		g_free(listEpisodesArgs);
		gdk_threads_leave();
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
	
	void setSensitiveItemsResults() {
		gtk_widget_set_sensitive(GTK_WIDGET(btnUp), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(btnRefresh), TRUE);
	}
	
	void switchToTreeView() {
		gtk_widget_set_visible(swTree, TRUE);
		gtk_widget_set_visible(swIcon, FALSE);
		gtk_widget_set_visible(spCenter, FALSE);
		gtk_widget_set_visible(hbResultsError, FALSE);
		gtk_spinner_stop(GTK_SPINNER(spCenter));
	}
	
	void onPostExecuteNew(CURLcode res) {
		if(res == CURLE_OK) {
			if(gtk_widget_get_visible(spCenter)) {
				switchToIconView();
			    setSensitiveItemsResults();
			    // Create dialog for nothing found
			    GtkWidget* dialog = gtk_message_dialog_new(GTK_WINDOW(window),
			                                               GTK_DIALOG_MODAL,
			                                               GTK_MESSAGE_INFO,
			                                               GTK_BUTTONS_OK,
			                                               "Nothing found.");
			    gtk_dialog_run(GTK_DIALOG(dialog));
			    gtk_widget_destroy(dialog);
			    if(results != NULL) {
					updateTitle();
				}else {
					updateTitle("");
				}
			}else {
				//TODO: maybe I need to clear it while saving....
				// clear forward results stack on fetching new results
			    clearForwardResultsStack();
				removeBackStackDuplicate();
				
			    // Clear results links set if not paging
			    resultsThreadsLinks.clear();
			    
			    // Attempt to fix refresh on error problem
				if(results->isRefresh()) {
					results->setRefresh(FALSE);
				}
			}
		}else { //error
			showResultsRepeat(FALSE);
			error = RESULTS_NEW_ERROR;
		}
	}
	
	void onPostExecuteAppend(Results *resultsAppend, CURLcode res) {
		if(res != CURLE_OK) { // error
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
		
		updatePrevNextButtons();
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
		removeBackStackDuplicate();
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
		gtk_widget_set_sensitive(GTK_WIDGET(btnUp), results != NULL);
		gtk_widget_set_sensitive(GTK_WIDGET(btnPrev), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(btnNext), FALSE);
	}
	
	void displayPlaylists() {
		switchToTreeView();
		setSensitiveItemsPlaylists();
	}
	
	void updateTitle() {
		string title = progName + " - " + results->getTitle();
		gtk_window_set_title(GTK_WINDOW(window), title.c_str());
	}
	
	void updateTitle(string resultsTitle) {
		if(!resultsTitle.empty()) {
			string title = progName + " - " + resultsTitle;
		    gtk_window_set_title(GTK_WINDOW(window), title.c_str());
		}else {
			gtk_window_set_title(GTK_WINDOW(window), progName.c_str());
		}
	}
	
	void newThread(ResultsArgs *resultsArgs) {
		// New images for new indexes will be downloaded
	    imageIndexes->clear();
	    error = NONE_ERROR;
	    resultsAppendError = NULL;
		g_thread_pool_push(resultsNewThreadPool, (gpointer)resultsArgs, NULL);
	}
	
	void appendThreadOnError() {
		g_thread_pool_push(resultsAppendThreadPool,
		                   (gpointer)resultsAppendError,
		                   NULL);
	}
	
	static void find_item(ResultsArgs *resultsArgs, 
	                      int &count, 
	                      string &div, 
	                      set<string> &titles) {				  
		size_t item_begin = div.find("<div class=\"custom-poster\"");
		size_t item_end = div.find("</a>", item_begin+3);
		if(item_begin != string::npos && item_end != string::npos) {
			string item = div.substr(item_begin, item_end - item_begin + 4);
			gdk_threads_enter();
			resultsArgs->resultsHistory->parser(resultsArgs,
			                                    count,
			                                    item,
			                                    titles);
			count++;
			gdk_threads_leave();
		}
	}
	
	static void find_pager(Results *res, string &div) {
		size_t pager_begin = div.find("class=\"navigation\"");
	    size_t pager_end = div.find("</div>", pager_begin+1);
	    if(pager_begin != string::npos && pager_end != string::npos) {
			string pager = div.substr(pager_begin, pager_end - pager_begin + 6);
			res->parse_pager(pager);
		}
	}
	
	static int results_writer(char *data, size_t size, size_t nmemb,
	                      ResultsArgs *resultsArgs)
	{
	    if (resultsArgs == NULL)
	       return 0;
	    
	    static int count = 0;    
	    static set<string> titles;
	    
	    string strData(data);
	
	    // Find end
	    string strEnd("</table>");
	    size_t end = strData.find(strEnd);
	    
	    // Find div
	    static string partial_div;
	    static bool divBeginFound = FALSE;
	    
	    size_t starting_point = strData.find("tom-pos");
	    // If starting point is not found, don't parse divs
	    if(starting_point == string::npos && count == 0) {
			return size*nmemb;
		}
	    
	    size_t div_end_first = strData.find("</div>");
	    size_t div_begin = strData.find("<div");
	    size_t div_end = strData.find("</div>", div_begin+3);
	    
	    if(div_end_first != string::npos && divBeginFound) {
			divBeginFound = FALSE;
			partial_div += strData.substr(0, div_end_first+6);
			// On first item found clear found end
			if(count == 0) {
				end = string::npos;
			}
			find_item(resultsArgs,
			          count,
			          partial_div,
			          titles);
			find_pager(resultsArgs->results, partial_div);
		}
	    
	    while(div_begin != string::npos && div_end != string::npos) {
			string div = strData.substr(div_begin, div_end - div_begin + 6);
			// On first item found clear found end
			if(count == 0) {
				end = string::npos;
			}
			find_item(resultsArgs,
			          count,
			          div,
			          titles);
			find_pager(resultsArgs->results, div);
			div_begin = strData.find("<div", div_end+4);
	        div_end = strData.find("</div>", div_begin+3);
		}
		
		if(div_begin != string::npos) {
			divBeginFound = TRUE;
			partial_div = strData.substr(div_begin);
		}

		// Detect end
		if(end != string::npos && count > 0) {
			count = 0;
			titles.clear();
			//return CURL_READFUNC_ABORT; 
		}
	
	    return size*nmemb;
	}
	
	CURLcode getResultsFromNet(ResultsArgs *resultsArgs) {
		CURL *curl_handle;
	    
		CURLcode res;		
		/* init the curl session */
		curl_handle = curl_easy_init();		
		if(curl_handle) {
			/* remove crash bug */
			curl_easy_setopt(curl_handle, CURLOPT_NOSIGNAL, 1);		    
		    /* set url to get here */
			curl_easy_setopt(curl_handle, 
			                 CURLOPT_URL, 
			                 resultsArgs->link.c_str());			
			/* send all data to this function */		
			curl_easy_setopt(curl_handle, 
			                 CURLOPT_WRITEFUNCTION, 
			                 ResultsHistory::results_writer);
			curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);	
			curl_easy_setopt(curl_handle,
			                 CURLOPT_WRITEDATA, 
			                 resultsArgs);			
			/* get it */
			res = curl_easy_perform(curl_handle);

			/* cleanup curl stuff */
			curl_easy_cleanup(curl_handle);
			
		}
		g_free(resultsArgs);
		return res;
	}
};
