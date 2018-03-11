//ActorsHistory.hpp
#include "Actors.hpp"
#include "FileUtils.hpp"

enum LinkResponse {
	LINK_RESPONSE_PLAY,
	LINK_RESPONSE_DOWNLOAD,
	LINK_RESPONSE_CANCEL
};

class ActorsHistory {
    Actors actors, prevActors;
    map <string, Actors> backActors;
    
    GtkWidget *window;
    GtkWidget *tvActors, *tvBackActors;
    GtkWidget *frRightBottom;
    GdkPixbuf *icon;
    GtkWidget *lbInfo;
    
    GtkWidget *frRightTop, *frInfo;
    GtkWidget *spActors;
    GtkWidget *hbActorsError;
    GtkWidget *vbRight;
    
    GThreadPool *actorsThreadPool;
    GThreadPool *linksSizeThreadPool;
    GThreadPool *detectThreadPool;
    
    GtkWidget *spLinks;
    GtkWidget *btnLinksError;
    GtkWidget *btnGetLinks;
    GtkWidget *btnListEpisodes;
    GtkWidget *btnSave;
    GtkWidget *btnDelete;
    GtkWidget *tvSavedItems;
    GtkToolItem *btnSavedItems;
    GtkToolItem *btnActors;
    
    public:
    
    ActorsHistory(GtkWidget *window,
                  GtkWidget *a,
                  GtkWidget *pa,
                  GtkWidget* fr, 
                  GtkWidget* li,
                  GtkWidget *frt, 
                  GtkWidget *fri, 
                  GtkWidget *sa, 
                  GtkWidget *hba,
                  GtkWidget *vbr,
                  GtkWidget *spLinks,
				  GtkWidget *btnLinksError,
				  GtkWidget *btnGetLinks,
				  GtkWidget *btnListEpisodes,
				  GtkWidget *btnSave,
				  GtkWidget *btnDelete,
				  GtkWidget *tvSavedItems,
				  GtkToolItem *btnSavedItems,
				  GtkToolItem *btnActors) {
		this->window = window;			  
		tvActors = a;
		tvBackActors = pa;
		frRightBottom = fr;
		lbInfo = li;
		
		frRightTop = frt;
		frInfo = fri;
		spActors = sa;
		hbActorsError = hba;
		vbRight = vbr;
		
		this->spLinks = spLinks;
		this->btnLinksError = btnLinksError;
		this->btnGetLinks = btnGetLinks;
		this->btnListEpisodes = btnListEpisodes;
		
		this->btnSave = btnSave;
		this->btnDelete = btnDelete;
		this->tvSavedItems = tvSavedItems;
		this->btnSavedItems = btnSavedItems;
		this->btnActors = btnActors;
		
		icon = IconsFactory::getLinkIcon();
		
		// GThreadPool for actors
	    actorsThreadPool = g_thread_pool_new(ActorsHistory::actorsTask,
	                                   this,
	                                   1, // Run one thread at the time
	                                   FALSE,
	                                   NULL);
	                                   
	    // GThreadPool for links sizes
	    linksSizeThreadPool = g_thread_pool_new(ActorsHistory::linksSizeTask,
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
	} 
	
    void newThread(string title, string href) {
		if(!actors.getTitle().empty() && actors.isNetworkOk()) {
			prevActors = actors;
		}
		actors.setTitle(title);
		actors.setUrl(href);
		actors.setNetworkOk(FALSE);
		showSaveOrDeleteButton();
		newThread();
	}
	
	void newThread() {
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
				backActors[actors.getTitle()] = actors; // save prev actors
				backActorsListAdd(actors.getTitle());
			}
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
			break;
		}
		
