#include <gtk/gtk.h>
#include <vector>
#include <string>
#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <string.h>
#include <map>
#include <set>

bool curlCategoriesStop, curlResultsStop, curlActorsStop, curlStop;

#define DOMAIN "http://online-life.club"
#define WDOMAIN "http://www.online-life.club/"

#include "Converter.hpp"
#include "DisplayMode.hpp"
#include "HtmlString.hpp"
#include "CreatePixbuf.hpp"
#include "ColumnsEnum.hpp"

GdkPixbuf *defaultPixbuf;
map<string, GdkPixbuf*> imagesCache;
GtkWidget *iconView;

#include "Categories.hpp"
#include "Results.hpp"
#include "Playlists.hpp"
#include "Actors.hpp"

using namespace std;

//Categories
Categories categories;
//Results
Results results, prevResults;
//Playlists
Playlists playlists;
//Actors
Actors actors, prevActors;

map <string, Results> backResults;
map <string, Actors> backActors;

vector<Results> backResultsStack, forwardResultsStack;

GtkWidget *frRightBottom, *frLeftTop, *frLeftBottom;
GtkWidget *lbInfo;
GtkWidget *frRightTop, *frInfo;

DisplayMode displayMode;

string title;
const string PROG_NAME("Online life");

GtkWidget *treeView;

GtkToolItem *btnUp;
GtkToolItem *btnPrev;
GtkToolItem *btnNext;
GtkToolItem *btnHistory;

GtkToolItem *rbActors;
GtkToolItem *rbPlay;
GtkToolItem *rbDownload;

GtkWidget *window;
GtkWidget *entry;

GtkWidget *swTree, *swIcon;
GtkWidget *tvBackResults, *tvCategories, *tvActors, *tvBackActors;
GtkWidget *vbLeft, *vbRight;

GtkWidget *spCenter;

GtkWidget *spCategories;
GtkWidget *hbCategoriesError;

GtkWidget *swLeftTop;

GtkWidget *spActors;
GtkWidget *hbActorsError;

GtkWidget *vbCenter;

GThreadPool *imagesThreadPool;
GThreadPool *resultsThreadPool;
GThreadPool *actorsThreadPool;
GThreadPool *playlistsThreadPool;

string lastActorsHref;

bool isPage;
set<string> resultsThreadsLinks;

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
	gtk_spinner_stop(GTK_SPINNER(spCenter));
}

void switchToIconView() {
	gtk_widget_set_visible(swTree, FALSE);
	gtk_widget_set_visible(swIcon, TRUE);
	gtk_widget_set_visible(spCenter, FALSE);
	gtk_spinner_stop(GTK_SPINNER(spCenter));
}

void disableAllItems() {
	gtk_widget_set_sensitive(GTK_WIDGET(btnUp), FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(btnPrev), FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(btnNext), FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(btnHistory), FALSE);
	
	gtk_widget_set_sensitive(GTK_WIDGET(rbActors), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(rbPlay), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(rbDownload), FALSE);
}

void setSensitiveItemsPlaylists() {
	gtk_widget_set_sensitive(GTK_WIDGET(btnUp), TRUE);
	gtk_widget_set_sensitive(GTK_WIDGET(btnPrev), FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(btnNext), FALSE);
	
	gtk_widget_set_sensitive(GTK_WIDGET(rbPlay), TRUE);
    gtk_widget_set_sensitive(GTK_WIDGET(rbDownload), TRUE);
    gtk_widget_set_sensitive(GTK_WIDGET(rbActors), FALSE);
}

void setSensitiveItemsResults() {
	if(backResults.size() > 0) {
		gtk_widget_set_sensitive(GTK_WIDGET(btnHistory), TRUE);
	}
	
	gtk_widget_set_sensitive(GTK_WIDGET(btnUp), FALSE);

	if(backResultsStack.empty()) {
		gtk_widget_set_sensitive(GTK_WIDGET(btnPrev), FALSE);
	}else {
		gtk_widget_set_sensitive(GTK_WIDGET(btnPrev), TRUE);
	}
	
	if(forwardResultsStack.empty()) {
		gtk_widget_set_sensitive(GTK_WIDGET(btnNext), FALSE);
	}else {
		gtk_widget_set_sensitive(GTK_WIDGET(btnNext), TRUE);
	}
	
	gtk_widget_set_sensitive(GTK_WIDGET(rbPlay), TRUE);
    gtk_widget_set_sensitive(GTK_WIDGET(rbDownload), TRUE);
    gtk_widget_set_sensitive(GTK_WIDGET(rbActors), TRUE);
}

void setSensitiveItemsActors() {
	gtk_widget_set_sensitive(GTK_WIDGET(btnUp), TRUE);
	gtk_widget_set_sensitive(GTK_WIDGET(btnPrev), FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(btnNext), FALSE);
	
	gtk_widget_set_sensitive(GTK_WIDGET(rbActors), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(rbPlay), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(rbDownload), FALSE);
}

set<int> imageIndexes;

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
			(GTK_ICON_VIEW(iconView)));
		gtk_list_store_set(store, &args->iter, IMAGE_COLUMN, pixbufOrig, -1);  
		gdk_threads_leave();
	}
	
	return realsize;
}

