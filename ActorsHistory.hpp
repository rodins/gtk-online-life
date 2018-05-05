//ActorsHistory.hpp
#include "Actors.hpp"
#include "FileUtils.hpp"
#include "ResultsHistory.hpp"
#include "ProcessPlayItemDialog.hpp"

class ActorsHistory {
    Actors actors, prevActors;
    map <string, Actors> backActors;
    
    GtkWidget *window;
    GtkWidget *tvActors, *tvBackActors;
    GtkWidget *frRightBottom;
    GdkPixbuf *icon;
    GtkWidget *lbInfo;
    
    GtkWidget *frRightTop, *frInfo, *frActions;
    GtkWidget *spActors;
    GtkWidget *hbActorsError;
    GtkWidget *vbRight;
    
    GThreadPool *actorsThreadPool;
    GThreadPool *linksSizeThreadPool;
    GThreadPool *detectThreadPool;
    GThreadPool *constantsThreadPool;
    
    GtkWidget *spLinks;
    GtkWidget *btnLinksError;
    GtkWidget *btnGetLinks;
    GtkWidget *btnListEpisodes;
    GtkWidget *btnSave;
    GtkWidget *btnDelete;
    GtkToolItem *btnSavedItems;
    GtkToolItem *btnActors;
    
    GtkWidget *swTree, *swIcon;
    GtkWidget *spCenter;
    GtkWidget *hbResultsError;
    // TODO: make resultsHistory not pointer
    ResultsHistory *resultsHistory;
    string player;
    
    public:
    
    ActorsHistory(GtkWidget *window,
                  GtkWidget *a,
                  GtkWidget *pa,
                  GtkWidget* fr, 
                  GtkWidget* li,
                  GtkWidget *frt, 
                  GtkWidget *fri, 
                  GtkWidget *frActions,
                  GtkWidget *sa, 
                  GtkWidget *hba,
                  GtkWidget *vbr,
                  GtkWidget *spLinks,
				  GtkWidget *btnLinksError,
				  GtkWidget *btnGetLinks,
				  GtkWidget *btnListEpisodes,
				  GtkWidget *btnSave,
				  GtkWidget *btnDelete,
				  GtkToolItem *btnSavedItems,
				  GtkToolItem *btnActors,
				  ResultsHistory *resultsHistory) {
		this->window = window;			  
		tvActors = a;
		tvBackActors = pa;
		frRightBottom = fr;
		lbInfo = li;
		
		frRightTop = frt;
		frInfo = fri;
		this->frActions = frActions;
		spActors = sa;
		hbActorsError = hba;
		vbRight = vbr;
		
		this->spLinks = spLinks;
		this->btnLinksError = btnLinksError;
		this->btnGetLinks = btnGetLinks;
		this->btnListEpisodes = btnListEpisodes;
		
		this->btnSave = btnSave;
		this->btnDelete = btnDelete;
		this->btnSavedItems = btnSavedItems;
		this->btnActors = btnActors;
		this->resultsHistory = resultsHistory;
		
		icon = IconsFactory::getLinkIcon();
		
		// GThreadPool for actors
	    actorsThreadPool = g_thread_pool_new(ActorsHistory::actorsTask,
	                                   this,
	                                   1, // Run one thread at the time
	                                   FALSE,
	                                   NULL);
	                                   
	    // GThreadPool for detect
	    detectThreadPool = g_thread_pool_new(ActorsHistory::detectTask,
	                                   this,
	                                   1, // Run one thread at the time
	                                   FALSE,
	                                   NULL);
	                                   
	    // GThreadPool for constant links
	    constantsThreadPool = g_thread_pool_new(
	                                   ActorsHistory::constantsTask,
	                                   this,
	                                   1, // Run one thread at the time
	                                   FALSE,
	                                   NULL); 
	                                   
	    // GThreadPool for links sizes
	    linksSizeThreadPool = g_thread_pool_new(ActorsHistory::linksSizeTask,
	                                   this,
	                                   2, // Run two threads at the time
	                                   FALSE,
	                                   NULL);
	                                   
	    player = detectPlayer();
	} 
	
	GtkWidget* getWindow() {
		return window;
	}
	
    void newThread(string title, string href, GdkPixbuf *pixbuf) {
		// Save current actors to back actors map before getting new actors
		if(actors.isNetworkOk()) {
			if(backActors.count(actors.getTitle()) == 0) {
			    backActorsListAdd(actors.getTitle());	
			}
			backActors[actors.getTitle()] = actors;
		}
		
		actors.setTitle(title);
		actors.setUrl(href);
		actors.setPixbuf(pixbuf);
		actors.setNetworkOk(FALSE);
		showSaveOrDeleteButton();
		
		// If btnActors is active show tab and find actors and links
		if(gtk_toggle_tool_button_get_active(
		   GTK_TOGGLE_TOOL_BUTTON(btnActors))) {	   
			newActorsThread();
		}else { // Do not get actors, get only js based on constant links
		    // id should not disappear after exit from function
		    static string id;
		    id = PlaylistsUtils::get_href_id(href);
			g_thread_pool_push(constantsThreadPool,
			                  (gpointer)id.c_str(),
			                   NULL);
		}
	}
	
