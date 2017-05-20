#include <gtk/gtk.h>
#include <vector>
#include <string>
#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <string.h>
#include <map>
#include <set>

#include "Converter.hpp"
#include "DisplayMode.hpp"
#include "HtmlString.hpp"

#define DOMAIN "http://online-life.in"

#include "Categories.hpp"
#include "Results.hpp"
#include "Playlists.hpp"
#include "Actors.hpp"

//#define OLD

using namespace std;

enum {
	IMAGE_COLUMN = 0,
	TITLE_COLUMN,
	NUM_COLS
};

GtkWidget *pbStatus;
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
GtkWidget *swRightBottom, *swLeftTop, *swLeftBottom;

DisplayMode displayMode;

string title;
const string PROG_NAME("Online life");

GtkWidget *treeView;
GtkWidget *iconView;

GtkToolItem *btnUp;
GtkToolItem *btnPrev;
GtkToolItem *btnNext;
GtkToolItem *btnHistory;

GtkToolItem *rbActors;
GtkToolItem *rbPlay;
GtkToolItem *rbDownload;

GtkWidget *window;
GtkWidget *lbPage;
GtkWidget *entry;

GtkWidget *swTree, *swIcon;
GtkWidget *tvBackResults, *tvCategories, *tvActors, *tvBackActors;
GtkWidget *vbLeft, *vbRight;

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
}

void switchToIconView() {
	gtk_widget_set_visible(swTree, FALSE);
	gtk_widget_set_visible(swIcon, TRUE);
}

GdkPixbuf *create_pixbuf(const gchar * filename) {
    
    GdkPixbuf *pixbuf;
    GError *error = NULL;
    pixbuf = gdk_pixbuf_new_from_file(filename, &error);
    
    if (!pixbuf) {
        
        fprintf(stderr, "%s\n", error->message);
        g_error_free(error);
        
    }
    
    return pixbuf;
}

void clearActorsResults() {
	//resultsBack.clear();
	//actorsBack.clear();
}

void disableAllItems() {
	gtk_label_set_text(GTK_LABEL(lbPage), "");
	gtk_widget_set_sensitive(GTK_WIDGET(btnUp), FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(btnPrev), FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(btnNext), FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(btnHistory), FALSE);
	
	gtk_widget_set_sensitive(GTK_WIDGET(rbActors), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(rbPlay), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(rbDownload), FALSE);
}

//bool is_switched_from_actors = FALSE;

void setSensitiveItemsPlaylists() {
	gtk_widget_set_sensitive(GTK_WIDGET(btnUp), TRUE);
	gtk_widget_set_sensitive(GTK_WIDGET(btnPrev), FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(btnNext), FALSE);
	
	gtk_widget_set_sensitive(GTK_WIDGET(rbPlay), TRUE);
    gtk_widget_set_sensitive(GTK_WIDGET(rbDownload), TRUE);
    gtk_widget_set_sensitive(GTK_WIDGET(rbActors), FALSE);
    
    //Actors is selected but will be disabled, so switch to rbPlay
    /*if(gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(rbActors))){
		gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(rbPlay), TRUE);
		is_switched_from_actors = TRUE; // flag to switch back to actors on the way back
	}*/
}

void setSensitiveItemsResults() {
	if(backResults.size() > 0) {
		gtk_widget_set_sensitive(GTK_WIDGET(btnHistory), TRUE);
	}
	
	gtk_widget_set_sensitive(GTK_WIDGET(btnUp), FALSE);

	if(results.getPrevLink().empty()) {
		gtk_widget_set_sensitive(GTK_WIDGET(btnPrev), FALSE);
	}else {
		gtk_widget_set_sensitive(GTK_WIDGET(btnPrev), TRUE);
	}
	
	if(results.getNextLink().empty()) {
		gtk_widget_set_sensitive(GTK_WIDGET(btnNext), FALSE);
	}else {
		gtk_widget_set_sensitive(GTK_WIDGET(btnNext), TRUE);
	}
	gtk_widget_set_sensitive(GTK_WIDGET(rbPlay), TRUE);
    gtk_widget_set_sensitive(GTK_WIDGET(rbDownload), TRUE);
    gtk_widget_set_sensitive(GTK_WIDGET(rbActors), TRUE);
    
    // Switching back to actors if flag is true if actors were selected in results list.
    /*if(is_switched_from_actors) {
		gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(rbActors), TRUE);
		is_switched_from_actors = FALSE;
	}*/
}