void getPixbufFromUrl(string url, GtkTreeIter iter) {
    
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
	}else { 
        GError *error = NULL;
		gboolean ok = gdk_pixbuf_loader_close(loader, &error);
		if(!ok) {
	        fprintf(stderr, "On close: %s\n", error->message);
            g_error_free(error);
		}else {
			// Setting new fully downloaded image here
            //Make copy of pixbuf to be able to free loader
			pixbuf = GDK_PIXBUF(g_object_ref(gdk_pixbuf_loader_get_pixbuf(loader)));
			gdk_threads_enter();
			GtkListStore *store;
			imagesCache[url] = pixbuf;
				
			store = GTK_LIST_STORE(gtk_icon_view_get_model
				(GTK_ICON_VIEW(iconView))); 
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
	string link((char*)arg);
	
	gdk_threads_enter();
	int count = imagesCache.count(link);
	int itersCount = results.getIters().count(link);
	
	// get stored iter by link in iconView
	GtkTreeIter iter = results.getIters()[link];
	gdk_threads_leave();
	//if map does not have pixbuf and has iter with link
	if(count == 0 && itersCount > 0) { 
		getPixbufFromUrl(link, iter);
	}
}

void saveResultsPostion() {
	GtkTreePath *path1, *path2;
	if(gtk_icon_view_get_visible_range(GTK_ICON_VIEW(iconView), &path1, &path2)) {
		string index(gtk_tree_path_to_string(path1));
        results.setIndex(index);
		gtk_tree_path_free(path1);
	    gtk_tree_path_free(path2);
	}
}

void displayRange() {
	GtkTreePath *path1, *path2;
	if(gtk_icon_view_get_visible_range(GTK_ICON_VIEW(iconView), &path1, &path2)) {
		gint *indices1 = gtk_tree_path_get_indices(path1);
		gint *indices2 = gtk_tree_path_get_indices(path2);
		gint index1 = indices1[0];
		gint index2 = indices2[0];
		for(int i = index1; i <= index2; i++) {
			if(imageIndexes.count(i) == 0) {
				imageIndexes.insert(i);
				g_thread_pool_push(imagesThreadPool, 
				    (gpointer) results.getResults()[i].get_image_link().c_str(),
				     NULL);
			}
		}
		gtk_tree_path_free(path1);
		gtk_tree_path_free(path2);
	}
}

void updateResults() {
	if (!isPage) {
		imageIndexes.clear();
	}
	if(displayMode != RESULTS) {
		displayMode = RESULTS;
	}
	switchToIconView();
	string title = PROG_NAME + " - " + results.getTitle();
	gtk_window_set_title(GTK_WINDOW(window), title.c_str());
	gtk_entry_set_text(GTK_ENTRY(entry), "");
	
	setSensitiveItemsResults();
}

void backResultsListAdd(string title) {
	GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(
	    GTK_TREE_VIEW(tvBackResults)));
	GtkTreeIter iter;
	GdkPixbuf *icon = create_pixbuf("link_16.png");
	
	gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, IMAGE_COLUMN, icon,
           TITLE_COLUMN, title.c_str(), -1);
}

void showSpCenter() {
	gtk_widget_set_visible(swTree, FALSE);
	// Show and hide of iconView depends on isPage
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
	gtk_spinner_start(GTK_SPINNER(spCenter));
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

void resultsTask(gpointer arg, gpointer arg1) {
	string link((char *)arg);
	//cout << "Link: " << link << endl;
	// On pre execute
	gdk_threads_enter();
	// Display spinner for new and paging results
    showSpCenter();
	if(!isPage) {
		// Save position and copy to save variable
		if(results.getResults().size() > 0) {
			saveResultsPostion();
		    prevResults = results;
		}
	}
	// Stop all other threads except results
	curlActorsStop = TRUE;
	curlCategoriesStop = TRUE;
	curlStop = TRUE;
	curlResultsStop = FALSE;
    gdk_threads_leave();
	string page = HtmlString::getResultsPage(link);
    gdk_threads_enter();
	if(!page.empty()) {
		// add back results
		// save prev results to map
		// add title to tvBackResults
		if(prevResults.getResults().size() > 0 && !isPage) {
			// Add first time to map, add to listView
			if(backResults.count(prevResults.getTitle()) == 0) { 
				backResultsListAdd(prevResults.getTitle());
			}
			backResults[prevResults.getTitle()] = prevResults;
			// also save prevResults to backResultsStack
			backResultsStack.push_back(prevResults);
			// clear forward results stack on fetching new results
			forwardResultsStack.clear();
		}
		if (!isPage) {
			// Scroll to the top of the list
		    GtkTreePath *path = gtk_tree_path_new_first();
		    gtk_icon_view_scroll_to_path(GTK_ICON_VIEW(iconView), path, FALSE, 0, 0);
		    
		    // Clear results links set if not paging
		    resultsThreadsLinks.clear();
		    results.setTitle(title);
		}
		results.clearResultsAndModel(isPage);
		results.getResultsPage(page);
		
		updateResults();
	}else {
		// Remove link from set on results error
		if(resultsThreadsLinks.count(results.getNextLink()) > 0) {
			resultsThreadsLinks.erase(results.getNextLink());
		}
		switchToIconView();
		show_error_dialog();
	}
	gdk_threads_leave();
}

void processCategory(gint *indices, gint count) {//move to results
    isPage = false;
	if(count == 1) { //Node
		gint i = indices[0];
		title = categories.getCategories()[i].get_title();
		g_thread_pool_push(resultsThreadPool,
		     (gpointer) categories.getCategories()[i].get_link().c_str(),
		     NULL);
	}else if(count == 2) { //Leaf
		gint i = indices[0];
		gint j = indices[1];
		title = categories.getCategories()[i].get_title() + " - " 
		      + categories.getCategories()[i].get_subctgs()[j].get_title();
		g_thread_pool_push(resultsThreadPool,
		     (gpointer) categories.getCategories()[i].get_subctgs()[j].get_link().c_str(),
		     NULL);
	}
	
}

string get_txt_link(string page) {
	string begin = " {";
	string end = "\"};";
	size_t json_begin = page.find(begin);
	size_t json_end = page.find(end);
	if(json_end != string::npos && json_begin != string::npos) {
		size_t json_length = json_end - json_begin;
		string json = page.substr(json_begin+2, json_length-2);
        
        // Find link
        size_t link_begin = json.find("pl:");
        size_t link_end = json.find("\"", link_begin+4);
        if(link_begin != string::npos && link_end != string::npos) {
			size_t link_length = link_end - link_begin;
			string link = json.substr(link_begin+4, link_length-4);
			return link;
		}
	}
	return "";
}

GtkTreeModel *getPlaylistModel() {
	GtkListStore *store;
    GtkTreeIter iter;
    
	GdkPixbuf *item = create_pixbuf("link_16.png");
    
    store = gtk_list_store_new(NUM_COLS, GDK_TYPE_PIXBUF, G_TYPE_STRING);
    for(unsigned i=0; i<playlists.getPlaylists()[0].get_items().size(); i++) {
		gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, IMAGE_COLUMN, item, TITLE_COLUMN,
             playlists.getPlaylists()[0].get_items()[i].get_comment().c_str(), -1);
	}
    
    return GTK_TREE_MODEL(store);
}

