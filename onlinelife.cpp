#include <gtk/gtk.h>
#include <vector>
#include <string>
#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <string.h>
#include <map>
#include <set>
#include <sstream>

#define SSTR( x ) static_cast< std::ostringstream & >( \
        ( std::ostringstream() << std::dec << x ) ).str()

#define DOMAIN "http://online-life.club"

#include "Converter.hpp"
#include "HtmlString.hpp"
#include "CreatePixbuf.hpp"
#include "ColumnsEnum.hpp"

GdkPixbuf *defaultPixbuf;
map<string, GdkPixbuf*> imagesCache;

#include "CategoriesParser.hpp"
#include "Results.hpp"
#include "ResultsHistory.hpp"
#include "Playlists.hpp"
#include "Actors.hpp"

using namespace std;

//Actors
Actors actors, prevActors;

map <string, Actors> backActors;

GtkWidget *frRightBottom;
GtkWidget *lbInfo;
GtkWidget *frRightTop, *frInfo;

const string PROG_NAME("Online life");

GtkWidget *ivResults;
GtkWidget *tvPlaylists;

GtkToolItem *btnUp;
GtkToolItem *btnPrev;
GtkToolItem *btnNext;

GtkToolItem *rbActors;
GtkToolItem *rbPlay;
GtkToolItem *rbDownload;

GtkToolItem *btnRefresh;

GtkWidget *window;
GtkWidget *entry;

GtkWidget *swTree, *swIcon;
GtkWidget *tvCategories, *tvActors, *tvBackActors;
GtkWidget *vbLeft, *vbRight;

GtkWidget *spCenter;

GtkWidget *spCategories;
GtkWidget *hbCategoriesError;

GtkWidget *swLeftTop;

GtkWidget *spActors;
GtkWidget *hbActorsError;

GtkWidget *vbCenter;
GtkWidget *hbResultsError;
GtkWidget *btnResultsError;

GThreadPool *imagesThreadPool;
GThreadPool *resultsNewThreadPool;
GThreadPool *resultsAppendThreadPool;
GThreadPool *actorsThreadPool;
GThreadPool *playlistsThreadPool;

set<string> resultsThreadsLinks;
set<int> imageIndexes;

/*string readFromFile(string filename) {
	ifstream in(filename.c_str());
	if(in) {
		return string((istreambuf_iterator<char>(in)), istreambuf_iterator<char>());
	}
	return "";
}

void writeToFile(string filename, string input) {
	ofstream out(filename.c_str());
	if(out) {
		out << input;
	}
}*/

void switchToTreeView() {
	gtk_widget_set_visible(swTree, TRUE);
	gtk_widget_set_visible(swIcon, FALSE);
	gtk_widget_set_visible(spCenter, FALSE);
	gtk_widget_set_visible(hbResultsError, FALSE);
	gtk_spinner_stop(GTK_SPINNER(spCenter));
}

void switchToIconView() {
	gtk_widget_set_visible(swTree, FALSE);
	gtk_widget_set_visible(swIcon, TRUE);
	gtk_widget_set_visible(spCenter, FALSE);
	gtk_widget_set_visible(hbResultsError, FALSE);
	gtk_spinner_stop(GTK_SPINNER(spCenter));
}

void disableAllItems() {
	gtk_widget_set_sensitive(GTK_WIDGET(btnUp), FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(btnPrev), FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(btnNext), FALSE);
	
	gtk_widget_set_sensitive(GTK_WIDGET(rbActors), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(rbPlay), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(rbDownload), FALSE);
    
    gtk_widget_set_sensitive(GTK_WIDGET(btnRefresh), FALSE);
}

void setSensitiveItemsPlaylists() {
	gtk_widget_set_sensitive(GTK_WIDGET(btnUp), TRUE);
	gtk_widget_set_sensitive(GTK_WIDGET(btnPrev), FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(btnNext), FALSE);
	
	gtk_widget_set_sensitive(GTK_WIDGET(rbPlay), TRUE);
    gtk_widget_set_sensitive(GTK_WIDGET(rbDownload), TRUE);
    gtk_widget_set_sensitive(GTK_WIDGET(rbActors), FALSE);
    
    gtk_widget_set_sensitive(GTK_WIDGET(btnRefresh), FALSE);
}

void setSensitiveItemsResults() {
	gtk_widget_set_sensitive(GTK_WIDGET(btnUp), FALSE);
	
	gtk_widget_set_sensitive(GTK_WIDGET(rbPlay), TRUE);
    gtk_widget_set_sensitive(GTK_WIDGET(rbDownload), TRUE);
    gtk_widget_set_sensitive(GTK_WIDGET(rbActors), TRUE);
    
    gtk_widget_set_sensitive(GTK_WIDGET(btnRefresh), TRUE);
    
    //gtk_entry_set_text(GTK_ENTRY(entry), "");
}

struct ArgsStruct {
	GdkPixbufLoader* loader;
	GtkTreeIter iter;
};

static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	size_t realsize = size * nmemb;
	struct ArgsStruct *args = (struct ArgsStruct *)userp;
	// Setting new partially downloaded image here
	gdk_pixbuf_loader_write(args->loader, (const guchar*)contents, realsize, NULL);
	GdkPixbuf* pixbufOrig = gdk_pixbuf_loader_get_pixbuf(args->loader);
	if(pixbufOrig != NULL) {
		gdk_threads_enter();
		GtkListStore *store = GTK_LIST_STORE(gtk_icon_view_get_model
			(GTK_ICON_VIEW(ivResults)));
		gtk_list_store_set(store, &args->iter, IMAGE_COLUMN, pixbufOrig, -1);  
		gdk_threads_leave();
	}
	
	return realsize;
}