	void newActorsThread() {
		g_thread_pool_push(actorsThreadPool, (gpointer)1, NULL);
	}
	
	void detectThread() {
		g_thread_pool_push(detectThreadPool, (gpointer)1, NULL);
	}
    
	void changed(GtkTreeSelection *treeselection) {
		GtkTreeIter iter;
		GtkTreeModel *model;
		gchar *value;
		
		if (gtk_tree_selection_get_selected(treeselection, 
		                                    &model, 
		                                    &iter)) {
			gtk_tree_model_get(model, &iter, TITLE_COLUMN, &value,  -1);
			if(backActors.count(actors.getTitle()) == 0) {
				backActorsListAdd(actors.getTitle());
			}
			
			backActors[actors.getTitle()] = actors; // save prev actors
			actors = backActors[string(value)]; // set new actors
			updateActors();
			g_free(value);
	    }
	    
	    //Links button change
	    switch(actors.getLinksMode()) {
			case LINKS_MODE_MOVIE:
			    showGetLinksButton();
			break;
			case LINKS_MODE_SERIAL:
			    showListEpisodesButton();
			    resultsHistory->setListEpisodesArgs(
			                    actors.getListEpisodesArgs());
			break;
			case LINKS_MODE_HIDE:
			    hideAllLinksButtons();
			break;
			case LINKS_MODE_REFRESH:
			    showLinksErrorButton();
			break;
		}
		
		// Save or delete button change
		showSaveOrDeleteButton();
	}
	
	void btnActorsClicked() {
		gboolean isActive = gtk_toggle_tool_button_get_active(
		                    GTK_TOGGLE_TOOL_BUTTON(btnActors));
		if(isActive) {
			if(actors.isNetworkOk()) {
				gtk_widget_show(vbRight);
			}else if(!actors.getUrl().empty()) {
				newActorsThread();
			}
		}else {
			gtk_widget_hide(vbRight);
		} 
	}
	
	ListEpisodesArgs getCurrentActorsListEpisodesArgs() {
		return actors.getListEpisodesArgs();
	}
	
	void runPlayItemDialog(string js = "") {
		if(js == "") {
			js = actors.getJs();
		}
		runPlayItemDialog(
		   PlaylistsUtils::parse_play_item(js, FALSE));
	}
	
	void runPlayItemDialog(PlayItem pi) {
		// playItem address transfered to async task,
		// it shouldn't disappear when function ends
		static PlayItem playItem;
		playItem = pi;
		playItem.player = player;
		if(!playItem.comment.empty()) { // PlayItem found
			// Set title for trailers
			if(playItem.comment.size() == 1) {
				playItem.comment = actors.getTitle();
			}
			
			//get sizes first
			g_thread_pool_push(linksSizeThreadPool, 
		                       (gpointer)&playItem, 
		                       NULL);
		}
	}
	
	void btnGetLinksClicked() {
		runPlayItemDialog();
	}
	
	void btnLinksErrorClicked() {
		detectThread();
	}
	
	void btnSaveClicked() {
		FileUtils::writeToFile(actors.getTitle(), actors.getUrl());
		FileUtils::writeImageToFile(actors.getTitle(),
		                            actors.getPixbuf());
		showSaveOrDeleteButton(TRUE);
	}
	
	void btnDeleteClicked() {
		FileUtils::removeFile(actors.getTitle());
		FileUtils::removeImageFile(actors.getTitle());
		showSaveOrDeleteButton(TRUE);
	}
	
	void hideAllLinksButtons() {
		gtk_widget_hide(btnGetLinks);
		gtk_widget_hide(btnListEpisodes);
		gtk_widget_hide(btnLinksError);
		gtk_widget_hide(spLinks);
		gtk_spinner_stop(GTK_SPINNER(spLinks));
	}
	
	string getPlayer() {
		return player;
	}
	
	private:
	
	string detectPlayer() {
		if(system("which mpv") == 0) {
			return "mpv --cache=2048 ";
		}
		if(system("which mplayer") == 0) {
			return "mplayer -cache 2048 ";
		}
		return "";
	}
	