void setSensitiveItemsActors() {
	gtk_widget_set_sensitive(GTK_WIDGET(btnUp), TRUE);
	gtk_widget_set_sensitive(GTK_WIDGET(btnPrev), FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(btnNext), FALSE);
	
	gtk_widget_set_sensitive(GTK_WIDGET(rbActors), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(rbPlay), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(rbDownload), FALSE);
}

map<string, GdkPixbuf*> imagesCache;
map<string, GtkTreeIter> iters;
set<int> imageIndexes;

/*struct MemoryStruct {
  char *memory;
  size_t size;
};*/

struct ArgsStruct {
	GdkPixbufLoader* loader;
	GtkTreeIter iter;
};

static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	size_t realsize = size * nmemb;
	//struct MemoryStruct *mem = (struct MemoryStruct *)userp;
	//GdkPixbufLoader* loader = GDK_PIXBUF_LOADER(userp);
	struct ArgsStruct *args = (struct ArgsStruct *)userp;
	
	gdk_pixbuf_loader_write(args->loader, (const guchar*)contents, realsize, NULL);
	GdkPixbuf* pixbufOrig = gdk_pixbuf_loader_get_pixbuf(args->loader);
	if(pixbufOrig != NULL) {
		gdk_threads_enter();
		GtkListStore *store = GTK_LIST_STORE(gtk_icon_view_get_model
			(GTK_ICON_VIEW(iconView)));
		gtk_list_store_set(store, &args->iter, IMAGE_COLUMN, pixbufOrig, -1);  
		gdk_threads_leave();
	}
	
	/*mem->memory = (char*)realloc(mem->memory, mem->size + realsize + 1);
	if(mem->memory == NULL) {
		// out of memory! 
		cout << "not enough memory (realloc returned NULL)" << endl;
		return 0;
	}
	
	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;*/
	
	return realsize;
}

void getPixbufFromUrl(string url) {

	CURL *curl_handle;
	CURLcode res;
	GdkPixbuf *pixbuf = NULL;
	
	//struct MemoryStruct chunk;
	GdkPixbufLoader* loader = gdk_pixbuf_loader_new();
	GtkTreeIter iter = iters[url];
	struct ArgsStruct args;
	args.loader = loader;
	args.iter = iter;
	
	//chunk.memory = (char*) malloc(1);  /* will be grown as needed by the realloc above */ 
	//chunk.size = 0;    /* no data at this point */ 
	
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
		/*
		 * Now, our chunk.memory points to a memory block that is chunk.size
		 * bytes big and contains the remote file.
		 *
		 * Do something nice with it!
		 */ 
		 
		 /*GInputStream *gis = g_memory_input_stream_new_from_data(
		     chunk.memory, chunk.size , NULL);
		     
		 GError *err = NULL;
		 pixbuf = gdk_pixbuf_new_from_stream(gis, NULL, &err);
		 if(!pixbuf) {
			 fprintf(stderr, "%s\n", err->message);
             g_error_free(err);
		 }*/
		 
		 //GdkPixbufLoader* loader = gdk_pixbuf_loader_new();
		 //gdk_pixbuf_loader_write(loader, (const guchar*)chunk.memory, chunk.size, NULL);
		 GError *error = NULL;
		 gboolean ok = gdk_pixbuf_loader_close(loader, &error);
		 if(!ok) {
			 fprintf(stderr, "On close: %s\n", error->message);
             g_error_free(error);
		 }
		 
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
	
	/* cleanup curl stuff */ 
	curl_easy_cleanup(curl_handle);
	
	//free(chunk.memory);
	g_object_unref(loader);
}