void getPixbufFromUrl(string url, GtkTreeIter iter, int index) {
    
	CURL *curl_handle;
	CURLcode res;
	GdkPixbuf *pixbuf;
	
	GdkPixbufLoader* loader = gdk_pixbuf_loader_new();
	struct ArgsStruct args;
	args.loader = loader;
	args.iter = iter;
	
	/* init the curl session */ 
	curl_handle = curl_easy_init();
	
	/* remove crash bug */
	curl_easy_setopt(curl_handle, CURLOPT_NOSIGNAL, 1);
	
	/* specify URL to get */ 
	curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
	
	/* send all data to this function  */ 
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
	
	/* we pass our 'chunk' struct to the callback function */ 
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&args);
	
	/* some servers don't like requests that are made without a user-agent
	 field, so we provide one */ 
	curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
	
	/* get it! */ 
	res = curl_easy_perform(curl_handle);
	
	/* check for errors */ 
	if(res != CURLE_OK) {
		gdk_pixbuf_loader_close(loader, NULL); //close in case of error
		fprintf(stderr, "curl_easy_perform() failed: %s\n",
				curl_easy_strerror(res));
		// In case of error remove index from imageIndexes 
        // to have a chance to reload image
        imageIndexes.erase(index);
	}else { 
        GError *error = NULL;
		gboolean ok = gdk_pixbuf_loader_close(loader, &error);
		if(!ok) {
	        fprintf(stderr, "On close: %s\n", error->message);
	        // In case of error remove index from imageIndexes 
	        // to have a chance to reload image
	        imageIndexes.erase(index);
            g_error_free(error);
		}else {
			// Setting new fully downloaded image here
            //Make copy of pixbuf to be able to free loader
			pixbuf = GDK_PIXBUF(g_object_ref(gdk_pixbuf_loader_get_pixbuf(loader)));
			gdk_threads_enter();
			GtkListStore *store;
			imagesCache[url] = pixbuf;
				
			store = GTK_LIST_STORE(gtk_icon_view_get_model
				(GTK_ICON_VIEW(ivResults))); 
			gtk_list_store_set(store, &iter, IMAGE_COLUMN, pixbuf, -1);
			gdk_threads_leave();
        }
	}
	
	/* cleanup curl stuff */ 
	curl_easy_cleanup(curl_handle);
	
	//free(chunk.memory);
	g_object_unref(loader);
}

void imageDownloadTask(gpointer arg, gpointer arg1) {
	gint index = (gint)(long)arg;
	index--;
	
	gdk_threads_enter();
	GtkTreeModel* model = gtk_icon_view_get_model(GTK_ICON_VIEW(ivResults));
	gdk_threads_leave();
	
	string strIndex = SSTR(index);
	GtkTreeIter iter;
	if(gtk_tree_model_get_iter_from_string (
	    model,
	    &iter,
	    strIndex.c_str())) {
		// Get image link value from iter
        gchar *imageLink = NULL;
        gtk_tree_model_get(model, &iter, ICON_IMAGE_LINK, &imageLink, -1);
        string link(imageLink);
        g_free(imageLink);
        
        gdk_threads_enter();
        int count = imagesCache.count(link);
        gdk_threads_leave();
        
	    if(count == 0) { 
			getPixbufFromUrl(link, iter, index);
		}
	}
}

void displayRange() {
	GtkTreePath *path1, *path2;
	if(gtk_icon_view_get_visible_range(GTK_ICON_VIEW(ivResults), &path1, &path2)) {
		gint *indices1 = gtk_tree_path_get_indices(path1);
		gint *indices2 = gtk_tree_path_get_indices(path2);
		gint indexDisplayFirst = indices1[0];
		gint indexDisplayLast = indices2[0];
		
	    // Downloading images for displayed items
		for(int i = indexDisplayFirst; i <= indexDisplayLast; i++) {
			if(imageIndexes.count(i) == 0) {
				imageIndexes.insert(i);
				g_thread_pool_push(imagesThreadPool, 
				    (gpointer)(long)(i+1),
				     NULL);
			}	
		}		
		
		gtk_tree_path_free(path1);
		gtk_tree_path_free(path2);
	}
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

void show_error_dialog() {
	GtkWidget *dialog;
	dialog = gtk_message_dialog_new(GTK_WINDOW(window),
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_ERROR,
			GTK_BUTTONS_OK,
			"Nothing found");
	gtk_window_set_title(GTK_WINDOW(dialog), "Error");
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}

void resultsAppendTask(gpointer arg, gpointer arg1) {
	ResultsHistory *resultsHistory = (ResultsHistory *)arg1;
	// On pre execute
	gdk_threads_enter();
	string link = resultsHistory->getNextLink();
	// Display spinner at the bottom of list
    showSpCenter(TRUE);
	gdk_threads_leave();
	// async part
	string page = HtmlString::getResultsPage(link);
	// On post execute
	gdk_threads_enter();
	if(!page.empty()) {
		resultsHistory->show(page);
		switchToIconView();
	}else { // error
		if(resultsHistory->threadLinksContainNextLink()) {
			resultsHistory->eraseNextLinkFromThreadLinks();
		}
		switchToIconView();
		showResultsRepeat(TRUE);
		resultsHistory->setError(TRUE);
	}
	gdk_threads_leave();
}

void resultsNewTask(gpointer arg, gpointer arg1) {
	// On pre execute
	gdk_threads_enter();
	ResultsHistory *resultsHistory = (ResultsHistory*)arg1;
	// New images for new indexes will be downloaded
    imageIndexes.clear();
    
	// Display spinner for new results
    showSpCenter(FALSE);
    string link = resultsHistory->getUrl();
    gdk_threads_leave();
    // async part
	string page = HtmlString::getResultsPage(link);
    gdk_threads_enter();
	if(!page.empty()) {
		//TODO: maybe I need to clear it while saving....
		// clear forward results stack on fetching new results
	    resultsHistory->clearForwardResultsStack();

		resultsHistory->removeBackStackDuplicate();
		
		// Scroll to the top of the list
	    GtkTreePath *path = gtk_tree_path_new_first();
	    gtk_icon_view_scroll_to_path(GTK_ICON_VIEW(ivResults), path, FALSE, 0, 0);
	    
	    // Clear results links set if not paging
	    resultsHistory->clearThreadsLinks();
		
		resultsHistory->createNewModel();
		switchToIconView();
		resultsHistory->show(page);
		
		setSensitiveItemsResults();
	}else {
		switchToIconView();
		showResultsRepeat(FALSE);
		resultsHistory->setError(TRUE);
	}
	
	if(resultsHistory->isRefresh()) {
		resultsHistory->setRefresh(FALSE);
	}
	gdk_threads_leave();
}

void displayPlaylists() {
	switchToTreeView();
	setSensitiveItemsPlaylists();
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

void playOrDownload(string file, string download) {
	if(gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(rbDownload))){
	    string command = detectTerminal() + "wget -P ~/Download -c " + download + " &";
        system(command.c_str());
	}else if(gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(rbPlay))) {
	    string command = detectPlayer() + file + " &";
        system(command.c_str());
	}
}