	void showSaveOrDeleteButton(bool isUpdate = FALSE) {
		if(FileUtils::isTitleSaved(actors.getTitle())) {
			gtk_widget_set_visible(btnSave, FALSE);
			gtk_widget_set_visible(btnDelete, TRUE);
		}else {
			gtk_widget_set_visible(btnSave, TRUE);
			gtk_widget_set_visible(btnDelete, FALSE);
		}
		
		if(isUpdate) {
			// Update file list
		    FileUtils::listSavedFiles(resultsHistory->getIvResults(),
		                              btnSavedItems);
		    gboolean isSavedItemsAvailable = gtk_widget_get_sensitive(
	                                         GTK_WIDGET(btnSavedItems));
	        resultsHistory->setSavedItemsAvailable(isSavedItemsAvailable);
		}	
	}
	
	// Info links frame functions
    void showSpLinks() {
		gtk_widget_hide(btnGetLinks);
		gtk_widget_hide(btnListEpisodes);
		gtk_widget_hide(btnLinksError);
		gtk_widget_show(spLinks);
		gtk_spinner_start(GTK_SPINNER(spLinks));
	}
	
	void showListEpisodesButton() {
		gtk_widget_hide(btnGetLinks);
		gtk_widget_show(btnListEpisodes);
		gtk_widget_hide(btnLinksError);
		gtk_widget_hide(spLinks);
		gtk_spinner_stop(GTK_SPINNER(spLinks));
	}
	
	void showGetLinksButton() {
		gtk_widget_show(btnGetLinks);
		gtk_widget_hide(btnListEpisodes);
		gtk_widget_hide(btnLinksError);
		gtk_widget_hide(spLinks);
		gtk_spinner_stop(GTK_SPINNER(spLinks));
	}
	
	void showLinksErrorButton() {
		gtk_widget_hide(btnGetLinks);
		gtk_widget_hide(btnListEpisodes);
		gtk_widget_show(btnLinksError);
		gtk_widget_hide(spLinks);
		gtk_spinner_stop(GTK_SPINNER(spLinks));
	}
	
	static void linksSizeTask(gpointer arg1, gpointer arg2) {
		ActorsHistory *actorsHistory = (ActorsHistory *)arg2;
		PlayItem *playItem = (PlayItem*)arg1;
		string sizeFile = HtmlString::getSizeOfLink(playItem->file);
		string sizeDownload = HtmlString::getSizeOfLink(playItem->download);
		gdk_threads_enter();
		if(sizeFile.empty() && sizeDownload.empty()) {
			actorsHistory->runErrorDialog();
		}else {
		    ProcessPlayItemDialog ppid(actorsHistory->window, 
		                               *playItem, 
		                               sizeFile,
		                               sizeDownload);
		}
		gdk_threads_leave();
	}
	
	// Use constant links and id to get js
	static void constantsTask(gpointer arg, gpointer arg2) {
		ActorsHistory *actorsHistory = (ActorsHistory *)arg2;
		string id((const char*)arg);
		
		string url = "http://play.cidwo.com/js.php?id=" + id;
		string referer = "http://play.cidwo.com/player.php?newsid=" + id;
		
		// On pre execute
		gdk_threads_enter();
		// Show links spinner
		actorsHistory->resultsHistory->showSpCenter(FALSE);
		gdk_threads_leave();
		// Async part	
		string js = HtmlString::getPage(url, referer);
		gdk_threads_enter();
		if(js.length() > 1000) { // Serial
			string playlist_link = PlaylistsUtils::get_txt_link(js);
			if(!playlist_link.empty()) { // Playlists found
				ListEpisodesArgs listEpisodesArgs;
				listEpisodesArgs.title = actorsHistory->actors.getTitle();
				listEpisodesArgs.playlist_link = playlist_link;
				actorsHistory->resultsHistory->setListEpisodesArgs(listEpisodesArgs);
				actorsHistory->resultsHistory->btnListEpisodesClicked();
            }
		}else if(js.length() > 500) { // Movie
			actorsHistory->resultsHistory->showResults();
		    actorsHistory->runPlayItemDialog(js);
		}else{ // Error
			actorsHistory->resultsHistory->showResults();
			actorsHistory->runErrorDialog();
		}
		gdk_threads_leave();
	}
	
