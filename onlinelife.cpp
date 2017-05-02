#include <gtk/gtk.h>
#include <vector>
#include <string>
#include <iostream>
#include <stdlib.h>
#include <fstream>

#include "HtmlString.hpp"
#include "Converter.hpp"

#include "Categories.hpp"
#include "Results.hpp"
#include "Playlists.hpp"
#include "Actors.hpp"

#define DOMAIN "http://online-life.cc"
#define PROG_NAME "Online life"

using namespace std;

enum {
	COLUMN = 0,
	NUM_COLS
};

enum DisplayMode {
	CATEGORIES,
	RESULTS,
	ACTORS,
	PLAYLISTS
};

GtkWidget *pbStatus;
//Categories
Categories categories;
//Results
Results results(pbStatus);
//Playlists
Playlists playlists;
//Actors
Actors actors;

vector<Results> resultsBack;
vector<Actors> actorsBack;

DisplayMode displayMode;

string title(PROG_NAME);

GtkWidget *treeView;

GtkToolItem *btnUp;
GtkToolItem *btnPrev;
GtkToolItem *btnNext;

GtkToolItem *rbActors;
GtkToolItem *rbPlay;
GtkToolItem *rbDownload;

GtkWidget *lbPage;
GtkWidget *entry;

string readFromFile(string filename) {
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
}

void clearActorsResults() {
	resultsBack.clear();
	actorsBack.clear();
}

void disableAllItems() {
	gtk_widget_set_sensitive(GTK_WIDGET(btnUp), FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(btnPrev), FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(btnNext), FALSE);
	
	gtk_widget_set_sensitive(GTK_WIDGET(rbActors), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(rbPlay), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(rbDownload), FALSE);
}

bool is_switched_from_actors = FALSE;

void setSensitiveItemsPlaylists() {
	gtk_widget_set_sensitive(GTK_WIDGET(btnUp), TRUE);
	gtk_widget_set_sensitive(GTK_WIDGET(btnPrev), FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(btnNext), FALSE);
	
	gtk_widget_set_sensitive(GTK_WIDGET(rbPlay), TRUE);
    gtk_widget_set_sensitive(GTK_WIDGET(rbDownload), TRUE);
    gtk_widget_set_sensitive(GTK_WIDGET(rbActors), FALSE);
    
    //Actors is selected but will be disabled, so switch to rbPlay
    if(gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(rbActors))){
		gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(rbPlay), TRUE);
		is_switched_from_actors = TRUE; // flag to switch back to actors on the way back
	}
}

void setSensitiveItemsResults() {
	if(!categories.getCategories().empty() || !actorsBack.empty()) {
		gtk_widget_set_sensitive(GTK_WIDGET(btnUp), TRUE);
	}else {
		gtk_widget_set_sensitive(GTK_WIDGET(btnUp), FALSE);
	}
	
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
    
    // Switching back to actors if flag is true
    if(is_switched_from_actors) {
		gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(rbActors), TRUE);
		is_switched_from_actors = FALSE;
	}
}

void setSensitiveItemsActors() {
	gtk_widget_set_sensitive(GTK_WIDGET(btnUp), TRUE);
	gtk_widget_set_sensitive(GTK_WIDGET(btnPrev), FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(btnNext), FALSE);
	
	gtk_widget_set_sensitive(GTK_WIDGET(rbActors), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(rbPlay), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(rbDownload), FALSE);
}

GtkTreeModel *getResultsModel() {
	GtkListStore *store;
    GtkTreeIter iter;
    
    store = gtk_list_store_new(NUM_COLS, G_TYPE_STRING);
    for(unsigned i=0; i < results.getResults().size(); i++) {
		gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, COLUMN, results.getResults()[i].get_title().c_str(), -1);
	}
    
    return GTK_TREE_MODEL(store);
}

void updateResults() {
	displayMode = RESULTS;
	
	GtkTreeModel *model;
	model = getResultsModel();
    gtk_tree_view_set_model(GTK_TREE_VIEW(treeView), model);
	g_object_unref(model);
	
	gtk_label_set_text(GTK_LABEL(lbPage), results.getCurrentPage().c_str());
	gtk_entry_set_text(GTK_ENTRY(entry), "");
	
	setSensitiveItemsResults();
}