GtkTreeModel *getPlaylistsModel() {
	GtkTreeStore *treestore;
	GtkTreeIter topLevel, child;
	
	treestore = gtk_tree_store_new(NUM_COLS, GDK_TYPE_PIXBUF,
				  G_TYPE_STRING);
    
    GdkPixbuf *category = create_pixbuf("folder_16.png");
	GdkPixbuf *item = create_pixbuf("link_16.png");
	
	for(unsigned i=0; i<playlists.getPlaylists().size(); i++) {
		gtk_tree_store_append(treestore, &topLevel, NULL);
		gtk_tree_store_set(treestore, &topLevel,
		    IMAGE_COLUMN, category, TITLE_COLUMN,
		    playlists.getPlaylists()[i].get_title().c_str(), -1);
		for(unsigned j=0; j<playlists.getPlaylists()[i].get_items().size(); j++) {
			gtk_tree_store_append(treestore, &child, &topLevel);
			gtk_tree_store_set(treestore, &child,
			    IMAGE_COLUMN, item, TITLE_COLUMN, 
			    playlists.getPlaylists()[i].get_items()[j].get_comment().c_str(), -1);
		}
	}
	
	return GTK_TREE_MODEL(treestore);
}

void displayPlaylists() {
	switchToTreeView();
	displayMode = PLAYLISTS;
	gtk_window_set_title(GTK_WINDOW(window), playlists.getTitle().c_str());
	
	GtkTreeModel *model;
	if(playlists.getPlaylists().size() == 1 && playlists.getPlaylists()[0].get_title().empty()) {
		//Display single playlist
	    model = getPlaylistModel();
	}else {
		//Display multiple playlists
		model = getPlaylistsModel();
	}
	gtk_tree_view_set_model(GTK_TREE_VIEW(treeView), model);
	g_object_unref(model);
	
	setSensitiveItemsPlaylists();
}

void processPlayItem(PlayItem item) {
	if(!item.get_comment().empty()) {
	    if(gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(rbDownload))){
		    string command = "xterm -e wget -P ~/Download -c " + item.get_download() + " &";
	        system(command.c_str());
		}else if(gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(rbPlay))) {
		    string command = "xterm -e mplayer -cache 2048 " + item.get_file() + " &";
	        system(command.c_str());
		}	
	}
}

void processPlayItem(gint *indices, gint count) {
	if(count == 1) { //Node or List
		gint i = indices[0];
		if(playlists.getPlaylists().size() == 1) { //one playlist only
		    processPlayItem(playlists.getPlaylists()[0].get_items()[i]);	
		}
	}else if(count == 2) { //Leaf
		gint i = indices[0];
		gint j = indices[1];
		processPlayItem(playlists.getPlaylists()[i].get_items()[j]);
	}
}

string getTrailerId(string &page) {
	size_t begin = page.find("?trailer_id=");
	size_t end = page.find("' ", begin+13);
	if(begin != string::npos && end != string::npos) {
		size_t length = end - begin;
		string trailerId = page.substr(begin+12, length-12);
		return trailerId;
	}
	return "";
}