	// Parse info page, get player link to get js
	static void detectTask(gpointer arg, gpointer arg2) {
		ActorsHistory *actorsHistory = (ActorsHistory *)arg2;
	    // On pre execute
		gdk_threads_enter();
		string href = actorsHistory->actors.getUrl();
		// Show links spinner
		actorsHistory->showSpLinks();
		gdk_threads_leave();	
		string referer = actorsHistory->actors.getPlayerUrl();
		string player = HtmlString::getPage(referer);
		string url = PlaylistsUtils::parsePlayerForUrl(player);
		string js = HtmlString::getPage(url, referer);
		
		gdk_threads_enter();
		if(js.length() > 1000) { // Serial
			string playlist_link = PlaylistsUtils::get_txt_link(js);
			if(!playlist_link.empty()) { // Playlists found
				actorsHistory->showListEpisodesButton();
				ListEpisodesArgs listEpisodesArgs;
				listEpisodesArgs.title = actorsHistory->actors.getTitle();
				listEpisodesArgs.playlist_link = playlist_link;
				actorsHistory->actors.setListEpisodesArgs(listEpisodesArgs);
				actorsHistory->actors.setLinksMode(LINKS_MODE_SERIAL);
				actorsHistory->resultsHistory->
				               setListEpisodesArgs(listEpisodesArgs);
			}
		}else if(js.length() > 500) { // Movie
				actorsHistory->showGetLinksButton();
				actorsHistory->actors.setJs(js);
				actorsHistory->actors.setLinksMode(LINKS_MODE_MOVIE);
		}else {
			actorsHistory->showLinksErrorButton();
			actorsHistory->actors.setLinksMode(LINKS_MODE_REFRESH);
		}
		gdk_threads_leave();
	}
	
	static void actorsTask(gpointer args, gpointer args2) {
		ActorsHistory *actorsHistory = (ActorsHistory*)args2;
		// On pre execute
		gdk_threads_enter();
		string link = actorsHistory->onPreExecute();
		gdk_threads_leave();
		string page = HtmlString::getActorsPage(link);
		// On post execute
		gdk_threads_enter();
		actorsHistory->onPostExecute(page);
		gdk_threads_leave();
	}
	
	void showSpActors() {
		gtk_widget_show(frInfo);
		gtk_widget_hide(frRightTop);
		gtk_widget_hide(hbActorsError);
		gtk_widget_show(spActors);
		gtk_spinner_start(GTK_SPINNER(spActors));
	    gtk_widget_show(frActions);
	}
	
	void showActors() {
		gtk_widget_show(frInfo);
		gtk_widget_show(frRightTop);
		gtk_widget_hide(hbActorsError);
		gtk_widget_hide(spActors);
		gtk_spinner_stop(GTK_SPINNER(spActors));
		gtk_widget_show(frActions);
	}
	
	void showActorsError() {
		gtk_widget_show(frInfo);
		gtk_widget_hide(frRightTop);
		gtk_widget_show(hbActorsError);
		gtk_widget_hide(spActors);
		gtk_spinner_stop(GTK_SPINNER(spActors));
		gtk_widget_show(frActions);
	}
	
	void runErrorDialog() {
		string message("No links found. ");
		if(!gtk_toggle_tool_button_get_active(
		   GTK_TOGGLE_TOOL_BUTTON(btnActors))) {
			   message += 
			   "Try to press info button on the toolbar and try again."; 
		}
		GtkWidget *dialog = gtk_message_dialog_new(
		              GTK_WINDOW(window),
		              GTK_DIALOG_DESTROY_WITH_PARENT,
		              GTK_MESSAGE_INFO,
		              GTK_BUTTONS_OK,
		              "%s", message.c_str());
		gtk_window_set_title(GTK_WINDOW(dialog), "Error");
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
	}
	
	void updateActors() {
		gtk_label_set_text(GTK_LABEL(lbInfo), actors.getInfo().c_str());
		GtkTreeModel *model;
		model = actors.getModel();
	    gtk_tree_view_set_model(GTK_TREE_VIEW(tvActors), model);
		//g_object_unref(model); // do not free, used for actors history
	}

	void backActorsListAdd(string title) {
		gtk_widget_set_visible(frRightBottom, TRUE);
		
		GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(
		                      GTK_TREE_VIEW(tvBackActors)));
		GtkTreeIter iter; // TODO: make iter class member
		
		gtk_list_store_append(store, &iter);
	    gtk_list_store_set(store, 
	                       &iter, 
	                       IMAGE_COLUMN, 
	                       icon,
	                       TITLE_COLUMN, 
	                       title.c_str(),
	                       -1);
	}
	
	string onPreExecute() {
		gtk_label_set_text(GTK_LABEL(lbInfo), actors.getTitle().c_str());
	    showSpActors();
	    hideAllLinksButtons();
	    gtk_widget_show(vbRight);
	    return actors.getUrl();
	}
	
	void onPostExecute(string &page) {
		if(!page.empty()) {
			actors.setNetworkOk(TRUE);
			actors.parse(page);
			
			if(!actors.getPlayerUrl().empty()) {
				detectThread();
			}else {
				actors.setLinksMode(LINKS_MODE_HIDE);
			}
			updateActors();
			showActors();
		}else {
		    showActorsError();
		}
	}
};