gpointer imageDownloadTask(gpointer arg) {
	string link((char*)arg);
	
	gdk_threads_enter();
	int count = imagesCache.count(link);
	gdk_threads_leave();
	
	if(count == 0) { //if map does not have pixbuf
		getPixbufFromUrl(link);
	}
	
	return NULL;
}

GtkTreeModel *getResultsModel() {
	GtkListStore *store;
    GtkTreeIter iter;
    iters.clear();
    
    store = gtk_list_store_new(NUM_COLS, GDK_TYPE_PIXBUF, G_TYPE_STRING);
    
    GdkPixbuf *item = NULL;
    GdkPixbuf *defaultItem = create_pixbuf("blank.png");
    
    for(unsigned i=0; i < results.getResults().size(); i++) {
		string link = results.getResults()[i].get_image_link();
		
		gtk_list_store_append(store, &iter);
		
		if(imagesCache.count(link) > 0) {
			item = imagesCache[link];
		}else {
			item = defaultItem;
			iters[link] = iter;
		}
		
        gtk_list_store_set(store, &iter, IMAGE_COLUMN, item,
            TITLE_COLUMN, results.getResults()[i].get_title().c_str(), -1);
	}
	g_object_unref(defaultItem);
    return GTK_TREE_MODEL(store);
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
				#ifdef OLD
					g_thread_create(imageDownloadTask,
					 (gpointer) results.getResults()[i].get_image_link().c_str(),
					  FALSE, NULL);
				#else
					g_thread_new(NULL, imageDownloadTask, 
					    (gpointer) results.getResults()[i].get_image_link().c_str());
				#endif
			}
		}
		gtk_tree_path_free(path1);
		gtk_tree_path_free(path2);
	}
}

void updateResults() {
	imageIndexes.clear();
	if(displayMode != RESULTS) {
		switchToIconView();
		displayMode = RESULTS;
	}
	string title = PROG_NAME + " - " + results.getTitle();
	gtk_window_set_title(GTK_WINDOW(window), title.c_str());
	
	GtkTreeModel *model;
	model = getResultsModel();
	gtk_icon_view_set_model(GTK_ICON_VIEW(iconView), model);
    //gtk_tree_view_set_model(GTK_TREE_VIEW(treeView), model);
	g_object_unref(model);
	
	// Scroll to the top of the list
	GtkTreePath *path = gtk_tree_path_new_first();
	gtk_icon_view_scroll_to_path(GTK_ICON_VIEW(iconView), path, FALSE, 0, 0);
	
	gtk_label_set_text(GTK_LABEL(lbPage), results.getCurrentPage().c_str());
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

gpointer getResultsTask(gpointer arg) {
	string link((char *)arg);
	//cout << "Link: " << link << endl;
	gdk_threads_enter();
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(pbStatus), "Searching results...");
    gdk_threads_leave();
    
	HtmlString html_string(pbStatus);
	html_string.setMode(RESULTS);
	string page = html_string.get_string(link);
	
    gdk_threads_enter();
	results.getResultsPage(page);
	
	if(!results.getResults().empty()) {
		// add back results
		// save prev results to map
		// add title to tvBackResults
		if(backResults.count(prevResults.getTitle()) == 0 
		        && prevResults.getResults().size() > 0) {
			backResults[prevResults.getTitle()] = prevResults;
			backResultsListAdd(prevResults.getTitle());
		}
		gtk_progress_bar_set_text(GTK_PROGRESS_BAR(pbStatus), "Done");
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(pbStatus), 0);
		updateResults();
	}else {
		gtk_progress_bar_set_text(GTK_PROGRESS_BAR(pbStatus), "Nothing found");
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(pbStatus), 0);
	}
	gdk_threads_leave();
	return NULL;
}