void playlistsTask(gpointer args, gpointer args2) {
	gint i = (gint)(glong)args;
	i--; // decrement index to restore it's value
	Item result = results.getResults()[i];
	string id = result.get_id();
	string url = "http://dterod.com/js.php?id=" + id;
	string referer = "http://dterod.com/player.php?newsid=" + id;
	// On pre execute
	gdk_threads_enter();
	showSpCenter();
	
	// Stop others threads
	curlActorsStop = TRUE;
	curlCategoriesStop = TRUE;
	curlStop = FALSE;
	curlResultsStop = TRUE;
	gdk_threads_leave();
	
	string js = HtmlString::getPage(url, referer);
	string playlist_link = get_txt_link(js);
	if(!playlist_link.empty()) { // Playlists found
		string json = HtmlString::getPage(playlist_link);
		gdk_threads_enter();
		playlists.parse(json);
		if(!playlists.getPlaylists().empty()) {
			displayPlaylists();
		}else {
			show_error_dialog();
		}
		gdk_threads_leave();
	}else { //PlayItem found
		gdk_threads_enter();
		// get results list back
		switchToIconView();
		PlayItem playItem = playlists.parse_play_item(js);
		if(!playItem.get_comment().empty()) {
			processPlayItem(playItem); 
		}else {
			if(results.getTitle().find("Трейлеры") != string::npos) {
				gdk_threads_leave();
				// Searching for alternative trailers links
	            string infoHtml = HtmlString::getPage(result.get_href(), referer);
	            string trailerId = getTrailerId(infoHtml); 
	            url = "http://dterod.com/js.php?id=" + trailerId + "&trailer=1";
	            referer = "http://dterod.com/player.php?trailer_id=" + trailerId;
	            string json = HtmlString::getPage(url, referer);
				gdk_threads_enter();
				processPlayItem(playlists.parse_play_item(json)); 
			}else {
			    show_error_dialog();
			}
		}
		gdk_threads_leave();
	}
}

GtkTreeModel *getActorsModel() {
	GtkListStore *store;
    GtkTreeIter iter;
    
	GdkPixbuf *item = create_pixbuf("link_16.png");
    
    store = gtk_list_store_new(NUM_COLS, GDK_TYPE_PIXBUF, G_TYPE_STRING);
    for(unsigned i=0; i < actors.getActors().size(); i++) {
		gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, IMAGE_COLUMN, item,
           TITLE_COLUMN, actors.getActors()[i].get_title().c_str(), -1);
	}
    
    return GTK_TREE_MODEL(store);
}