void *getResultsTask(void *arg) {
	string link((char *)arg);
	/*string page = readFromFile("Results.html");
	if(page.empty()) {
		cout << "Getting info from net..." << endl;
	    HtmlString html_string;
	    page = html_string.get_string(link);
	    writeToFile("Results.html", page);	
	}else {
		cout << "Getting info from file..." << endl;
		//cout << page << endl;
	}*/
	gdk_threads_enter();
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(pbStatus), "Searching results...");
	gdk_threads_leave();
	HtmlString html_string(pbStatus);
	string page = html_string.get_string(link);
    gdk_threads_enter();
	results.getResultsPage(page);
	if(!results.getResults().empty()) {
		updateResults();
		gtk_progress_bar_set_text(GTK_PROGRESS_BAR(pbStatus), "Done");
	}else {
		gtk_progress_bar_set_text(GTK_PROGRESS_BAR(pbStatus), "Nothing found");
	}
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(pbStatus), 0);
	gdk_threads_leave();
	
	return NULL;
}

void processCategory(gint *indices, gint count) {//move to results
	string link;
	if(count == 1) { //Node
		gint i = indices[0];
		link = categories.getCategories()[i].get_link();
	}else if(count == 2) { //Leaf
		gint i = indices[0];
		gint j = indices[1];
		link = categories.getCategories()[i].get_subctgs()[j].get_link();
	}
	g_thread_create(getResultsTask, (void*) link.c_str(), FALSE, NULL);
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
    
    store = gtk_list_store_new(NUM_COLS, G_TYPE_STRING);
    for(unsigned i=0; i<playlists.getPlaylists()[0].get_items().size(); i++) {
		gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, COLUMN,
             playlists.getPlaylists()[0].get_items()[i].get_comment().c_str(), -1);
	}
    
    return GTK_TREE_MODEL(store);
}

GtkTreeModel *getPlaylistsModel() {
	GtkTreeStore *treestore;
	GtkTreeIter topLevel, child;
	
	treestore = gtk_tree_store_new(NUM_COLS,
				  G_TYPE_STRING);
	
	for(unsigned i=0; i<playlists.getPlaylists().size(); i++) {
		gtk_tree_store_append(treestore, &topLevel, NULL);
		gtk_tree_store_set(treestore, &topLevel, COLUMN,
		    playlists.getPlaylists()[i].get_title().c_str(), -1);
		for(unsigned j=0; j<playlists.getPlaylists()[i].get_items().size(); j++) {
			gtk_tree_store_append(treestore, &child, &topLevel);
			gtk_tree_store_set(treestore, &child, COLUMN, 
			    playlists.getPlaylists()[i].get_items()[j].get_comment().c_str(), -1);
		}
	}
	
	return GTK_TREE_MODEL(treestore);
}

void displayPlaylists() {
	displayMode = PLAYLISTS;
	
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
		    string command = "rxvt -e wget -c " + item.get_download() + "&";
	        system(command.c_str());
		}else if(gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(rbPlay))) {
		    string command = "rxvt -e mplayer -cache 1024 " + item.get_file() + "&";
	        system(command.c_str());
		}	
	}
}

void processPlayItem(gint *indices, gint count) {
	PlayItem playItem;
	if(count == 1) { //Node or List
		gint i = indices[0];
		if(playlists.getPlaylists().size() == 1) { //one playlist only
		    playItem = playlists.getPlaylists()[0].get_items()[i];	
		}
	}else if(count == 2) { //Leaf
		gint i = indices[0];
		gint j = indices[1];
		playItem = playlists.getPlaylists()[i].get_items()[j];
	}
	processPlayItem(playItem);
}

void *getPlaylistsTask(void *args) {
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
			displayPlaylists();
		    gtk_progress_bar_set_text(GTK_PROGRESS_BAR(pbStatus), "Done");
		}else {
			gtk_progress_bar_set_text(GTK_PROGRESS_BAR(pbStatus), "Nothing found");
		}
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(pbStatus), 0);
		gdk_threads_leave();
	}else {
		gdk_threads_enter();
		PlayItem playItem = playlists.parse_play_item(js);
		if(!playItem.get_comment().empty()) {
			processPlayItem(playItem);
		    gtk_progress_bar_set_text(GTK_PROGRESS_BAR(pbStatus), "Done");
		}else {
			gtk_progress_bar_set_text(GTK_PROGRESS_BAR(pbStatus), "Nothing found");
		}
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(pbStatus), 0);
		gdk_threads_leave();
	}
	return NULL;
}

GtkTreeModel *getActorsModel() {
	GtkListStore *store;
    GtkTreeIter iter;
    
    store = gtk_list_store_new(NUM_COLS, G_TYPE_STRING);
    for(unsigned i=0; i < actors.getActors().size(); i++) {
		gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, COLUMN, actors.getActors()[i].get_title().c_str(), -1);
	}
    
    return GTK_TREE_MODEL(store);
}