		// Save or delete button change
		showSaveOrDeleteButton();
	}
	
	void btnActorsClicked(GtkWidget *widget) {
		//Toggle visibility of actors list (vbRight)
		if(!gtk_widget_get_visible(vbRight)) {
			if(actors.getCount() > 0) {
				gtk_widget_set_visible(vbRight, TRUE);
			}
		}else {
			gtk_widget_set_visible(vbRight, FALSE);
		}
	}
	
	void linksSizeDialogThread(PlayItem playItem) {
		PlayItem *playItemArg = new PlayItem();
		playItemArg->comment = playItem.comment;
		playItemArg->file = playItem.file;
		playItemArg->download = playItem.download;
		g_thread_pool_push(linksSizeThreadPool, 
		                   (gpointer)playItemArg, 
		                   NULL);
	}
	
	ListEpisodesArgs getCurrentActorsListEpisodesArgs() {
		return actors.getListEpisodesArgs();
	}
	
	void btnGetLinksClicked() {
		string js = actors.getJs();
		PlayItem playItem = PlaylistsUtils::parse_play_item(js, FALSE);
		
		if(!playItem.comment.empty()) { // PlayItem found
		    linksSizeDialogThread(playItem);
		}
	}
	
	void btnLinksErrorClicked() {
		detectThread();
	}
	
	void btnSaveClicked() {
		FileUtils::writeToFile(actors.getTitle(), actors.getUrl());
		showSaveOrDeleteButton();
	}
	
	void btnDeleteClicked() {
		FileUtils::removeFile(actors.getTitle());
		showSaveOrDeleteButton();
	}
	
	private:
	
	void showSaveOrDeleteButton() {
		if(FileUtils::isTitleSaved(actors.getTitle())) {
			gtk_widget_set_visible(btnSave, FALSE);
			gtk_widget_set_visible(btnDelete, TRUE);
		}else {
			gtk_widget_set_visible(btnSave, TRUE);
			gtk_widget_set_visible(btnDelete, FALSE);
		}
		// Update file list
		FileUtils::listSavedFiles(tvSavedItems, btnSavedItems);
	}
	
	// Info links frame functions
    void showSpLinks() {
		gtk_widget_set_visible(btnGetLinks, FALSE);
		gtk_widget_set_visible(btnListEpisodes, FALSE);
		gtk_widget_set_visible(btnLinksError, FALSE);
		gtk_widget_set_visible(spLinks, TRUE);
		gtk_spinner_start(GTK_SPINNER(spLinks));
	}
	
	void showListEpisodesButton() {
		gtk_widget_set_visible(btnGetLinks, FALSE);
		gtk_widget_set_visible(btnListEpisodes, TRUE);
		gtk_widget_set_visible(btnLinksError, FALSE);
		gtk_widget_set_visible(spLinks, FALSE);
		gtk_spinner_stop(GTK_SPINNER(spLinks));
	}
	
	void showGetLinksButton() {
		gtk_widget_set_visible(btnGetLinks, TRUE);
		gtk_widget_set_visible(btnListEpisodes, FALSE);
		gtk_widget_set_visible(btnLinksError, FALSE);
		gtk_widget_set_visible(spLinks, FALSE);
		gtk_spinner_stop(GTK_SPINNER(spLinks));
	}
	
	void showLinksErrorButton() {
		gtk_widget_set_visible(btnGetLinks, FALSE);
		gtk_widget_set_visible(btnListEpisodes, FALSE);
		gtk_widget_set_visible(btnLinksError, TRUE);
		gtk_widget_set_visible(spLinks, FALSE);
		gtk_spinner_stop(GTK_SPINNER(spLinks));
	}
	
	void hideAllLinksButtons() {
		gtk_widget_set_visible(btnGetLinks, FALSE);
		gtk_widget_set_visible(btnListEpisodes, FALSE);
		gtk_widget_set_visible(btnLinksError, FALSE);
		gtk_widget_set_visible(spLinks, FALSE);
		gtk_spinner_stop(GTK_SPINNER(spLinks));
	}
	
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
		if(!js.empty()) {
			string playlist_link = PlaylistsUtils::get_txt_link(js);
			if(!playlist_link.empty()) { // Playlists found
				actorsHistory->showListEpisodesButton();
				ListEpisodesArgs listEpisodesArgs;
				listEpisodesArgs.title = actorsHistory->actors.getTitle();
				listEpisodesArgs.playlist_link = playlist_link;
				actorsHistory->actors.setListEpisodesArgs(listEpisodesArgs);
				actorsHistory->actors.setLinksMode(LINKS_MODE_SERIAL);
			}else {
				actorsHistory->showGetLinksButton();
				actorsHistory->actors.setJs(js);
				actorsHistory->actors.setLinksMode(LINKS_MODE_MOVIE);
			}
		}else {
			actorsHistory->showLinksErrorButton();
		}
		gdk_threads_leave();
	}
	
	static string detectPlayer() {
		if(system("which mpv") == 0) {
			return "mpv ";
		}
		if(system("which mplayer") == 0) {
			return "mplayer -cache 2048 ";
		}
		return "";
	}
	
	static void processPlayItem(PlayItem* playItem) {
		string command;
		if(!playItem->fileSize.empty()) {
			command = playItem->player + playItem->file + " &";
		}else if(!playItem->downloadSize.empty()){
			command = playItem->player + playItem->download + " &";
		}
	    system(command.c_str());
	}
	
	static void dialogResponse(GtkWidget *dialog,
	                           gint response_id, 
	                           gpointer user_data) {
		PlayItem *playItem = (PlayItem*)user_data;
		gtk_widget_destroy(dialog);
		// For pasting with "paste" or ctrl-v
        GtkClipboard* clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
        // For pasting with middle mouse button (in urxvt)
        GtkClipboard* clipboardX = gtk_clipboard_get(GDK_SELECTION_PRIMARY);
        switch(response_id) {
			case LINK_RESPONSE_PLAY:
			    gtk_clipboard_set_text(clipboard,
			                           playItem->file.c_str(),
			                           playItem->file.size());
			    gtk_clipboard_set_text(clipboardX,
			                           playItem->file.c_str(),
			                           playItem->file.size());
			                           
			    processPlayItem(playItem);
			break;
			case LINK_RESPONSE_DOWNLOAD:
			    gtk_clipboard_set_text(clipboard,
			                           playItem->download.c_str(),
			                           playItem->download.size());
			    gtk_clipboard_set_text(clipboardX,
			                           playItem->file.c_str(),
			                           playItem->file.size());
			break;
			case LINK_RESPONSE_CANCEL:
			    // Do nothing. Dialog should already be destroyed.
			break;
		}
		g_free(playItem);
	}
	
	static void linksSizeTask(gpointer args, gpointer args2) {
		ActorsHistory *actorsHistory = (ActorsHistory *)args2;
		PlayItem *playItem = (PlayItem *)args;
		
		// On pre execute
		gdk_threads_enter();
		// Create dialog, add buttons to it, get size information and display it
		GtkWidget *dialog, 
		          *label, 
		          *content_area,
		          *btnPlay,
		          *btnDownload,
		          *spDialog;
		          
		spDialog = gtk_spinner_new();
		gtk_widget_set_size_request(spDialog, 32, 32);
		
		dialog = gtk_dialog_new_with_buttons ("Play or copy link...",
                                              GTK_WINDOW(actorsHistory->window),
                                              (GtkDialogFlags)(GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT),
                                              NULL);
                                              
        btnPlay = gtk_dialog_add_button(GTK_DIALOG(dialog),
		                      "Play",
                              LINK_RESPONSE_PLAY);                   
        gtk_widget_set_sensitive(btnPlay, FALSE);
        
        btnDownload = gtk_dialog_add_button(GTK_DIALOG(dialog),
		                      "Copy",
                              LINK_RESPONSE_DOWNLOAD);
        gtk_widget_set_sensitive(btnDownload, FALSE);
        
        gtk_dialog_add_button(GTK_DIALOG(dialog),
		                      "Cancel",
                              LINK_RESPONSE_CANCEL);
                                              
        /* Ensure that the dialog box is destroyed when the user responds. */
		g_signal_connect(dialog,
						 "response",
					     G_CALLBACK (ActorsHistory::dialogResponse),
						 playItem);
                                              
        content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
        
        if(playItem->comment.size() == 1) {
			playItem->comment = actorsHistory->actors.getTitle();
		}
        
	    label = gtk_label_new (playItem->comment.c_str());
	    
	    /* Add the label, and show everything we've added to the dialog. */
	    //gtk_container_add (GTK_CONTAINER (content_area), label);
	    gtk_box_pack_start(GTK_BOX(content_area), spDialog, TRUE, FALSE, 1);
	    gtk_box_pack_start(GTK_BOX(content_area), label, TRUE, FALSE, 5);
        gtk_widget_show_all(dialog); 
        // Hide label (title) and show spinner
        gtk_widget_set_visible(label, FALSE);
        gtk_spinner_start(GTK_SPINNER(spDialog)); 
        
		gdk_threads_leave();
		
		// Async part
		if(!playItem->file.empty()) {
			playItem->fileSize = HtmlString::getSizeOfLink(playItem->file);
		}
		if(!playItem->download.empty()) {
			playItem->downloadSize = HtmlString::getSizeOfLink(playItem->download);
		}
	
		gdk_threads_enter();
		//On post execute
		// Show label (title) and hide spinner
		gtk_widget_set_visible(label, TRUE);
		gtk_spinner_stop(GTK_SPINNER(spDialog));
		gtk_widget_set_visible(spDialog, FALSE);
		
		if(!playItem->fileSize.empty()) {
			string sizeFileTitle = "Play (" + playItem->fileSize + " Mb)";
			gtk_button_set_label(GTK_BUTTON(btnPlay), sizeFileTitle.c_str());
		}else if(!playItem->downloadSize.empty()) {
			string sizeFileTitle = "Play (" + playItem->downloadSize + " Mb)";
			gtk_button_set_label(GTK_BUTTON(btnPlay), sizeFileTitle.c_str());
		}
		
		if(!playItem->downloadSize.empty()) {
			string sizeDownloadTitle = "Copy (" + playItem->downloadSize + " Mb)";
			gtk_button_set_label(GTK_BUTTON(btnDownload), sizeDownloadTitle.c_str());
		}
		
		playItem->player = detectPlayer();
		string msg;
		if(!playItem->player.empty()) {
			msg = "Play in " + playItem->player;
		}else {
			msg = "Mplayer or mpv not detected. Copy to clipboard";
		}
		gtk_widget_set_tooltip_text(btnPlay, msg.c_str());
		gtk_widget_set_tooltip_text(btnDownload, "Copy to clipboard");
		
		if(!playItem->file.empty() && (!playItem->fileSize.empty() || !playItem->downloadSize.empty())) {
			gtk_widget_set_sensitive(btnPlay, TRUE);
		}
		
		if(!playItem->download.empty()) {
			gtk_widget_set_sensitive(btnDownload, TRUE);
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
		gtk_widget_set_visible(frInfo, FALSE);
		gtk_widget_set_visible(frRightTop, FALSE);
		gtk_widget_set_visible(hbActorsError, FALSE);
		gtk_widget_set_visible(spActors, TRUE);
		gtk_spinner_start(GTK_SPINNER(spActors));
		gtk_widget_set_visible(vbRight, TRUE);
		gtk_widget_set_sensitive(GTK_WIDGET(btnActors), TRUE);
	}
	
	void showActors() {
		gtk_widget_set_visible(frInfo, TRUE);
		gtk_widget_set_visible(frRightTop, TRUE);
		gtk_widget_set_visible(hbActorsError, FALSE);
		gtk_widget_set_visible(spActors, FALSE);
		gtk_spinner_stop(GTK_SPINNER(spActors));
	}
	
	void showActorsError() {
		gtk_widget_set_visible(frInfo, FALSE);
		gtk_widget_set_visible(frRightTop, FALSE);
		gtk_widget_set_visible(hbActorsError, TRUE);
		gtk_widget_set_visible(spActors, FALSE);
		gtk_spinner_stop(GTK_SPINNER(spActors));
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
	    showSpActors();
	    hideAllLinksButtons();
	    return actors.getUrl();
	}
	
	void onPostExecute(string &page) {
		if(!page.empty()) {
			detectThread();
			actors.setNetworkOk(TRUE);
			actors.parse(page);
			// Save to back actors map
			if(backActors.count(prevActors.getTitle()) == 0 
			        && prevActors.getCount() > 0
			        && prevActors.getTitle() != actors.getTitle()) {
			    backActors[prevActors.getTitle()] = prevActors;
			    backActorsListAdd(prevActors.getTitle());	
			}
			updateActors();
			showActors();
		}else {
		    showActorsError();
		}
	}
};