void processCategory(gint *indices, gint count) {//move to results
	if(count == 1) { //Node
		gint i = indices[0];
		prevResults = results;
		title = categories.getCategories()[i].get_title();
		results.setTitle(title);
		#ifdef OLD
			g_thread_create(getResultsTask,
				 (gpointer) categories.getCategories()[i].get_link().c_str(),
				  FALSE, NULL);
	    #else
		    g_thread_new(NULL, getResultsTask,
			    (gpointer) categories.getCategories()[i].get_link().c_str());
	    #endif
		
	}else if(count == 2) { //Leaf
		gint i = indices[0];
		gint j = indices[1];
		prevResults = results;
		title = categories.getCategories()[i].get_subctgs()[j].get_title();
		results.setTitle(title);
		#ifdef OLD
			g_thread_create(getResultsTask,
				 (gpointer) categories.getCategories()[i].get_subctgs()[j].get_link().c_str(),
				  FALSE, NULL);
	    #else
		    g_thread_new(NULL, getResultsTask, 
			    (gpointer) categories.getCategories()[i].get_subctgs()[j].get_link().c_str());
		#endif
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
	
	gtk_label_set_text(GTK_LABEL(lbPage), "");
	
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

gpointer getPlaylistsTask(gpointer args) {
	string id((char*)args);
	string url = "http://dterod.com/js.php?id=" + id;
	string referer = "http://dterod.com/player.php?newsid=" + id;
	
	gdk_threads_enter();
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(pbStatus), "Searching playlists...");
	gdk_threads_leave();
	
	HtmlString html_string(pbStatus);
	string js = html_string.get_string(url, referer);
	string playlist_link = get_txt_link(js);
	if(!playlist_link.empty()) {
		string json = html_string.get_string(playlist_link);
		gdk_threads_enter();
		playlists.parse(json);
		if(!playlists.getPlaylists().empty()) {
			gtk_progress_bar_set_text(GTK_PROGRESS_BAR(pbStatus), "Done");
			gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(pbStatus), 0);
			displayPlaylists();
		}else {
			gtk_progress_bar_set_text(GTK_PROGRESS_BAR(pbStatus), "Nothing found");
			gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(pbStatus), 0);
		}
		gdk_threads_leave();
	}else {
		gdk_threads_enter();
		PlayItem playItem = playlists.parse_play_item(js);
		if(!playItem.get_comment().empty()) {
			gtk_progress_bar_set_text(GTK_PROGRESS_BAR(pbStatus), "Done");
			gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(pbStatus), 0);
			processPlayItem(playItem); 
		}else {
			gtk_progress_bar_set_text(GTK_PROGRESS_BAR(pbStatus), "Nothing found");
			gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(pbStatus), 0);
		}
		gdk_threads_leave();
	}
	return NULL;
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
	//switchToTreeView();
	//displayMode = ACTORS;
	//gtk_window_set_title(GTK_WINDOW(window), actors.getTitle().c_str());
	
	if(!gtk_widget_get_visible(vbRight)) {
		gtk_widget_set_visible(vbRight, TRUE);
	}
	
	GtkTreeModel *model;
	model = getActorsModel();
    gtk_tree_view_set_model(GTK_TREE_VIEW(tvActors), model);
	g_object_unref(model);
	
	//gtk_label_set_text(GTK_LABEL(lbPage), "");
	
	//setSensitiveItemsActors();
}

void backActorsListAdd(string title) {
	gtk_widget_set_visible(swRightBottom, TRUE);
	
	GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(
	    GTK_TREE_VIEW(tvBackActors)));
	GtkTreeIter iter;
	GdkPixbuf *icon = create_pixbuf("link_16.png");
	
	gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, IMAGE_COLUMN, icon,
           TITLE_COLUMN, title.c_str(), -1);
}