void playlistsTask(gpointer args, gpointer args2) {
	Playlists *playlists = (Playlists*)args2;
	ResultsHistory *resultsHistory = playlists->getResultsHistory();
	playlists->setUrl(args);
	string href = playlists->getUrl();
	string id = playlists->getHrefId();
	if(!id.empty()) {
		string url = "http://dterod.com/js.php?id=" + id;
		string referer = "http://dterod.com/player.php?newsid=" + id;
		// On pre execute
		gdk_threads_enter();
		// Show spinner fullscreen
		showSpCenter(FALSE);
		gdk_threads_leave();
		
		string js = HtmlString::getPage(url, referer);
		string playlist_link = playlists->get_txt_link(js);
		if(!playlist_link.empty()) { // Playlists found
			string json = HtmlString::getPage(playlist_link);
			gdk_threads_enter();
			playlists->parse(json);
			if(playlists->getCount() > 0) {
				displayPlaylists();
			}else {
				showResultsRepeat(FALSE);
				setSensitiveItemsPlaylists();
			}
			gdk_threads_leave();
		}else { //PlayItem found or nothing found
			gdk_threads_enter();
			PlayItem playItem = playlists->parse_play_item(js);
			if(!playItem.comment.empty()) { // PlayItem found
			    // get results list back
			    switchToIconView();
			    resultsHistory->updateTitle();
				processPlayItem(playItem); 
			}else {
				if(resultsHistory->getTitle().find("Трейлеры") != string::npos) {
					gdk_threads_leave();
					// Searching for alternative trailers links
		            string infoHtml = HtmlString::getPage(href, referer);
		            string trailerId = playlists->getTrailerId(infoHtml); 
		            url = "http://dterod.com/js.php?id=" + trailerId + "&trailer=1";
		            referer = "http://dterod.com/player.php?trailer_id=" + trailerId;
		            string json = HtmlString::getPage(url, referer);
					gdk_threads_enter();
					// get results list back
			        switchToIconView();
			        resultsHistory->updateTitle();
					processPlayItem(playlists->parse_play_item(json)); 
				}else {
					showResultsRepeat(FALSE);
				    setSensitiveItemsPlaylists();
				}
			}
			gdk_threads_leave();
		}
	}
}

void updateActors() {
	gtk_label_set_text(GTK_LABEL(lbInfo), actors.getTitle().c_str());
	GtkTreeModel *model;
	model = actors.getModel();
    gtk_tree_view_set_model(GTK_TREE_VIEW(tvActors), model);
	//g_object_unref(model); // do not free, used for actors history
}

void backActorsListAdd(string title) {
	gtk_widget_set_visible(frRightBottom, TRUE);
	
	GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(
	    GTK_TREE_VIEW(tvBackActors)));
	GtkTreeIter iter;
	GdkPixbuf *icon = create_pixbuf("link_16.png");
	
	gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, IMAGE_COLUMN, icon,
           TITLE_COLUMN, title.c_str(), -1);
}