void updateActors() {
	displayMode = ACTORS;
	
	GtkTreeModel *model;
	model = getActorsModel();
    gtk_tree_view_set_model(GTK_TREE_VIEW(treeView), model);
	g_object_unref(model);
	
	gtk_label_set_text(GTK_LABEL(lbPage), "");
	
	setSensitiveItemsActors();
}

void *getActorsTask(void *args) {
	string link((char*)args);
	gdk_threads_enter();
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(pbStatus), "Searching actors...");
	gdk_threads_leave();
	HtmlString html_string(pbStatus);
	string page = html_string.get_string(link);
	gdk_threads_enter();
	actors.parse(page);
	if(!actors.getActors().empty()) {
	    //Save results 
		resultsBack.push_back(results);
		//Save actors
		actorsBack.push_back(actors);
		updateActors();	
		gtk_progress_bar_set_text(GTK_PROGRESS_BAR(pbStatus), "Done");
	}else {
	    gtk_progress_bar_set_text(GTK_PROGRESS_BAR(pbStatus), "Nothing found");	
	}
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(pbStatus), 0);
	gdk_threads_leave();
	return NULL;
}

void processResult(gint *indices, gint count) {//move to playlists
	if(count == 1) { //Node
		gint i = indices[0];
		Item item = results.getResults()[i];
		if(gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(rbActors))){
	        g_thread_create(getActorsTask, (void*) item.get_href().c_str(), FALSE, NULL);	
		}else {
	        g_thread_create(getPlaylistsTask, (void*) item.get_id().c_str(), FALSE, NULL);	
		}
	}
}

void processActor(gint *indices, gint count) {
	if(count == 1) { //Node
		gint i = indices[0];
		Item item = actors.getActors()[i];
	    g_thread_create(getResultsTask, (void*) item.get_href().c_str(), FALSE, NULL);
	}
}

void on_changed(GtkWidget *widget, gpointer statusbar) {
    
	GtkTreeIter iter;
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
				    processCategory(indices, count);
				    break;
				case RESULTS:
				    processResult(indices, count);
				    break;
				case ACTORS:
				    processActor(indices, count);
				    break;
				case PLAYLISTS:
				    processPlayItem(indices, count);
				    break;
			}
		}
	}
}

GtkWidget *create_view_and_model(void) {
    
	GtkTreeViewColumn *col;
	GtkCellRenderer *renderer;
	GtkWidget *view;
	
	view = gtk_tree_view_new();
	
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(col, "Results");
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), col);
	
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(col, renderer, TRUE);
	gtk_tree_view_column_add_attribute(col, renderer, 
	  "text", COLUMN);
	
	return view;
}

GtkTreeModel *getCategoriesModel() {
	GtkTreeStore *treestore;
	GtkTreeIter topLevel, child;
	
	treestore = gtk_tree_store_new(NUM_COLS,
				  G_TYPE_STRING);
	
	for(unsigned i = 0; i < categories.getCategories().size(); i++) {
		gtk_tree_store_append(treestore, &topLevel, NULL);
		gtk_tree_store_set(treestore, &topLevel, COLUMN,
		    categories.getCategories()[i].get_title().c_str(), -1);
		
		for(unsigned j = 0; j < categories.getCategories()[i].get_subctgs().size(); j++) {
			gtk_tree_store_append(treestore, &child, &topLevel);
			gtk_tree_store_set(treestore, &child, COLUMN, 
			    categories.getCategories()[i].get_subctgs()[j].get_title().c_str(), -1);
		}
	}
	
	return GTK_TREE_MODEL(treestore);
}

void updateCategories() {
	displayMode = CATEGORIES;
	clearActorsResults();
	GtkTreeModel *model;
	model = getCategoriesModel();
    gtk_tree_view_set_model(GTK_TREE_VIEW(treeView), model);
	g_object_unref(model);
	
	disableAllItems();
}

void *getCategoriesTask(void *arg) {
	/*string page = readFromFile("Categories.html");
	if(page.empty()) {
		cout << "Getting info from net..." << endl;
	    HtmlString html_string(pbStatus);
	    page = html_string.get_string(DOMAIN);
	    writeToFile("Categories.html", page);	
	}else {
		cout << "Getting info from file..." << endl;
		//cout << page << endl;
	}*/
	
	gdk_threads_enter();
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(pbStatus), "Searching categories...");
	gdk_threads_leave();
	
	HtmlString html_string(pbStatus);
	string page = html_string.get_string(DOMAIN);
	gdk_threads_enter();
	categories.parse_categories(page);
	if(!categories.getCategories().empty()) {
		updateCategories();
		gtk_progress_bar_set_text(GTK_PROGRESS_BAR(pbStatus), "Done");
	}else {
		gtk_progress_bar_set_text(GTK_PROGRESS_BAR(pbStatus), "Nothing found");
	}
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(pbStatus), 0);
	gdk_threads_leave();
	
	return NULL;
}

