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
    
    GtkWidget *spLinks;
    GtkWidget *btnLinksError;
    GtkWidget *btnGetLinks;
    GtkWidget *btnListEpisodes;
    GtkWidget *btnSave;
    GtkWidget *btnDelete;
    GtkWidget *tvSavedItems;
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
				  GtkWidget *tvSavedItems,
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
		this->tvSavedItems = tvSavedItems;
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
	                                   
	    player = detectPlayer();
	} 
	
	GtkWidget* getWindow() {
		return window;
	}
	
    void newThread(string title, string href, GdkPixbuf *pixbuf) {
		if(!actors.getTitle().empty() && actors.isNetworkOk()) {
			prevActors = actors;
		}
		actors.setTitle(title);
		actors.setUrl(href);
		actors.setPixbuf(pixbuf);
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
			case LINKS_MODE_HIDE:
			    hideAllLinksButtons();
			break;
		}
		
		// Save or delete button change
		showSaveOrDeleteButton();
	}
	
	void btnActorsClicked(GtkWidget *widget) {
		//Toggle visibility of actors list (vbRight)
		gtk_widget_set_visible(vbRight,
		                       !gtk_widget_get_visible(vbRight));
	}
	
	ListEpisodesArgs getCurrentActorsListEpisodesArgs() {
		return actors.getListEpisodesArgs();
	}
	
	void runPlayItemDialog() {
		string js = actors.getJs();
		PlayItem playItem = PlaylistsUtils::parse_play_item(js, FALSE);
		playItem.player = player;
		if(!playItem.comment.empty()) { // PlayItem found
			// Set title for trailers
			if(playItem.comment.size() == 1) {
				playItem.comment = actors.getTitle();
			}
			
		    ProcessPlayItemDialog ppid(window, playItem);
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
				actorsHistory->resultsHistory->setListEpisodesArgs(listEpisodesArgs);
				if(!gtk_toggle_tool_button_get_active(
				    GTK_TOGGLE_TOOL_BUTTON(actorsHistory->btnActors))) {
					gtk_button_clicked(GTK_BUTTON(actorsHistory->btnListEpisodes));
				}
			}else {
				actorsHistory->showGetLinksButton();
				actorsHistory->actors.setJs(js);
				actorsHistory->actors.setLinksMode(LINKS_MODE_MOVIE);
				if(!gtk_toggle_tool_button_get_active(
				    GTK_TOGGLE_TOOL_BUTTON(actorsHistory->btnActors))) {
					actorsHistory->resultsHistory->showResults();
					actorsHistory->runPlayItemDialog();
				}
			}
		}else {
			actorsHistory->showLinksErrorButton();
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
	    // If actors pane is not shown
		if(!gtk_toggle_tool_button_get_active(
		    GTK_TOGGLE_TOOL_BUTTON(btnActors))) {
			resultsHistory->showSpCenter(FALSE);
		}else {
			gtk_widget_hide(frInfo);
			gtk_widget_hide(frRightTop);
			gtk_widget_hide(hbActorsError);
			gtk_widget_show(spActors);
			gtk_spinner_start(GTK_SPINNER(spActors));
		    gtk_widget_hide(frActions);
		}
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
		// If actors pane is not shown
		if(!gtk_toggle_tool_button_get_active(
		    GTK_TOGGLE_TOOL_BUTTON(btnActors))) {
			resultsHistory->showResults();
			runErrorDialog();
		}else {
			gtk_widget_hide(frInfo);
			gtk_widget_hide(frRightTop);
			gtk_widget_show(hbActorsError);
			gtk_widget_hide(spActors);
			gtk_spinner_stop(GTK_SPINNER(spActors));
			gtk_widget_hide(frActions);
		}
	}
	
	void runErrorDialog() {
		GtkWidget *dialog = gtk_message_dialog_new(
		              GTK_WINDOW(window),
		              GTK_DIALOG_DESTROY_WITH_PARENT,
		              GTK_MESSAGE_ERROR,
		              GTK_BUTTONS_OK,
		              "Network problem");
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
	    showSpActors();
	    hideAllLinksButtons();
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
				// If actors pane is not shown
				if(!gtk_toggle_tool_button_get_active(
				    GTK_TOGGLE_TOOL_BUTTON(btnActors))) {
					resultsHistory->showResults();
				}
			}
			
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