gpointer getActorsTask(gpointer args) {
	string link((char*)args);
	gdk_threads_enter();
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(pbStatus), "Searching actors...");
	gdk_threads_leave();
	HtmlString html_string(pbStatus);
	html_string.setMode(ACTORS);
	string page = html_string.get_string(link);
	gdk_threads_enter();
	actors.parse(page);
	if(!actors.getActors().empty()) {
	    //Save results 
		//resultsBack.push_back(results);
		//Save actors
		if(backActors.count(prevActors.getTitle()) == 0 
		        && prevActors.getActors().size() > 0) {
		    backActors[prevActors.getTitle()] = prevActors;
		    backActorsListAdd(prevActors.getTitle());	
		}
		gtk_progress_bar_set_text(GTK_PROGRESS_BAR(pbStatus), "Done");
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(pbStatus), 0);
		updateActors();	
	}else {
	    gtk_progress_bar_set_text(GTK_PROGRESS_BAR(pbStatus), "Nothing found");	
	    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(pbStatus), 0);
	}
	gdk_threads_leave();
	return NULL;
}

void processResult(gint *indices, gint count) {//move to playlists
	if(count == 1) { //Node
		gint i = indices[0];
		title = PROG_NAME + " - " + results.getResults()[i].get_title();
		if(gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(rbActors))){
			prevActors = actors;
			actors.setTitle(results.getResults()[i].get_title());
	        #ifdef OLD    	
		        g_thread_create(getActorsTask,
				 (gpointer) results.getResults()[i].get_href().c_str(),
				  FALSE, NULL);
			#else
				g_thread_new(NULL, getActorsTask,
		            (gpointer) results.getResults()[i].get_href().c_str());
	        #endif    
		}else {
			playlists.setTitle(title);
	        #ifdef OLD    
		        g_thread_create(getPlaylistsTask,
				 (gpointer) results.getResults()[i].get_id().c_str(),
				  FALSE, NULL);
			#else
				g_thread_new(NULL, getPlaylistsTask,
		            (gpointer) results.getResults()[i].get_id().c_str());
			#endif	
		}
	}
}

void processActor(gint *indices, gint count) {
	if(count == 1) { //Node
		gint i = indices[0];
		prevResults = results;
		title = actors.getActors()[i].get_title();
		results.setTitle(title);
	    #ifdef OLD    
		    g_thread_create(getResultsTask,
				 (gpointer) actors.getActors()[i].get_href().c_str(),
				  FALSE, NULL);
		#else
			g_thread_new(NULL, getResultsTask, 
		        (gpointer) actors.getActors()[i].get_href().c_str());
		#endif	  	
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
    
	/*GtkTreeIter iter;
	GtkTreeModel *model;
	GtkTreePath *path;
	GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
	
	if (gtk_tree_selection_get_selected(
	  GTK_TREE_SELECTION(selection), &model, &iter)) {
		
		path = gtk_tree_model_get_path(model, &iter);
		gint count;
		gint *indices = gtk_tree_path_get_indices_with_depth(path, &count);
		if(indices != NULL) {
			switch(displayMode) {
				case CATEGORIES:
				    //processCategory(indices, count);
				    break;
				case RESULTS:
				    processResult(indices, count);// Never called
				    break;
				case ACTORS:
				    //processActor(indices, count);
				    break;
				case PLAYLISTS:
				    processPlayItem(indices, count);
				    break;
				case NONE:
				    break;
			}
		}
	}*/
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
	/*if(displayMode == RESULTS) {
		//If moving from results
		switchToTreeView();
	}*/
	//displayMode = CATEGORIES;
	//gtk_window_set_title(GTK_WINDOW(window), categories.getTitle().c_str());
	//clearActorsResults();
	
	gtk_widget_set_visible(vbLeft, TRUE);
	gtk_widget_set_visible(swLeftTop, TRUE);
	
	GtkTreeModel *model;
	model = getCategoriesModel();
    gtk_tree_view_set_model(GTK_TREE_VIEW(tvCategories), model);
	g_object_unref(model);
	
	//disableAllItems();
}

gpointer getCategoriesTask(gpointer arg) {
	gdk_threads_enter();
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(pbStatus), "Searching categories...");
	gdk_threads_leave();
	
	HtmlString html_string(pbStatus);
	html_string.setMode(CATEGORIES);
	string page = html_string.get_string(DOMAIN);
	gdk_threads_enter();
	categories.parse_categories(page);
	if(!categories.getCategories().empty()) {
		gtk_progress_bar_set_text(GTK_PROGRESS_BAR(pbStatus), "Done");
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(pbStatus), 0);
		updateCategories();
	}else {
		gtk_progress_bar_set_text(GTK_PROGRESS_BAR(pbStatus), "Nothing found");
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(pbStatus), 0);
	}
	gdk_threads_leave();
	return NULL;
}