static void btnCategoriesClicked( GtkWidget *widget,
                      gpointer   treeView )
{
    if(categories.getCategories().empty()) {
		//Starting new thread to get categories from the net
		g_thread_create(getCategoriesTask, NULL, FALSE, NULL);
	}else {
		updateCategories();
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
		    if(!actorsBack.empty()) {
				//Restore actors
				actors = actorsBack.back();
				updateActors();
			}else if(!categories.getCategories().empty()) {
			    updateCategories();	
			}
	        break;
	    case ACTORS:
	        //Restore results
	        results = resultsBack.back();
	        resultsBack.pop_back();
	        actorsBack.pop_back();
	        updateResults();
	        break;
	    case PLAYLISTS:
	        updateResults();
	        break;
	}
}

static void btnPrevClicked( GtkWidget *widget,
                      gpointer   data )
{   
	g_thread_create(getResultsTask, (void*) results.getPrevLink().c_str(), FALSE, NULL);
}

static void btnNextClicked( GtkWidget *widget,
                      gpointer   data )
{   
	g_thread_create(getResultsTask, (void*) results.getNextLink().c_str(), FALSE, NULL);
}

static void entryActivated( GtkWidget *widget, 
                      gpointer data) {
    string query(gtk_entry_get_text(GTK_ENTRY(widget)));
    string base_url = string(DOMAIN) + "/?do=search&subaction=search&mode=simple&story=" + to_cp1251(query);
    results.setBaseUrl(base_url);
	g_thread_create(getResultsTask, (void*) base_url.c_str(), FALSE, NULL);						  
}


int main( int   argc,
          char *argv[] )
{
    GtkWidget *window;
    GtkWidget *sw;
    GtkWidget *vbox;
    GtkWidget *toolbar;
    
	GtkToolItem *btnCategories;
	GtkToolItem *sep;
	GtkToolItem *exit;
	
	 /* Must initialize libcurl before any threads are started */ 
    curl_global_init(CURL_GLOBAL_ALL);
    
    g_thread_init (NULL);
    gdk_threads_init ();
    gdk_threads_enter ();
    
    gtk_init(&argc, &argv);
    
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_window_set_title(GTK_WINDOW(window), PROG_NAME);
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    
    vbox = gtk_vbox_new(FALSE, 1);
    
    treeView = create_view_and_model();
    
    toolbar = gtk_toolbar_new();
	gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_ICONS);
	gtk_container_set_border_width(GTK_CONTAINER(toolbar), 2);
	
    btnCategories = gtk_tool_button_new_from_stock(GTK_STOCK_DIRECTORY);
    gtk_tool_item_set_tooltip_text(btnCategories, "Display categories");
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), btnCategories, -1);
    g_signal_connect(GTK_WIDGET(btnCategories), "clicked", G_CALLBACK(btnCategoriesClicked), NULL);
    
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
    
    gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(rbPlay), TRUE);
    
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), rbActors, -1);
    
    sep = gtk_separator_tool_item_new();
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), sep, -1);
    
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), rbPlay, -1);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), rbDownload, -1);
    
    sep = gtk_separator_tool_item_new();
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), sep, -1);
	
	exit = gtk_tool_button_new_from_stock(GTK_STOCK_QUIT);
	gtk_tool_item_set_tooltip_text(exit, "Quit program");
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), exit, -1);
	
	g_signal_connect(G_OBJECT(exit), "clicked",
	    G_CALLBACK(gtk_main_quit), NULL);
    
    disableAllItems();
    
    gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, FALSE, 1);
    
    sw = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),
            GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(sw),
            GTK_SHADOW_ETCHED_IN);
    
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(treeView), FALSE);
    gtk_container_add(GTK_CONTAINER(sw), treeView);
    gtk_box_pack_start(GTK_BOX(vbox), sw, TRUE, TRUE, 1);
    
    pbStatus = gtk_progress_bar_new();
    gtk_box_pack_start(GTK_BOX(vbox), pbStatus, FALSE, FALSE, 1);
    
    /*g_signal_connect(selection, "changed", 
         G_CALLBACK(on_changed), statusbar);*/
    
    g_signal_connect(treeView, "row-activated", 
        G_CALLBACK(on_changed), NULL);
    
    gtk_container_add(GTK_CONTAINER(window), vbox);
    
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 400);
    
    gtk_widget_show_all(window);
    
    gtk_main();
    gdk_threads_leave ();
    return 0;
}