void showSpActors() {
	gtk_widget_set_visible(frInfo, FALSE);
	gtk_widget_set_visible(frRightTop, FALSE);
	gtk_widget_set_visible(hbActorsError, FALSE);
	gtk_widget_set_visible(spActors, TRUE);
	gtk_spinner_start(GTK_SPINNER(spActors));
	gtk_widget_set_visible(vbRight, TRUE);
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

void actorsTask(gpointer args, gpointer args2) {
	// On pre execute
	gdk_threads_enter();
	string link = actors.getUrl();
	showSpActors();
	gdk_threads_leave();
	
	string page = HtmlString::getActorsPage(link);
	
	// On post execute
	gdk_threads_enter();
	if(!page.empty()) {
		actors.parse(page);
		if(backActors.count(prevActors.getTitle()) == 0 
		        && prevActors.getCount() > 0
		        && prevActors.getTitle() != actors.getTitle()) {
		    backActors[prevActors.getTitle()] = prevActors;
		    backActorsListAdd(prevActors.getTitle());	
		}
		showActors();
		updateActors();	
	}else {
	    showActorsError();
	}
	gdk_threads_leave();
}

void categoriesClicked(GtkWidget *widget, GtkTreePath *path, gpointer data) {
	ResultsHistory *resultsHistory = (ResultsHistory *)data;
	resultsHistory->saveToBackStack();
	
	// Get model from tree view
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
	
	// Get iter from path
	GtkTreeIter iter, parent;
	gtk_tree_model_get_iter(model, &iter, path);
	
	// Get title and link values from iter
	gchar *title = NULL;
	gchar *link = NULL;
	gtk_tree_model_get(model,
	                   &iter,
	                   TREE_TITLE_COLUMN,
	                   &title,
	                   TREE_HREF_COLUMN,
	                   &link,
	                   -1);
	                   
	// Get parent (category) of iter
	if(gtk_tree_model_iter_parent(model, &parent, &iter)) {
		gchar *parentTitle = NULL;
		gtk_tree_model_get(model,
		                   &parent,
		                   TREE_TITLE_COLUMN,
		                   &parentTitle,
		                   -1);
	    resultsHistory->setTitle(string(parentTitle) + " - " + title);
		g_free(parentTitle);
	}else {
		resultsHistory->setTitle(title);
	}
	
	resultsHistory->updateTitle();
	resultsHistory->setUrl(link);
	g_thread_pool_push(resultsNewThreadPool, (gpointer)1, NULL);
	
	g_free(title);
	g_free(link);
}

void actorsClicked(GtkWidget *widget, GtkTreePath *path, gpointer data) {
	ResultsHistory *resultsHistory = (ResultsHistory *)data;
	// Get model from tree view
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
	
	// Get iter from path
	GtkTreeIter iter;
	gtk_tree_model_get_iter(model, &iter, path);
	
	// Get title and link values from iter
	gchar *title = NULL;
	gchar *link = NULL;
	gtk_tree_model_get(model,
	                   &iter, 
	                   TREE_TITLE_COLUMN, 
	                   &title,
	                   TREE_HREF_COLUMN,
	                   &link, 
	                   -1);
	                   
	resultsHistory->saveToBackStack();
	resultsHistory->setTitle(title);
	resultsHistory->updateTitle();
	resultsHistory->setUrl(link);
    g_thread_pool_push(resultsNewThreadPool, (gpointer)1, NULL);
	                   
	g_free(title);
	g_free(link);
}

void playlistClicked(GtkWidget *widget, GtkTreePath *path, gpointer statusbar) {
	// Get model from tree view
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
	
	// Get iter from path
	GtkTreeIter iter;
	gtk_tree_model_get_iter(model, &iter, path);
	
	// Get title and link values from iter
	gchar *comment = NULL;
	gchar *file = NULL;
	gchar *download = NULL;
	gtk_tree_model_get(model,
	                   &iter, 
	                   PLAYLIST_COMMENT_COLUMN, 
	                   &comment,
	                   PLAYLIST_FILE_COLUMN,
	                   &file, 
	                   PLAYLIST_DOWNLOAD_COLUMN,
	                   &download,
	                   -1);
	
	if(file != NULL) {
		playOrDownload(file, download);
	}
	
	g_free(comment);
	g_free(file);
	g_free(download);                   
}

void resultFunc(GtkIconView *icon_view, GtkTreePath *path, gpointer data) {
	// Get model from ivResults
	GtkTreeModel *model = gtk_icon_view_get_model(GTK_ICON_VIEW(ivResults));
	
	// Get iter from path
	GtkTreeIter iter;
	gtk_tree_model_get_iter(model, &iter, path);
	
	// Get title and href value from iter
	gchar *resultTitle = NULL;
	gchar *href = NULL;
	gtk_tree_model_get(model,
	                   &iter, 
	                   ICON_TITLE_COLUMN,
	                   &resultTitle,
	                   ICON_HREF,
	                   &href,
	                   -1);
	
	if(gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(rbActors))){
		// Fetch actors
		prevActors = actors;
		actors.setTitle(resultTitle);
		actors.setUrl(href); 
        g_thread_pool_push(actorsThreadPool, (gpointer)1, NULL);
	}else {
		// Fetch playlists/playItem
		// Set playlists title before playlists task
		string title = PROG_NAME + " - " + resultTitle;
	    gtk_window_set_title(GTK_WINDOW(window), title.c_str());
	    
	    g_thread_pool_push(playlistsThreadPool, href, NULL);
	}
	
	g_free(resultTitle);
}

void resultActivated(GtkWidget *widget, gpointer data) {
	gtk_icon_view_selected_foreach(GTK_ICON_VIEW(widget), resultFunc, NULL);
}

GtkWidget *createTreeView(void) {
	GtkTreeViewColumn *col;
	GtkCellRenderer *renderer;
	GtkWidget *view;
	
	view = gtk_tree_view_new();
	
	renderer = gtk_cell_renderer_pixbuf_new();
	col = gtk_tree_view_column_new_with_attributes ("Image", renderer,
                                                      "pixbuf", IMAGE_COLUMN,
                                                      NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(view), col);
	
	renderer = gtk_cell_renderer_text_new();
	col = gtk_tree_view_column_new_with_attributes ("Title", renderer,
                                                      "text", TITLE_COLUMN,
                                                      NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(view), col);
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(view), FALSE);
	return view;
}

void showSpCategories() {
	gtk_widget_set_visible(vbLeft, TRUE);
	gtk_widget_set_visible(swLeftTop, FALSE);
	gtk_widget_set_visible(hbCategoriesError, FALSE);
	gtk_widget_set_visible(spCategories, TRUE);
	gtk_spinner_start(GTK_SPINNER(spCategories));
}

void showCategoriesError() {
	gtk_widget_set_visible(vbLeft, TRUE);
	gtk_widget_set_visible(swLeftTop, FALSE);
	gtk_widget_set_visible(hbCategoriesError, TRUE);
	gtk_widget_set_visible(spCategories, FALSE);
	gtk_spinner_stop(GTK_SPINNER(spCategories));
}

gpointer categoriesTask(gpointer arg) {
	// On pre execute
	gdk_threads_enter();
	showSpCategories();
	gdk_threads_leave();
	string page = HtmlString::getCategoriesPage();
	gdk_threads_enter();
	if(!page.empty()) {
		CategoriesParser categoriesParser(page);
		
		gtk_widget_set_visible(hbCategoriesError, FALSE);
		gtk_widget_set_visible(spCategories, FALSE);
		gtk_spinner_stop(GTK_SPINNER(spCategories));
		gtk_widget_set_visible(swLeftTop, TRUE);
		
		GtkTreeModel *model;
		model = categoriesParser.getModel();
	    gtk_tree_view_set_model(GTK_TREE_VIEW(tvCategories), model);
		g_object_unref(model);
	}else {
		showCategoriesError();
	}
	gdk_threads_leave();
	return NULL;
}