void processCategories() {
	//Starting new thread to get categories from the net
	#ifdef OLD
	    g_thread_create(getCategoriesTask,
		    NULL,
		    FALSE, NULL);
	#else
        g_thread_new(NULL, getCategoriesTask, NULL);
    #endif
}

static void btnCategoriesClicked( GtkWidget *widget,
                      gpointer   data)
{
	if(!gtk_widget_get_visible(vbLeft)) {
		if(categories.getCategories().empty()) {
			processCategories();
		}else {
			gtk_widget_set_visible(vbLeft, TRUE);
			gtk_widget_set_visible(swLeftTop, TRUE);
			gtk_widget_set_visible(swLeftBottom, FALSE);
		}
	}else {
		if(!gtk_widget_get_visible(swLeftBottom)) {
			gtk_widget_set_visible(vbLeft, FALSE);
			gtk_widget_set_visible(swLeftTop, FALSE);
		}else {
		    if(gtk_widget_get_visible(swLeftTop)) {
				gtk_widget_set_visible(swLeftTop, FALSE);
			}else {
				if(categories.getCategories().empty()) {
					processCategories();
				}else {
					gtk_widget_set_visible(swLeftTop, TRUE);
				}
			}	
		}
	}	
}

static void btnUpClicked( GtkWidget *widget,
                      gpointer   data )
{
    switch(displayMode) {
		case CATEGORIES:
		    //Nothing to do
		    break;
		case RESULTS:
	        break;
	    case ACTORS:
	        break;
	    case PLAYLISTS:
			updateResults();
	        break;
	    case NONE:
	        break;
	}
}

static void btnPrevClicked( GtkWidget *widget,
                      gpointer   data )
{   
	#ifdef OLD    
		g_thread_create(getResultsTask,
				 (gpointer)results.getPrevLink().c_str(),
				  FALSE, NULL);
	#else
	    g_thread_new(NULL, getResultsTask, 
	    (gpointer)results.getPrevLink().c_str());
	#endif		  
}

static void btnNextClicked( GtkWidget *widget,
                      gpointer   data )
{   
	#ifdef OLD    
		g_thread_create(getResultsTask,
		    (gpointer)results.getNextLink().c_str(),
		     FALSE, NULL);
	#else
	    g_thread_new(NULL, getResultsTask, 
	        (gpointer)results.getNextLink().c_str());
	#endif		  
}

static void entryActivated( GtkWidget *widget, 
                      gpointer data) {
    string query(gtk_entry_get_text(GTK_ENTRY(widget)));
    prevResults = results;
    title = "Search: " + query;
    results.setTitle(title);
    string base_url = string(DOMAIN) + "/?do=search&subaction=search&mode=simple&story=" + to_cp1251(query);
    results.setBaseUrl(base_url);
	#ifdef OLD
		g_thread_create(getResultsTask,
				 (gpointer)results.getBaseUrl().c_str(),
				  FALSE, NULL);
	#else
	    g_thread_new(NULL, getResultsTask, (gpointer) results.getBaseUrl().c_str());
	#endif		  						  
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
	
		gtk_tree_model_get(model, &iter, TITLE_COLUMN, &value,  -1);
		
		// Save displayed results if not saved
        if(backResults.count(results.getTitle()) == 0) {
			backResults[results.getTitle()] = results;
			backResultsListAdd(results.getTitle());
		}
		// Set and display back results
		results = backResults[string(value)];
		updateResults();
		
		g_free(value);
    }
}