void updateActors() {
	gtk_label_set_text(GTK_LABEL(lbInfo), actors.getTitle().c_str());
	GtkTreeModel *model;
	model = getActorsModel();
    gtk_tree_view_set_model(GTK_TREE_VIEW(tvActors), model);
	g_object_unref(model);
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
	string link((char*)args);
	// On pre execute
	gdk_threads_enter();
	showSpActors();
	// Stop others threads
	curlActorsStop = FALSE;
	curlCategoriesStop = TRUE;
	curlStop = TRUE;
	curlResultsStop = TRUE;
	gdk_threads_leave();
	string page = HtmlString::getActorsPage(link);
	gdk_threads_enter();
	if(!page.empty()) {
		actors.parse(page);
		if(backActors.count(prevActors.getTitle()) == 0 
		        && prevActors.getActors().size() > 0) {
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

void processResult(gint *indices, gint count) {//move to playlists
	if(count == 1) { //Node
		gint i = indices[0];
		title = PROG_NAME + " - " + results.getResults()[i].get_title();
		if(gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(rbActors))){
			lastActorsHref = results.getResults()[i].get_href();
			// Fetch actors
			prevActors = actors;
			actors.setTitle(results.getResults()[i].get_title());
	        g_thread_pool_push(actorsThreadPool,
	            (gpointer) results.getResults()[i].get_href().c_str(),
	            NULL);
		}else {
			// Fetch playlists/playItem
			playlists.setTitle(title);
			i++; // increment index as cast 0 to pointer gives NULL and error at 0 index
		    g_thread_pool_push(playlistsThreadPool, GINT_TO_POINTER(i), NULL);	
		}
	}
}

void processActor(gint *indices, gint count) {
	if(count == 1) { //Node
	    isPage = false;
		gint i = indices[0];
		title = actors.getActors()[i].get_title();
	    g_thread_pool_push(resultsThreadPool,
	        (gpointer) actors.getActors()[i].get_href().c_str(),
	        NULL);	  	
	}
}

struct IndicesCount {
	gint *indices;
	gint count;
};

IndicesCount getIndicesCount(GtkWidget *widget) {
	GtkTreeIter iter;
	GtkTreeModel *model;
	GtkTreePath *path;
	GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
	IndicesCount inCount;
	if (gtk_tree_selection_get_selected(GTK_TREE_SELECTION(selection), &model, &iter)) {
        path = gtk_tree_model_get_path(model, &iter);
		inCount.indices = gtk_tree_path_get_indices_with_depth(path, &inCount.count);
	}
	return inCount;
}

void categoriesClicked(GtkWidget *widget, gpointer data) {
	IndicesCount inCount = getIndicesCount(widget);
	if(inCount.indices != NULL) {
		processCategory(inCount.indices, inCount.count);
	}
}

void actorsClicked(GtkWidget *widget, gpointer data) {
	IndicesCount inCount = getIndicesCount(widget);
	if(inCount.indices != NULL) {
		processActor(inCount.indices, inCount.count);
	}
}

void on_changed(GtkWidget *widget, gpointer statusbar) {
    IndicesCount inCount = getIndicesCount(widget);
	if(inCount.indices != NULL) {
		processPlayItem(inCount.indices, inCount.count);
	}
}

void resultFunc(GtkIconView *icon_view, GtkTreePath *path, gpointer data) {
	gint count;
	gint *indices = gtk_tree_path_get_indices_with_depth(path, &count);
	if(indices != NULL) {
		processResult(indices, count);
	}
}

void resultActivated(GtkWidget *widget, gpointer statusbar) {
	gtk_icon_view_selected_foreach(GTK_ICON_VIEW(widget), resultFunc, NULL);
}

GtkWidget *create_view_and_model(void) {
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

GtkTreeModel *getCategoriesModel() {
	GtkTreeStore *treestore;
	GtkTreeIter topLevel, child;
	
	treestore = gtk_tree_store_new(NUM_COLS, GDK_TYPE_PIXBUF,
				  G_TYPE_STRING);
	
	GdkPixbuf *category = create_pixbuf("folder_16.png");
	GdkPixbuf *item = create_pixbuf("link_16.png"); 
	
	for(unsigned i = 0; i < categories.getCategories().size(); i++) {
		gtk_tree_store_append(treestore, &topLevel, NULL);
		gtk_tree_store_set(treestore, &topLevel, IMAGE_COLUMN, category, TITLE_COLUMN,
		    categories.getCategories()[i].get_title().c_str(), -1);
		
		for(unsigned j = 0; j < categories.getCategories()[i].get_subctgs().size(); j++) {
			gtk_tree_store_append(treestore, &child, &topLevel);
			gtk_tree_store_set(treestore, &child, IMAGE_COLUMN, item, TITLE_COLUMN, 
			    categories.getCategories()[i].get_subctgs()[j].get_title().c_str(), -1);
		}
	}
	
	return GTK_TREE_MODEL(treestore);
}

void updateCategories() {
	gtk_widget_set_visible(hbCategoriesError, FALSE);
	gtk_widget_set_visible(spCategories, FALSE);
	gtk_spinner_stop(GTK_SPINNER(spCategories));
	gtk_widget_set_visible(swLeftTop, TRUE);
	
	GtkTreeModel *model;
	model = getCategoriesModel();
    gtk_tree_view_set_model(GTK_TREE_VIEW(tvCategories), model);
	g_object_unref(model);
}

void showSpCategories() {
	gtk_widget_set_visible(vbLeft, TRUE);
	gtk_widget_set_visible(frLeftTop, TRUE);
	gtk_widget_set_visible(swLeftTop, FALSE);
	gtk_widget_set_visible(hbCategoriesError, FALSE);
	gtk_widget_set_visible(spCategories, TRUE);
	gtk_spinner_start(GTK_SPINNER(spCategories));
}

void showCategoriesError() {
	gtk_widget_set_visible(vbLeft, TRUE);
	gtk_widget_set_visible(frLeftTop, TRUE);
	gtk_widget_set_visible(swLeftTop, FALSE);
	gtk_widget_set_visible(hbCategoriesError, TRUE);
	gtk_widget_set_visible(spCategories, FALSE);
	gtk_spinner_stop(GTK_SPINNER(spCategories));
}

gpointer categoriesTask(gpointer arg) {
	// On pre execute
	gdk_threads_enter();
	showSpCategories();
	
	// Stop others threads
	curlActorsStop = TRUE;
	curlCategoriesStop = FALSE;
	curlStop = TRUE;
	curlResultsStop = TRUE;
	
	gdk_threads_leave();
	
	string page = HtmlString::getCategoriesPage();
	gdk_threads_enter();
	categories.parse_categories(page);
	if(!categories.getCategories().empty()) {
		updateCategories();
	}else {
		showCategoriesError();
	}
	gdk_threads_leave();
	return NULL;
}

void processCategories() {
	//Starting new thread to get categories from the net  
    g_thread_new(NULL, categoriesTask, NULL);
}

static void btnCategoriesClicked( GtkWidget *widget,
                      gpointer   data)
{
	if(!gtk_widget_get_visible(vbLeft)) {
		if(categories.getCategories().empty()) {
			processCategories();
		}else {
			gtk_widget_set_visible(vbLeft, TRUE);
			gtk_widget_set_visible(frLeftTop, TRUE);
			gtk_widget_set_visible(frLeftBottom, FALSE);
		}
	}else {
		if(!gtk_widget_get_visible(frLeftBottom)) {
			gtk_widget_set_visible(vbLeft, FALSE);
			gtk_widget_set_visible(frLeftTop, FALSE);
		}else {
		    if(gtk_widget_get_visible(frLeftTop)) {
				gtk_widget_set_visible(frLeftTop, FALSE);
			}else {
				if(categories.getCategories().empty()) {
					processCategories();
				}else {
					gtk_widget_set_visible(frLeftTop, TRUE);
				}
			}	
		}
	}	
}

static void btnUpClicked( GtkWidget *widget,
                      gpointer   data )
{
	if(displayMode == PLAYLISTS) {
		switchToIconView();
		updateResults();
	}
}

void savedRecovery() {
	// Clear results links set if not paging
    // (do not allow next page thread to be called twice)
    resultsThreadsLinks.clear(); 
    
	// Update iconView with history results
	results.copyToModel();
	// Scroll to saved position after updating model
	string index = results.getIndex();
	GtkTreePath *path1 = gtk_tree_path_new_from_string(index.c_str());
	gtk_icon_view_scroll_to_path(GTK_ICON_VIEW(iconView), path1, FALSE, 0, 0);
    gtk_tree_path_free(path1);
}

static void btnPrevClicked( GtkWidget *widget,
                      gpointer   data )
{   
	isPage = false;
	// Save current results to forwardResultsStack
	forwardResultsStack.push_back(results);  
	// Display top results from backResultsStack
	results = backResultsStack.back();
	backResultsStack.pop_back();
	savedRecovery();
	updateResults();		  
}

static void btnNextClicked( GtkWidget *widget,
                      gpointer   data )
{   
	isPage = false;
    // Save current results to backResultsStack
    backResultsStack.push_back(results);
    // Display top result from forwardResultsStack
    results = forwardResultsStack.back();
    forwardResultsStack.pop_back();
    savedRecovery();
    updateResults(); 
}

static void entryActivated( GtkWidget *widget, 
                      gpointer data) {
    string query(gtk_entry_get_text(GTK_ENTRY(widget)));
    title = "Search: " + query;
    string base_url = string(DOMAIN) + "/?do=search&subaction=search&mode=simple&story=" + to_cp1251(query);
    results.setBaseUrl(base_url);
	isPage = false;
    g_thread_pool_push(resultsThreadPool,
        (gpointer) results.getBaseUrl().c_str(),
        NULL);		  						  
}

static void rbActorsClicked(GtkWidget *widget, gpointer data) {
	//Toggle visibility of actors list (vbRight)
	if(!gtk_widget_get_visible(vbRight)) {
		if(!actors.getActors().empty() && 
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

static void backResultsChanged(GtkWidget *widget, gpointer data) {
	GtkTreeIter iter;
	GtkTreeModel *model;
	gchar *value;
	
	if (gtk_tree_selection_get_selected(
	    GTK_TREE_SELECTION(widget), &model, &iter)) {
	    isPage = false;
		gtk_tree_model_get(model, &iter, TITLE_COLUMN, &value,  -1);
		
		// Save or resave displayed results
		if(results.getResults().size() > 0) {
			// Add first time to map, add to listView
			if(backResults.count(results.getTitle()) == 0) {
				backResultsListAdd(results.getTitle());
			}
			saveResultsPostion();
			backResults[results.getTitle()] = results;
		}
	    
	    // Set back results as results
		results = backResults[string(value)];
	    savedRecovery();
		updateResults();
		g_free(value);
    }
}

static void btnCategoriesRepeatClicked(GtkWidget *widget, gpointer data) {
	processCategories();
}

static void btnActorsRepeatClicked(GtkWidget *widget, gpointer data) {
	g_thread_pool_push(actorsThreadPool, 
		(gpointer) lastActorsHref.c_str(),
		NULL);
}

static void btnHistoryClicked(GtkWidget *widget, gpointer data) {
	if(!gtk_widget_get_visible(vbLeft)) { // left vbox is hidden
		gtk_widget_set_visible(vbLeft, TRUE);
		gtk_widget_set_visible(frLeftBottom, TRUE);
		gtk_widget_set_visible(frLeftTop, FALSE); // hide categories
	}else { //left vbox is visible
		if(!gtk_widget_get_visible(frLeftTop)) { // hide left vbox if no categories
			gtk_widget_set_visible(frLeftBottom, FALSE);
			gtk_widget_set_visible(vbLeft, FALSE);
		}else {
			if(gtk_widget_get_visible(frLeftBottom)) { // show or hide history if categories present
				gtk_widget_set_visible(frLeftBottom, FALSE);
			}else {
				gtk_widget_set_visible(frLeftBottom, TRUE);
			}
		}
	}
}

gboolean iconViewExposed(GtkWidget *widget, GdkEvent *event, gpointer data) {
	displayRange();
	return FALSE;
}

void swIconVScrollChanged(GtkAdjustment* adj, gpointer data) {
	gdouble value = gtk_adjustment_get_value(adj);
	gdouble upper = gtk_adjustment_get_upper(adj);
	gdouble page_size = gtk_adjustment_get_page_size(adj);
	gdouble max_value = upper - page_size - page_size;
	if (value > max_value) {
		if(!results.getNextLink().empty()) {
			// Search for the same link only once if it's not saved in set.
			if(resultsThreadsLinks.count(results.getNextLink()) == 0) {
				isPage = true;
				resultsThreadsLinks.insert(results.getNextLink());
				g_thread_pool_push(resultsThreadPool,
	                (gpointer)results.getNextLink().c_str(),
	                NULL);
			}
		}
	}
}

int main( int   argc,
          char *argv[] )
{
    GtkWidget *vbox;
    GtkWidget *toolbar; 
    GtkWidget *hbCenter;    
    GtkWidget *swRightTop, *swRightBottom, *swLeftBottom;;
    GtkWidget *vbLeftTop;
    GtkWidget *btnCategoriesError;
    GtkWidget *btnActorsError;
    
	GtkToolItem *btnCategories;
	GtkToolItem *sep;
	GtkToolItem *exit;
	
	GdkPixbuf *icon;
	
	GtkTreeSelection *selection, *selection2; 
	
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
    
    treeView = create_view_and_model();
    
    iconView = gtk_icon_view_new();
    gtk_icon_view_set_pixbuf_column(GTK_ICON_VIEW(iconView), IMAGE_COLUMN);                                                  
    gtk_icon_view_set_text_column(GTK_ICON_VIEW(iconView), TITLE_COLUMN);
    gtk_icon_view_set_item_width(GTK_ICON_VIEW(iconView), 180);
    
    // set model to iconView
    GtkListStore *iconViewStore;
    GtkTreeModel *model;
	iconViewStore = gtk_list_store_new(NUM_COLS, GDK_TYPE_PIXBUF, G_TYPE_STRING);
	model = GTK_TREE_MODEL(iconViewStore);
	gtk_icon_view_set_model(GTK_ICON_VIEW(iconView), model);
	g_object_unref(model);
    
    toolbar = gtk_toolbar_new();
	gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_ICONS);
	gtk_container_set_border_width(GTK_CONTAINER(toolbar), 2);
	
    btnCategories = gtk_tool_button_new_from_stock(GTK_STOCK_DIRECTORY);
    gtk_tool_item_set_tooltip_text(btnCategories, "Display categories");
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), btnCategories, -1);
    g_signal_connect(GTK_WIDGET(btnCategories), "clicked", G_CALLBACK(btnCategoriesClicked), NULL);
    
    sep = gtk_separator_tool_item_new();
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), sep, -1);
	
	GtkWidget *historyIcon = gtk_image_new_from_file("history_24.png");
	btnHistory = gtk_tool_button_new(historyIcon, "History");
	gtk_tool_item_set_tooltip_text(btnHistory, "Results history");
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), btnHistory, -1);
	g_signal_connect(GTK_WIDGET(btnHistory), "clicked",
	    G_CALLBACK(btnHistoryClicked), NULL);
	    
	sep = gtk_separator_tool_item_new();
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), sep, -1);
    
    btnUp = gtk_tool_button_new_from_stock(GTK_STOCK_GO_UP);
    gtk_tool_item_set_tooltip_text(btnUp, "Move up");
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), btnUp, -1);
    g_signal_connect(GTK_WIDGET(btnUp), "clicked", G_CALLBACK(btnUpClicked), NULL);
    
    btnPrev = gtk_tool_button_new_from_stock(GTK_STOCK_GO_BACK);
    gtk_tool_item_set_tooltip_text(btnPrev, "Go back in history");
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), btnPrev, -1);
    g_signal_connect(btnPrev, "clicked", G_CALLBACK(btnPrevClicked), NULL);
    
    btnNext = gtk_tool_button_new_from_stock(GTK_STOCK_GO_FORWARD);
    gtk_tool_item_set_tooltip_text(btnNext, "Go forward in history");
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), btnNext, -1);
    g_signal_connect(GTK_WIDGET(btnNext), "clicked", G_CALLBACK(btnNextClicked), NULL);
    
    sep = gtk_separator_tool_item_new();
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), sep, -1);
    
    entry = gtk_entry_new();
    gtk_widget_set_tooltip_text(entry, "Search online-life");
	GtkToolItem *entryItem = gtk_tool_item_new();
	gtk_container_add(GTK_CONTAINER(entryItem), entry);
    g_signal_connect(entry, "activate", G_CALLBACK(entryActivated), NULL);
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
    
    gtk_container_add(GTK_CONTAINER(swTree), treeView);
    gtk_container_add(GTK_CONTAINER(swIcon), iconView);
    
    tvBackResults = create_view_and_model();
    // Set up store2
    GtkListStore *store2 
        = gtk_list_store_new(NUM_COLS, GDK_TYPE_PIXBUF, G_TYPE_STRING);    
    gtk_tree_view_set_model(GTK_TREE_VIEW(tvBackResults), 
        GTK_TREE_MODEL(store2));
	g_object_unref(store2);
	
	selection2 = gtk_tree_view_get_selection(GTK_TREE_VIEW(tvBackResults));

	g_signal_connect(selection2, "changed", 
	  G_CALLBACK(backResultsChanged), NULL);
    
    tvBackActors = create_view_and_model();
    // Set up store
    GtkListStore *store 
        = gtk_list_store_new(NUM_COLS, GDK_TYPE_PIXBUF, G_TYPE_STRING);    
    gtk_tree_view_set_model(GTK_TREE_VIEW(tvBackActors), 
        GTK_TREE_MODEL(store));
	g_object_unref(store);
	
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tvBackActors));

	g_signal_connect(selection, "changed", 
	  G_CALLBACK(backActorsChanged), NULL);
    
    tvCategories = create_view_and_model();
    tvActors = create_view_and_model();
    
    swLeftTop = gtk_scrolled_window_new(NULL, NULL);
    swLeftBottom = gtk_scrolled_window_new(NULL, NULL);
    swRightTop = gtk_scrolled_window_new(NULL, NULL);
    swRightBottom = gtk_scrolled_window_new(NULL, NULL);
    
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(swLeftTop),
            GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(swLeftTop),
            GTK_SHADOW_ETCHED_IN);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(swLeftBottom),
            GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(swLeftBottom),
            GTK_SHADOW_ETCHED_IN);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(swRightTop),
            GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(swRightTop),
            GTK_SHADOW_ETCHED_IN);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(swRightBottom),
            GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(swRightBottom),
            GTK_SHADOW_ETCHED_IN);
    
    vbLeft = gtk_vbox_new(TRUE, 1);
    vbRight = gtk_vbox_new(FALSE, 1);
    hbCenter = gtk_hbox_new(FALSE, 1);
    
    const int SIDE_SIZE = 220;
    gtk_widget_set_size_request(vbLeft, SIDE_SIZE, -1);
    gtk_widget_set_size_request(vbRight, SIDE_SIZE, -1);
    
    // Scroll containers
    gtk_container_add(GTK_CONTAINER(swLeftTop), tvCategories);
    gtk_container_add(GTK_CONTAINER(swLeftBottom), tvBackResults);
    gtk_container_add(GTK_CONTAINER(swRightTop), tvActors);
    gtk_container_add(GTK_CONTAINER(swRightBottom), tvBackActors);
    
    // Frames
    frLeftTop = gtk_frame_new("Categories");
    frLeftBottom = gtk_frame_new("Results history");
    frRightTop = gtk_frame_new("Actors");
    frRightBottom = gtk_frame_new("Actors history");
    
    // Add categories spinner and error
    vbLeftTop = gtk_vbox_new(FALSE, 1);
    hbCategoriesError = gtk_hbox_new(FALSE, 1); // for Repeat button normal size
    spCategories = gtk_spinner_new();
    btnCategoriesError = gtk_button_new_with_label("Repeat");
    g_signal_connect(btnCategoriesError, "clicked", 
        G_CALLBACK(btnCategoriesRepeatClicked), NULL);
    gtk_box_pack_start(GTK_BOX(hbCategoriesError), btnCategoriesError, TRUE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(vbLeftTop), swLeftTop, TRUE, TRUE, 1);
    gtk_box_pack_start(GTK_BOX(vbLeftTop), spCategories, TRUE, FALSE, 1);
    gtk_box_pack_start(GTK_BOX(vbLeftTop), hbCategoriesError, TRUE, FALSE, 1);
    
    gtk_container_add(GTK_CONTAINER(frLeftTop), vbLeftTop);
    gtk_container_add(GTK_CONTAINER(frLeftBottom), swLeftBottom);
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
    
    //vbLeft
    gtk_box_pack_start(GTK_BOX(vbLeft), frLeftTop, TRUE, TRUE, 1);
    gtk_box_pack_start(GTK_BOX(vbLeft), frLeftBottom, TRUE, TRUE, 1);
    
    //vbRight
    gtk_box_pack_start(GTK_BOX(vbRight), frRightBottom, TRUE, TRUE, 1);
    
    // Add center spinner
    spCenter = gtk_spinner_new();
    
    // add vbox center
    vbCenter = gtk_vbox_new(FALSE, 1);
    // add items to vbCenter
    gtk_box_pack_start(GTK_BOX(vbCenter), swTree, TRUE, TRUE, 1);
    gtk_box_pack_start(GTK_BOX(vbCenter), swIcon, TRUE, TRUE, 1);
    gtk_box_pack_start(GTK_BOX(vbCenter), spCenter, TRUE, FALSE, 1);
    
    //hbCenter
    gtk_box_pack_start(GTK_BOX(hbCenter), vbLeft, FALSE, FALSE, 1);
    gtk_box_pack_start(GTK_BOX(hbCenter), vbCenter, TRUE, TRUE, 1);
    gtk_box_pack_start(GTK_BOX(hbCenter), vbRight, FALSE, FALSE, 1);
    
    gtk_box_pack_start(GTK_BOX(vbox), hbCenter, TRUE, TRUE, 1);
    
    g_signal_connect(treeView, "row-activated", 
        G_CALLBACK(on_changed), NULL);
        
    g_signal_connect(tvCategories, "row-activated",
        G_CALLBACK(categoriesClicked), NULL);
        
    g_signal_connect(tvActors, "row-activated",
        G_CALLBACK(actorsClicked), NULL);
        
    g_signal_connect(iconView, "item-activated", 
        G_CALLBACK(resultActivated), NULL);  
    
    g_signal_connect(iconView, "expose-event",
        G_CALLBACK(iconViewExposed), NULL);
    
    gtk_container_add(GTK_CONTAINER(window), vbox);
    
    gtk_window_set_default_size(GTK_WINDOW(window), 700, 400);
    
    gtk_widget_show_all(window);
    
    g_signal_connect(GTK_WIDGET(rbActors), "clicked", 
        G_CALLBACK(rbActorsClicked), NULL);
    
    gtk_widget_set_visible(vbLeft, FALSE);
    gtk_widget_set_visible(vbRight, FALSE);
    
    gtk_widget_set_visible(swTree, FALSE);
    gtk_widget_set_visible(frLeftTop, FALSE);
    gtk_widget_set_visible(frLeftBottom, FALSE);
    gtk_widget_set_visible(frRightBottom, FALSE);
    
    gtk_widget_set_visible(spCenter, FALSE);
    gtk_widget_set_size_request(spCenter, 32, 32);
    
    gtk_widget_set_visible(spCategories, FALSE);
    gtk_widget_set_visible(hbCategoriesError, FALSE);
    gtk_widget_set_size_request(spCategories, 32, 32);
    
    gtk_widget_set_visible(spActors, FALSE);
    gtk_widget_set_visible(hbActorsError, FALSE);
    
    // GThreadPool for downloading images
    imagesThreadPool = g_thread_pool_new(imageDownloadTask,
                                   NULL, 
                                   -1,
                                   FALSE,
                                   NULL);
                                   
    // GThreadPool for results
    resultsThreadPool = g_thread_pool_new(resultsTask,
                                   NULL,
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
                                   NULL,
                                   1, // Run one thread at the time
                                   FALSE,
                                   NULL);
                                   
    // IconView scroll to the bottom detection code
    GtkAdjustment *vadjustment;
    vadjustment = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(swIcon));
    g_signal_connect(vadjustment, "value-changed",
        G_CALLBACK(swIconVScrollChanged), NULL);
        
    // Initialize default pixbuf for iconView here
    defaultPixbuf = create_pixbuf("blank.png");
    
    // Initialize breaking thread variable
    curlStop = FALSE;
    curlCategoriesStop = FALSE;
    curlResultsStop = FALSE;
    curlActorsStop = FALSE;
    
    gtk_main();
    gdk_threads_leave ();
 
    /* we're done with libcurl, so clean it up */ 
	curl_global_cleanup();

    return 0;
}