static void btnCategoriesClicked(GtkWidget *widget,
                      gpointer  data)
{
	if(!gtk_widget_get_visible(vbLeft)) { // Categories hidden
		//Get categories model
		GtkTreeModel *model = gtk_tree_view_get_model(
		                          GTK_TREE_VIEW(tvCategories));
		if(model == NULL) {
			//Starting new thread to get categories from the net  
            g_thread_new(NULL, categoriesTask, NULL);
		}else {
			gtk_widget_set_visible(vbLeft, TRUE);
		}
	}else { // Categories visible
		gtk_widget_set_visible(vbLeft, FALSE);
	}	
}

static void btnUpClicked( GtkWidget *widget,
                      gpointer   data )
{
	ResultsHistory *resultsHistory = (ResultsHistory *)data;
	switchToIconView();
	resultsHistory->updateTitle();
	resultsHistory->updatePrevNextButtons();
	setSensitiveItemsResults();
}

static void btnPrevClicked( GtkWidget *widget,
                      gpointer   data )
{   
	ResultsHistory *resultsHistory = (ResultsHistory *)data;
	// If results repeat button not displayed
	if(!gtk_widget_get_visible(hbResultsError)) {
		// Save current results to forwardResultsStack
		resultsHistory->saveToForwardStack();
	}else {
		switchToIconView();
	}
	
	// Display top results from backResultsStack
	resultsHistory->restoreFromBackStack();
	// New images for new indexes will be downloaded
	imageIndexes.clear();
	setSensitiveItemsResults();		  
}

static void btnNextClicked( GtkWidget *widget,
                      gpointer   data)
{   
	ResultsHistory *resultsHistory = (ResultsHistory *)data;
	// If results repeat button not displayed
	if(!gtk_widget_get_visible(hbResultsError)) {
		// Save current results to backResultsStack
		resultsHistory->saveToBackStack();
	}else {
		// If repeat button displayed
	    switchToIconView();
	}
	
    // Display top result from forwardResultsStack
    resultsHistory->restoreFromForwardStack();
    // New images for new indexes will be downloaded
	imageIndexes.clear();
    setSensitiveItemsResults();
}

static void entryActivated( GtkWidget *widget, 
                      gpointer data) {
    ResultsHistory *resultsHistory = (ResultsHistory *)data;
    string query(gtk_entry_get_text(GTK_ENTRY(widget)));
    if(!query.empty()) {
		resultsHistory->saveToBackStack();
	    string title = "Search: " + query;
	    resultsHistory->setTitle(title);
	    resultsHistory->updateTitle();
	    string base_url = string(DOMAIN) + 
	         "/?do=search&subaction=search&mode=simple&story=" + 
	         to_cp1251(query);
		resultsHistory->setBaseUrl(base_url);
		resultsHistory->setUrl(base_url);
	    g_thread_pool_push(resultsNewThreadPool, (gpointer)1, NULL);
	}		  						  
}

static void rbActorsClicked(GtkWidget *widget, gpointer data) {
	//Toggle visibility of actors list (vbRight)
	if(!gtk_widget_get_visible(vbRight)) {
		if(actors.getCount() > 0 && 
		  gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(rbActors))) {
			gtk_widget_set_visible(vbRight, TRUE);
		}
	}else {
		gtk_widget_set_visible(vbRight, FALSE);
	}
}

static void backActorsChanged(GtkWidget *widget, gpointer data) {
	GtkTreeIter iter;
	GtkTreeModel *model;
	gchar *value;
	
	if (gtk_tree_selection_get_selected(
	    GTK_TREE_SELECTION(widget), &model, &iter)) {
	
		gtk_tree_model_get(model, &iter, TITLE_COLUMN, &value,  -1);
		if(backActors.count(actors.getTitle()) == 0) {
			backActors[actors.getTitle()] = actors; // save prev actors
			backActorsListAdd(actors.getTitle());
		}
		actors = backActors[string(value)];// set new actors
		updateActors();
		g_free(value);
    }
}

static void btnCategoriesRepeatClicked(GtkWidget *widget, gpointer data) {
	//Starting new thread to get categories from the net  
    g_thread_new(NULL, categoriesTask, NULL);
}

static void btnActorsRepeatClicked(GtkWidget *widget, gpointer data) {
	g_thread_pool_push(actorsThreadPool, 
		              (gpointer) 1,
		               NULL);
}

gboolean iconViewExposed(GtkWidget *widget, GdkEvent *event, gpointer data) {
	displayRange();
	return FALSE;
}

void swIconVScrollChanged(GtkAdjustment* adj, gpointer data) {
	ResultsHistory *resultsHistory = (ResultsHistory *)data;
	gdouble value = gtk_adjustment_get_value(adj);
	gdouble upper = gtk_adjustment_get_upper(adj);
	gdouble page_size = gtk_adjustment_get_page_size(adj);
	gdouble max_value = upper - page_size - page_size;
	if (value > max_value) {
		if(!resultsHistory->getNextLink().empty()) {
			// Search for the same link only once if it's not saved in set.
			if(!resultsHistory->threadLinksContainNextLink()) {
				resultsHistory->insertNextLinkToThreadLinks();
				g_thread_pool_push(resultsAppendThreadPool, (gpointer)1, NULL);
			}
		}
	}
}