static void btnHistoryClicked(GtkWidget *widget, gpointer data) {
	if(!gtk_widget_get_visible(vbLeft)) { // left vbox is hidden
		gtk_widget_set_visible(vbLeft, TRUE);
		gtk_widget_set_visible(swLeftBottom, TRUE);
		gtk_widget_set_visible(swLeftTop, FALSE); // hide categories
	}else { //left vbox is visible
		if(!gtk_widget_get_visible(swLeftTop)) { // hide left vbox if no categories
			gtk_widget_set_visible(swLeftBottom, FALSE);
			gtk_widget_set_visible(vbLeft, FALSE);
		}else {
			if(gtk_widget_get_visible(swLeftBottom)) { // show or hide history if categories present
				gtk_widget_set_visible(swLeftBottom, FALSE);
			}else {
				gtk_widget_set_visible(swLeftBottom, TRUE);
			}
		}
	}
}

gboolean iconViewExposed(GtkWidget *widget, GdkEvent *event, gpointer data) {
	displayRange();
	return FALSE;
}

int main( int   argc,
          char *argv[] )
{
    
    GtkWidget *vbox;
    GtkWidget *toolbar; 
    GtkWidget *hbCenter;    
    GtkWidget *swRightTop;
    
	GtkToolItem *btnCategories;
	GtkToolItem *sep;
	GtkToolItem *exit;
	
	GdkPixbuf *icon;
	
	GtkTreeSelection *selection, *selection2; 
	
	 /* Must initialize libcurl before any threads are started */ 
    curl_global_init(CURL_GLOBAL_ALL);
    
    #ifdef OLD
    g_thread_init(NULL); // needed only for older gtk
    #endif
    
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
    gtk_tool_item_set_tooltip_text(btnPrev, "Go to previous page");
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), btnPrev, -1);
    g_signal_connect(btnPrev, "clicked", G_CALLBACK(btnPrevClicked), NULL);
    
    lbPage = gtk_label_new("");
    GtkToolItem *labelItem = gtk_tool_item_new();
    gtk_container_add(GTK_CONTAINER(labelItem), lbPage);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), labelItem, -1);
    
    btnNext = gtk_tool_button_new_from_stock(GTK_STOCK_GO_FORWARD);
    gtk_tool_item_set_tooltip_text(btnNext, "Go to next page");
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
    
    //vbLeft
    gtk_box_pack_start(GTK_BOX(vbLeft), swLeftTop, TRUE, TRUE, 1);
    gtk_box_pack_start(GTK_BOX(vbLeft), swLeftBottom, TRUE, TRUE, 1);
    
    //vbRight
    gtk_box_pack_start(GTK_BOX(vbRight), swRightTop, TRUE, TRUE, 1);
    gtk_box_pack_start(GTK_BOX(vbRight), swRightBottom, TRUE, TRUE, 1);
    
    //hbCenter
    gtk_box_pack_start(GTK_BOX(hbCenter), vbLeft, FALSE, FALSE, 1);
    gtk_box_pack_start(GTK_BOX(hbCenter), swTree, TRUE, TRUE, 1);
    gtk_box_pack_start(GTK_BOX(hbCenter), swIcon, TRUE, TRUE, 1);
    gtk_box_pack_start(GTK_BOX(hbCenter), vbRight, FALSE, FALSE, 1);
    
    gtk_box_pack_start(GTK_BOX(vbox), hbCenter, TRUE, TRUE, 1);
    
    pbStatus = gtk_progress_bar_new();
    gtk_box_pack_start(GTK_BOX(vbox), pbStatus, FALSE, FALSE, 1);
    
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
    gtk_widget_set_visible(swLeftTop, FALSE);
    gtk_widget_set_visible(swLeftBottom, FALSE);
    gtk_widget_set_visible(swRightBottom, FALSE);
    
    gtk_main();
    gdk_threads_leave ();
 
    /* we're done with libcurl, so clean it up */ 
	curl_global_cleanup();

    return 0;
}