static void btnRefreshClicked(GtkWidget *widget, gpointer data) {
	ResultsHistory *resultsHistory = (ResultsHistory *)data;
	resultsHistory->setRefresh(TRUE);
	g_thread_pool_push(resultsNewThreadPool, (gpointer)1, NULL);
}

static void btnResultsRepeatClicked(GtkWidget *widget, gpointer data) {
	ResultsHistory *resultsHistory = (ResultsHistory *)data;
	if(resultsHistory->isError()) {
		// Update results
	    g_thread_pool_push(resultsNewThreadPool, (gpointer)1, NULL);
	    resultsHistory->setError(FALSE);
	}else {
		// Update playlists
	    g_thread_pool_push(playlistsThreadPool, (gpointer)"", NULL);
	}
}

int main( int   argc,
          char *argv[] )
{   
    GtkWidget *vbox;
    GtkWidget *toolbar; 
    GtkWidget *hbCenter;    
    GtkWidget *swRightTop, *swRightBottom;
    GtkWidget *btnCategoriesError;
    GtkWidget *btnActorsError;
    
	GtkToolItem *btnCategories;
	GtkToolItem *sep;
	GtkToolItem *exit;
	
	GdkPixbuf *icon;
	
	GtkTreeSelection *selection; 
	
	 /* Must initialize libcurl before any threads are started */ 
    curl_global_init(CURL_GLOBAL_ALL);
    
    gdk_threads_init ();
    gdk_threads_enter ();
    
    gtk_init(&argc, &argv);
    
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_window_set_title(GTK_WINDOW(window), PROG_NAME.c_str());
    gtk_container_set_border_width(GTK_CONTAINER(window), 5);
    icon = create_pixbuf("online_life.png");
	gtk_window_set_icon(GTK_WINDOW(window), icon);
    
    vbox = gtk_vbox_new(FALSE, 1);
    
    tvPlaylists = createTreeView();
    
    // set model to ivResults
    // it's kind of not needed but it removes some error
    GtkTreeModel *model = GTK_TREE_MODEL(gtk_list_store_new(
	     ICON_NUM_COLS,   // Number of columns
	     GDK_TYPE_PIXBUF, // Image poster
	     G_TYPE_STRING,   // Title
	     G_TYPE_STRING,   // Href
	     G_TYPE_STRING    // Image link
    ));
    
    ivResults = gtk_icon_view_new_with_model(model);
    gtk_icon_view_set_pixbuf_column(GTK_ICON_VIEW(ivResults), ICON_IMAGE_COLUMN);                                                  
    gtk_icon_view_set_text_column(GTK_ICON_VIEW(ivResults), ICON_TITLE_COLUMN);
    gtk_icon_view_set_item_width(GTK_ICON_VIEW(ivResults), 180);
    
	g_object_unref(model);
	
	btnPrev = gtk_tool_button_new_from_stock(GTK_STOCK_GO_BACK);
	btnNext = gtk_tool_button_new_from_stock(GTK_STOCK_GO_FORWARD);
	
	ResultsHistory *resultsHistory = new ResultsHistory(window,
	                                                    ivResults,
	                                                    btnPrev,
	                                                    btnNext,
	                                                    PROG_NAME);
    
    toolbar = gtk_toolbar_new();
	gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_ICONS);
	gtk_container_set_border_width(GTK_CONTAINER(toolbar), 2);
	
    btnCategories = gtk_tool_button_new_from_stock(GTK_STOCK_DIRECTORY);
    gtk_tool_item_set_tooltip_text(btnCategories, "Display categories");
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), btnCategories, -1);
    g_signal_connect(GTK_WIDGET(btnCategories),
                     "clicked", 
                     G_CALLBACK(btnCategoriesClicked),
                     NULL);
        
    sep = gtk_separator_tool_item_new();
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), sep, -1);
	
	btnRefresh = gtk_tool_button_new_from_stock(GTK_STOCK_REFRESH);
    gtk_tool_item_set_tooltip_text(btnRefresh, "Update results");
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), btnRefresh, -1);
    g_signal_connect(GTK_WIDGET(btnRefresh), 
                     "clicked", 
                     G_CALLBACK(btnRefreshClicked), 
                     resultsHistory);
        
    sep = gtk_separator_tool_item_new();
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), sep, -1);
    
    btnUp = gtk_tool_button_new_from_stock(GTK_STOCK_GO_UP);
    gtk_tool_item_set_tooltip_text(btnUp, "Move up");
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), btnUp, -1);
    g_signal_connect(GTK_WIDGET(btnUp),
                     "clicked", 
                     G_CALLBACK(btnUpClicked), 
                     resultsHistory);
    
    gtk_tool_item_set_tooltip_text(btnPrev, "Go back in history");
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), btnPrev, -1);
    g_signal_connect(btnPrev,
                     "clicked", 
                     G_CALLBACK(btnPrevClicked),
                     resultsHistory);
    
    gtk_tool_item_set_tooltip_text(btnNext, "Go forward in history");
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), btnNext, -1);
    g_signal_connect(GTK_WIDGET(btnNext),
                     "clicked", 
                     G_CALLBACK(btnNextClicked),
                     resultsHistory);
    
    sep = gtk_separator_tool_item_new();
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), sep, -1);
    
    entry = gtk_entry_new();
    gtk_widget_set_tooltip_text(entry, "Search online-life");
	GtkToolItem *entryItem = gtk_tool_item_new();
	gtk_container_add(GTK_CONTAINER(entryItem), entry);
    g_signal_connect(entry,
                     "activate", 
                     G_CALLBACK(entryActivated), 
                     resultsHistory);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), entryItem, -1);
    
    sep = gtk_separator_tool_item_new();
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), sep, -1);
    
    rbActors = gtk_radio_tool_button_new_from_stock(NULL, "gtk-info");
    rbPlay = gtk_radio_tool_button_new_with_stock_from_widget(
        GTK_RADIO_TOOL_BUTTON(rbActors), "gtk-media-play");
    rbDownload = gtk_radio_tool_button_new_with_stock_from_widget(
        GTK_RADIO_TOOL_BUTTON(rbPlay), "gtk-goto-bottom");
        
    gtk_tool_item_set_tooltip_text(rbActors, "Process item: show actors");
    gtk_tool_item_set_tooltip_text(rbPlay, "Process item: play");
    gtk_tool_item_set_tooltip_text(rbDownload, "Process item: download");
    
    // Default is "play"
    gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(rbPlay), TRUE); 
    
    sep = gtk_separator_tool_item_new();
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), sep, -1);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), rbPlay, -1);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), rbDownload, -1);
    sep = gtk_separator_tool_item_new();
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), sep, -1);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), rbActors, -1);
    
    sep = gtk_separator_tool_item_new();
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), sep, -1);
	
	exit = gtk_tool_button_new_from_stock(GTK_STOCK_QUIT);
	gtk_tool_item_set_tooltip_text(exit, "Quit program");
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), exit, -1);
	
	g_signal_connect(G_OBJECT(exit), "clicked",
	    G_CALLBACK(gtk_main_quit), NULL);
    
    disableAllItems();
    
    gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, FALSE, 1);
    
    swTree = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(swTree),
            GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(swTree),
            GTK_SHADOW_ETCHED_IN);
            
    swIcon = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(swIcon),
            GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(swIcon),
            GTK_SHADOW_ETCHED_IN);
    
    gtk_container_add(GTK_CONTAINER(swTree), tvPlaylists);
    gtk_container_add(GTK_CONTAINER(swIcon), ivResults);
    
    tvBackActors = createTreeView();
    // Set up store
    GtkListStore *store 
        = gtk_list_store_new(NUM_COLS, GDK_TYPE_PIXBUF, G_TYPE_STRING);    
    gtk_tree_view_set_model(GTK_TREE_VIEW(tvBackActors), 
        GTK_TREE_MODEL(store));
	g_object_unref(store);
	
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tvBackActors));

	g_signal_connect(selection, "changed", 
	  G_CALLBACK(backActorsChanged), NULL);
    
    tvCategories = createTreeView();
    tvActors = createTreeView();
    
    swLeftTop = gtk_scrolled_window_new(NULL, NULL);
    swRightTop = gtk_scrolled_window_new(NULL, NULL);
    swRightBottom = gtk_scrolled_window_new(NULL, NULL);
    
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(swLeftTop),
            GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(swLeftTop),
            GTK_SHADOW_ETCHED_IN);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(swRightTop),
            GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(swRightTop),
            GTK_SHADOW_ETCHED_IN);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(swRightBottom),
            GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(swRightBottom),
            GTK_SHADOW_ETCHED_IN);
    
    vbLeft = gtk_vbox_new(FALSE, 1);
    vbRight = gtk_vbox_new(FALSE, 1);
    hbCenter = gtk_hbox_new(FALSE, 1);
    
    const int SIDE_SIZE = 220;
    gtk_widget_set_size_request(vbLeft, SIDE_SIZE, -1);
    gtk_widget_set_size_request(vbRight, SIDE_SIZE, -1);
    
    // Scroll containers
    gtk_container_add(GTK_CONTAINER(swLeftTop), tvCategories);
    gtk_container_add(GTK_CONTAINER(swRightTop), tvActors);
    gtk_container_add(GTK_CONTAINER(swRightBottom), tvBackActors);
    
    // Frames
    frRightTop = gtk_frame_new("Actors");
    frRightBottom = gtk_frame_new("Actors history");
    
    // Add categories spinner and error
    //vbLeft = gtk_vbox_new(FALSE, 1);
    hbCategoriesError = gtk_hbox_new(FALSE, 1); // for Repeat button normal size
    spCategories = gtk_spinner_new();
    btnCategoriesError = gtk_button_new_with_label("Repeat");
    g_signal_connect(btnCategoriesError, 
                     "clicked", 
                     G_CALLBACK(btnCategoriesRepeatClicked), 
                     NULL);
    gtk_box_pack_start(GTK_BOX(hbCategoriesError), btnCategoriesError, TRUE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(vbLeft), swLeftTop, TRUE, TRUE, 1);
    gtk_box_pack_start(GTK_BOX(vbLeft), spCategories, TRUE, FALSE, 1);
    gtk_box_pack_start(GTK_BOX(vbLeft), hbCategoriesError, TRUE, FALSE, 1);
    
    gtk_container_add(GTK_CONTAINER(frRightTop), swRightTop);
    gtk_container_add(GTK_CONTAINER(frRightBottom), swRightBottom);
    
    // Movie info
    frInfo = gtk_frame_new("Info");
    lbInfo = gtk_label_new("");
    gtk_widget_set_size_request(lbInfo, SIDE_SIZE, -1);
    gtk_label_set_line_wrap(GTK_LABEL(lbInfo), TRUE);
    gtk_container_add(GTK_CONTAINER(frInfo), lbInfo);
    gtk_box_pack_start(GTK_BOX(vbRight), frInfo, FALSE, FALSE, 1);
    gtk_box_pack_start(GTK_BOX(vbRight), frRightTop, TRUE, TRUE, 1);
    
    // Actors spinner and repeat button
    hbActorsError = gtk_hbox_new(FALSE, 1);
    spActors = gtk_spinner_new();
    btnActorsError = gtk_button_new_with_label("Repeat");
    g_signal_connect(btnActorsError, "clicked",
        G_CALLBACK(btnActorsRepeatClicked), NULL);
    gtk_box_pack_start(GTK_BOX(hbActorsError), btnActorsError, TRUE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(vbRight), spActors, TRUE, FALSE, 1);
    gtk_box_pack_start(GTK_BOX(vbRight), hbActorsError, TRUE, FALSE, 1);
    gtk_widget_set_size_request(spActors, 32, 32);
        
    //vbRight
    gtk_box_pack_start(GTK_BOX(vbRight), frRightBottom, TRUE, TRUE, 1);
    
    // Add center spinner
    spCenter = gtk_spinner_new();
    
    // Add results repeat button
    hbResultsError = gtk_hbox_new(FALSE, 1);
    btnResultsError = gtk_button_new_with_label("Repeat");
    gtk_box_pack_start(
        GTK_BOX(hbResultsError), 
        btnResultsError, 
        TRUE, 
        FALSE, 
        10);
    g_signal_connect(
        btnResultsError,
        "clicked",
        G_CALLBACK(btnResultsRepeatClicked),
        resultsHistory);
    
    // add vbox center
    vbCenter = gtk_vbox_new(FALSE, 1);
    // add items to vbCenter
    gtk_box_pack_start(GTK_BOX(vbCenter), swTree, TRUE, TRUE, 1);
    gtk_box_pack_start(GTK_BOX(vbCenter), swIcon, TRUE, TRUE, 1);
    gtk_box_pack_start(GTK_BOX(vbCenter), spCenter, TRUE, FALSE, 1);
    gtk_box_pack_start(GTK_BOX(vbCenter), hbResultsError, TRUE, FALSE, 1);
    
    //hbCenter
    gtk_box_pack_start(GTK_BOX(hbCenter), vbLeft, FALSE, FALSE, 1);
    gtk_box_pack_start(GTK_BOX(hbCenter), vbCenter, TRUE, TRUE, 1);
    gtk_box_pack_start(GTK_BOX(hbCenter), vbRight, FALSE, FALSE, 1);
    
    gtk_box_pack_start(GTK_BOX(vbox), hbCenter, TRUE, TRUE, 1);
    
    g_signal_connect(tvPlaylists, "row-activated", 
        G_CALLBACK(playlistClicked), NULL);
        
    g_signal_connect(tvCategories,
                     "row-activated",
                     G_CALLBACK(categoriesClicked), 
                     resultsHistory);
        
    g_signal_connect(tvActors, 
                     "row-activated",
                     G_CALLBACK(actorsClicked), 
                     resultsHistory);
        
    g_signal_connect(ivResults, "item-activated", 
        G_CALLBACK(resultActivated), NULL);  
    
    g_signal_connect(ivResults, "expose-event",
        G_CALLBACK(iconViewExposed), NULL);
    
    gtk_container_add(GTK_CONTAINER(window), vbox);
    
    gtk_window_set_default_size(GTK_WINDOW(window), 700, 400);
    
    gtk_widget_show_all(window);
    
    g_signal_connect(GTK_WIDGET(rbActors), "clicked", 
        G_CALLBACK(rbActorsClicked), NULL);
    
    gtk_widget_set_visible(vbLeft, FALSE);
    gtk_widget_set_visible(vbRight, FALSE);
    
    gtk_widget_set_visible(swTree, FALSE);
    gtk_widget_set_visible(frRightBottom, FALSE);
    
    gtk_widget_set_visible(spCenter, FALSE);
    gtk_widget_set_size_request(spCenter, 32, 32);
    
    gtk_widget_set_visible(spCategories, FALSE);
    gtk_widget_set_visible(hbCategoriesError, FALSE);
    gtk_widget_set_size_request(spCategories, 32, 32);
    
    gtk_widget_set_visible(spActors, FALSE);
    gtk_widget_set_visible(hbActorsError, FALSE);
    
    gtk_widget_set_visible(hbResultsError, FALSE);
    
    GtkTreeStore *playlistsStore = gtk_tree_store_new(PLAYLIST_NUM_COLS, 
						                              GDK_TYPE_PIXBUF,
									                  G_TYPE_STRING,
									                  G_TYPE_STRING,
									                  G_TYPE_STRING);
    gtk_tree_view_set_model(GTK_TREE_VIEW(tvPlaylists),
                            GTK_TREE_MODEL(playlistsStore));
    g_object_unref(playlistsStore);
    
    Playlists *playlists = new Playlists(
        gtk_tree_view_get_model(GTK_TREE_VIEW(tvPlaylists)),
        resultsHistory
    );
    
    // GThreadPool for downloading images
    imagesThreadPool = g_thread_pool_new(imageDownloadTask,
                                   NULL, 
                                   -1,
                                   FALSE,
                                   NULL);
                                   
    // GThreadPool for new results
    resultsNewThreadPool = g_thread_pool_new(resultsNewTask,
                                   resultsHistory,
                                   1, // Run one thread at the time
                                   FALSE,
                                   NULL);
                                   
    // GThreadPool for results pages
    resultsAppendThreadPool = g_thread_pool_new(resultsAppendTask,
                                   resultsHistory,
                                   1, // Run one thread at the time
                                   FALSE,
                                   NULL);
                                   
    // GThreadPool for actors
    actorsThreadPool = g_thread_pool_new(actorsTask,
                                   NULL,
                                   1, // Run one thread at the time
                                   FALSE,
                                   NULL);
                                   
    // GThreadPool for playlists
    playlistsThreadPool = g_thread_pool_new(playlistsTask,
                                   playlists,
                                   1, // Run one thread at the time
                                   FALSE,
                                   NULL);
                                   
    // IconView scroll to the bottom detection code
    GtkAdjustment *vadjustment;
    vadjustment = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(swIcon));
    g_signal_connect(vadjustment, "value-changed",
        G_CALLBACK(swIconVScrollChanged), resultsHistory);
        
    // Initialize default pixbuf for ivResults here
    defaultPixbuf = create_pixbuf("blank.png");
    
    gtk_main();
    
    gdk_threads_leave ();
    
    g_free(playlists);
    g_free(resultsHistory);
 
    /* we're done with libcurl, so clean it up */ 
	curl_global_cleanup();

    return 0;
}
