#include <gtk/gtk.h>
#include <vector>
#include <string>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <map>
#include <set>
#include <sstream>

#define SSTR( x ) static_cast< std::ostringstream & >( \
        ( std::ostringstream() << std::dec << x ) ).str()

#include "DomainFactory.hpp"
#include "Converter.hpp"
#include "HtmlString.hpp"
#include "IconsFactory.hpp"
#include "ColumnsEnum.hpp"
#include "ResultsModel.hpp"
#include "SavedItemsModel.hpp"
#include "FileUtils.hpp"
#include "CenterView.hpp"
#include "PlaylistsTask.hpp"
#include "ResultsController.hpp"
//#include "ActorsHistory.hpp"
#include "CategoriesModel.hpp"
#include "CategoriesParser.hpp"
#include "CategoriesNet.hpp"
#include "CategoriesView.hpp"
#include "CategoriesTask.hpp"
#include "CategoriesController.hpp"
#include "ImagesDownloader.hpp"
#include "PlayItem.hpp"
#include "PlaylistsUtils.hpp"
#include "PlayItemTask.hpp"
#include "ErrorDialogs.hpp"
#include "ActorsTask.hpp"
#include "ConstantLinksTask.hpp"
#include "ProcessResultController.hpp"

using namespace std;

void categoriesClicked(GtkTreeView *treeView,
                       GtkTreePath *path,
                       GtkTreeViewColumn *column,
                       ResultsController *controller) {
	// Get model from tree view
	GtkTreeModel *model = gtk_tree_view_get_model(treeView);
	
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
	    controller->newResults(string(parentTitle) + " - " + title, link);
		g_free(parentTitle);
	}else {
		controller->newResults(title, link);
	}
	
	g_free(title);
	g_free(link);
}

void actorsClicked(GtkTreeView *treeView,
                   GtkTreePath *path,
                   GtkTreeViewColumn *column,
                   ResultsController *controller) {
	// Get model from tree view
	GtkTreeModel *model = gtk_tree_view_get_model(treeView);
	
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
	                   
	controller->newResults(title, link);
	                   
	g_free(title);
	g_free(link);
}

/*void playlistClicked(GtkTreeView *treeView,
                     GtkTreePath *path,
                     GtkTreeViewColumn *column,
                     ActorsHistory *actorsHistory) {
	// Get model from tree view
	GtkTreeModel *model = gtk_tree_view_get_model(treeView);
	
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
		PlayItem playItem;
		playItem.comment = comment;
		playItem.file = file;
		playItem.download = download;
		playItem.player = actorsHistory->getPlayer();
		actorsHistory->runPlayItemDialog(playItem);
	}
	
	g_free(comment);
	g_free(file);
	g_free(download);                   
}*/

void resultFunc(GtkIconView *icon_view, 
                GtkTreePath *path, 
                gpointer data) {
	ProcessResultController *controller = (ProcessResultController*)data;
	// Get model from ivResults
	GtkTreeModel *model = gtk_icon_view_get_model(icon_view);
	
	// Get iter from path
	GtkTreeIter iter;
	gtk_tree_model_get_iter(model, &iter, path);
	
	// Get title and href value from iter
	gchar *resultTitle = NULL;
	gchar *href = NULL;
	GdkPixbuf *pixbuf = NULL;

	gtk_tree_model_get(model,
	                   &iter,
	                   ICON_IMAGE_COLUMN,
	                   &pixbuf, 
	                   ICON_TITLE_COLUMN,
	                   &resultTitle,
	                   ICON_HREF,
	                   &href,
	                   -1);
	                   
	controller->onClick(resultTitle, href, pixbuf);
	
	g_free(resultTitle);
	g_free(href);
}

void resultActivated(GtkWidget *widget,
                     GtkTreePath *path,
                     gpointer data) {
	gtk_icon_view_selected_foreach(GTK_ICON_VIEW(widget),
	                               resultFunc,
	                               data);
}

GtkWidget *createTreeView(void) {
	GtkTreeViewColumn *col;
	GtkCellRenderer *renderer;
	GtkWidget *view;
	
	view = gtk_tree_view_new();
	
	renderer = gtk_cell_renderer_pixbuf_new();
	col = gtk_tree_view_column_new_with_attributes ("Image", 
	                                                renderer,
                                                    "pixbuf", 
                                                    IMAGE_COLUMN,
                                                    NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(view), col);
	
    renderer = gtk_cell_renderer_text_new();
	col = gtk_tree_view_column_new_with_attributes ("Title", 
	                                                renderer,
                                                    "text", 
                                                    TITLE_COLUMN,
                                                    NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(view), col);
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(view), FALSE);
	return view;
}

static void btnCategoriesClicked(GtkWidget *widget,
                                 CategoriesController *controller) {
    controller->click();	
}

static void btnSavedItemsClicked(GtkToolItem *widget,
                                 ResultsController *controller) {
	controller->btnSavedItemsClicked();	
}

static void btnUpClicked( GtkWidget *widget,
                          ResultsController *controller) {
	controller->btnUpClicked();
}

static void btnPrevClicked( GtkToolButton *widget,
                            ResultsController *controller) {   
	controller->btnPrevClicked();  
}

static void btnNextClicked( GtkToolButton *widget,
                            ResultsController *controller) {   
	controller->btnNextClicked();
}

static void entryActivated( GtkWidget *widget, 
                            ResultsController *controller) {
    string query(gtk_entry_get_text(GTK_ENTRY(widget)));
    if(!query.empty()) {
	    string title = "Search: " + query;
	    string base_url = DomainFactory::getDomain() + 
	         "/?do=search&subaction=search&mode=simple&story=" + 
	         to_cp1251(query);
		controller->newResults(title, base_url);
	}		  						  
}

/*static void btnActorsClicked(GtkWidget *widget,
	                         ActorsHistory *actorsHistory){
	actorsHistory->btnActorsClicked();
}

static void backActorsChanged(GtkTreeSelection *treeselection,
	                          ActorsHistory *actorsHistory) {
	actorsHistory->changed(treeselection);
}*/

static void btnCategoriesRepeatClicked(GtkWidget *widget,
                CategoriesController *controller) {
	controller->repeat();
}

/*static void btnActorsRepeatClicked(GtkWidget *widget,
                                   ActorsHistory *actorsHistory) {
    actorsHistory->newActorsThread();
}*/

void swIconVScrollChanged(GtkAdjustment* adj,
	                      ResultsController *controller) {
	gdouble value = gtk_adjustment_get_value(adj);
	gdouble upper = gtk_adjustment_get_upper(adj);
	gdouble page_size = gtk_adjustment_get_page_size(adj);
	gdouble max_value = upper - page_size - page_size;
	if (value > max_value) {
		controller->append();
	}
}

static void btnRefreshClicked(GtkWidget *widget,
	                          ResultsController *controller) {
	controller->refresh();
}

static void btnResultsRepeatClicked(GtkWidget *widget,
	                                ResultsController *controller) {
	controller->repeat();
}

/*static void btnLinksErrorClicked(GtkWidget *widget,
	                             ActorsHistory *actorsHistory) {
	actorsHistory->btnLinksErrorClicked();
}

static void btnGetLinksClicked(GtkWidget *widget,
	                           ActorsHistory *actorsHistory) {
	actorsHistory->btnGetLinksClicked();
}

static void btnListEpisodesClicked(GtkWidget *widget,
	                               ResultsHistory *resultsHistory) {
	resultsHistory->btnListEpisodesClicked();
}

static void btnSaveClicked(GtkWidget *widget,
	                       ActorsHistory *actorsHistory) {
	actorsHistory->btnSaveClicked();
}

static void btnDeleteClicked(GtkWidget *widget,
	                         ActorsHistory *actorsHistory) {
	actorsHistory->btnDeleteClicked();
}*/

int main( int   argc,
          char *argv[] )
{   
	GtkWidget *window;
	GtkWidget *tvPlaylists;
	GtkWidget *ivResults;
	
	GtkWidget *frRightBottom;
	GtkWidget *lbInfo;
	GtkWidget *frRightTop,
	          *frInfo,
	          *frActions;
	          
	GtkWidget *spActors, *spLinks;
    GtkWidget *hbActorsError;
    
    GtkWidget *spCenter;
    GtkWidget *vbCenter;
    GtkWidget *hbResultsError;
    GtkWidget *btnResultsError;
    
    GtkWidget *vbLeft, *vbRight;
    GtkWidget *tvCategories, 
              *tvActors, 
              *tvBackActors;
              
    GtkWidget *spCategories;
    GtkWidget *hbCategoriesError;
    GtkWidget *swCategories;
	
    GtkWidget *vbox;
    GtkWidget *toolbar; 
    GtkWidget *hbCenter;    
    GtkWidget *swRightTop, *swRightBottom;
    GtkWidget *swTree, *swIcon;
    GtkWidget *btnCategoriesError;
    GtkWidget *btnActorsError;
    
    GtkWidget *btnGetLinks, 
              *btnListEpisodes, 
              *btnLinksError, 
              *btnSave,
              *btnDelete;
    GtkWidget *hbActions;
    
	GtkToolItem *btnCategories;
	GtkToolItem *btnSavedItems;
	GtkToolItem *btnRefresh;
	GtkToolItem *btnUp;
    GtkToolItem *btnPrev;
    GtkToolItem *btnNext;
    GtkWidget *entry;
    GtkToolItem *btnActors;
	GtkToolItem *sep;
	GtkToolItem *exit;
	
	GdkPixbuf *icon;
	
	GtkTreeSelection *selection; 
	
	const string PROG_NAME("Online life");
	set<int> *imageIndices = new set<int>();
	map<string, GdkPixbuf*> *imagesCache = new map<string, GdkPixbuf*>();
	
	 /* Must initialize libcurl before any threads are started */ 
    curl_global_init(CURL_GLOBAL_ALL);
    //TODO: make autodetection of old gtk
    //g_thread_init(NULL); // for Wary-5.5 (old gtk)
    
    gdk_threads_init ();
    gdk_threads_enter ();
    
    gtk_init(&argc, &argv);
    
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_window_set_title(GTK_WINDOW(window), PROG_NAME.c_str());
    gtk_container_set_border_width(GTK_CONTAINER(window), 5);
    icon = IconsFactory::getAppIcon();
	gtk_window_set_icon(GTK_WINDOW(window), icon);
    
    vbox = gtk_vbox_new(FALSE, 1);
    
    tvPlaylists = createTreeView();
    
    GtkTreeStore *playlistsStore = gtk_tree_store_new(PLAYLIST_NUM_COLS, 
						                              GDK_TYPE_PIXBUF,
									                  G_TYPE_STRING,
									                  G_TYPE_STRING,
									                  G_TYPE_STRING);
    gtk_tree_view_set_model(GTK_TREE_VIEW(tvPlaylists),
                            GTK_TREE_MODEL(playlistsStore));
    g_object_unref(playlistsStore);
    
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
	
	btnPrev = gtk_tool_button_new_from_stock(GTK_STOCK_GO_BACK);
	btnNext = gtk_tool_button_new_from_stock(GTK_STOCK_GO_FORWARD);
	
    toolbar = gtk_toolbar_new();
	gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_ICONS);
	gtk_container_set_border_width(GTK_CONTAINER(toolbar), 2);
	
    btnCategories = gtk_toggle_tool_button_new_from_stock(GTK_STOCK_DIRECTORY);
    gtk_tool_item_set_tooltip_text(btnCategories, "Show categories");
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), btnCategories, -1);
        
    sep = gtk_separator_tool_item_new();
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), sep, -1);
	
	GtkWidget *bookmarkIcon = gtk_image_new_from_pixbuf(
	                                        IconsFactory::getBookmarkIcon24());
	btnSavedItems = gtk_toggle_tool_button_new();
	gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(btnSavedItems), bookmarkIcon);
	gtk_tool_item_set_tooltip_text(btnSavedItems, "Show bookmarks");
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), btnSavedItems, -1);
	
	sep = gtk_separator_tool_item_new();
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), sep, -1);
	
	btnRefresh = gtk_tool_button_new_from_stock(GTK_STOCK_REFRESH);
    gtk_tool_item_set_tooltip_text(btnRefresh, "Update results");
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), btnRefresh, -1);
        
    sep = gtk_separator_tool_item_new();
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), sep, -1);
    
    btnUp = gtk_tool_button_new_from_stock(GTK_STOCK_GO_UP);
    gtk_tool_item_set_tooltip_text(btnUp, "Move up");
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), btnUp, -1);
    
    gtk_tool_item_set_tooltip_text(btnPrev, "Go back in history");
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), btnPrev, -1);
    
    gtk_tool_item_set_tooltip_text(btnNext, "Go forward in history");
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), btnNext, -1);
    
    sep = gtk_separator_tool_item_new();
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), sep, -1);
    
    entry = gtk_entry_new();
    gtk_widget_set_tooltip_text(entry, "Search online-life");
	GtkToolItem *entryItem = gtk_tool_item_new();
	gtk_container_add(GTK_CONTAINER(entryItem), entry);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), entryItem, -1);
    
    btnActors = gtk_toggle_tool_button_new_from_stock("gtk-info");
        
    gtk_tool_item_set_tooltip_text(btnActors, "Show info");
    
    sep = gtk_separator_tool_item_new();
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), sep, -1);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), btnActors, -1);
    
    sep = gtk_separator_tool_item_new();
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), sep, -1);
	
	exit = gtk_tool_button_new_from_stock(GTK_STOCK_QUIT);
	gtk_tool_item_set_tooltip_text(exit, "Quit program");
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), exit, -1);
	
	g_signal_connect(G_OBJECT(exit), "clicked",
	    G_CALLBACK(gtk_main_quit), NULL);
    
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
    GtkListStore *store = gtk_list_store_new(NUM_COLS, 
                                             GDK_TYPE_PIXBUF, 
                                             G_TYPE_STRING);    
    gtk_tree_view_set_model(GTK_TREE_VIEW(tvBackActors), 
        GTK_TREE_MODEL(store));
	g_object_unref(store);
	
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tvBackActors));

    tvCategories = createTreeView();
    tvActors = createTreeView();
    
    swCategories = gtk_scrolled_window_new(NULL, NULL);
    swRightTop = gtk_scrolled_window_new(NULL, NULL);
    swRightBottom = gtk_scrolled_window_new(NULL, NULL);
    
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(swCategories),
            GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(swCategories),
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
    gtk_container_add(GTK_CONTAINER(swCategories), tvCategories);
    gtk_container_add(GTK_CONTAINER(swRightTop), tvActors);
    gtk_container_add(GTK_CONTAINER(swRightBottom), tvBackActors);
    
    // Frames
    frRightTop = gtk_frame_new("Actors");
    frRightBottom = gtk_frame_new("Actors history");
    
    // Add categories spinner and error
    hbCategoriesError = gtk_hbox_new(FALSE, 1); // for Repeat button normal size
    spCategories = gtk_spinner_new();
    btnCategoriesError = gtk_button_new_with_label("Repeat");
    
    gtk_box_pack_start(GTK_BOX(hbCategoriesError), btnCategoriesError, TRUE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(vbLeft), swCategories, TRUE, TRUE, 1);
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
    
    // Actions frame in actors pane
    frActions = gtk_frame_new("Actions");
    
    //btnGetLinks
    btnGetLinks = gtk_button_new();
    GtkWidget *copyImage = gtk_image_new_from_stock(GTK_STOCK_MEDIA_PLAY,
                                                    GTK_ICON_SIZE_BUTTON);
    gtk_button_set_image(GTK_BUTTON(btnGetLinks), copyImage);
    gtk_widget_set_tooltip_text(btnGetLinks, "Get links");
    
    //btnListEpisodes
    btnListEpisodes = gtk_button_new();
    GtkWidget *openImage = gtk_image_new_from_stock(GTK_STOCK_DIRECTORY,
                                                    GTK_ICON_SIZE_BUTTON);
    gtk_button_set_image(GTK_BUTTON(btnListEpisodes), openImage);
    gtk_widget_set_tooltip_text(btnListEpisodes, "List episodes");
    
    //btnLinksError
    btnLinksError = gtk_button_new();
    GtkWidget *refreshImage = gtk_image_new_from_stock(GTK_STOCK_REFRESH,
                                                       GTK_ICON_SIZE_BUTTON);
    gtk_button_set_image(GTK_BUTTON(btnLinksError), refreshImage);
    gtk_widget_set_tooltip_text(btnLinksError, "Repeat");
    
    //btnSave
    btnSave = gtk_button_new();
    GtkWidget *saveImage = gtk_image_new_from_stock(GTK_STOCK_ADD,
                                                    GTK_ICON_SIZE_BUTTON);
    gtk_button_set_image(GTK_BUTTON(btnSave), saveImage);
    gtk_widget_set_tooltip_text(btnSave, "Add to bookmarks");
    
    //btnDelete
    btnDelete = gtk_button_new();
    GtkWidget *deleteImage = gtk_image_new_from_stock(GTK_STOCK_REMOVE,
                                                      GTK_ICON_SIZE_BUTTON);
    gtk_button_set_image(GTK_BUTTON(btnDelete), deleteImage);
    gtk_widget_set_tooltip_text(btnDelete, "Remove from bookmarks");
    
    spLinks = gtk_spinner_new();
    hbActions = gtk_hbox_new(FALSE, 1);
    gtk_box_pack_start(GTK_BOX(hbActions), spLinks, TRUE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(hbActions), btnLinksError, TRUE, TRUE, 1);
    gtk_box_pack_start(GTK_BOX(hbActions), btnGetLinks, TRUE, TRUE, 1);
    gtk_box_pack_start(GTK_BOX(hbActions), btnListEpisodes, TRUE, TRUE, 1);
    gtk_box_pack_end(GTK_BOX(hbActions), btnSave, TRUE, TRUE, 1);
    gtk_box_pack_end(GTK_BOX(hbActions), btnDelete, TRUE, TRUE, 1);
    gtk_container_add(GTK_CONTAINER(frActions), hbActions);
    
    gtk_box_pack_start(GTK_BOX(hbActorsError), btnActorsError, TRUE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(vbRight), spActors, TRUE, FALSE, 1);
    gtk_box_pack_start(GTK_BOX(vbRight), hbActorsError, TRUE, FALSE, 1);
    gtk_box_pack_start(GTK_BOX(vbRight), frActions, FALSE, FALSE, 1);
    gtk_widget_set_size_request(spActors, 32, 32);
    gtk_widget_set_size_request(spLinks, 32, 32);
    
    // add vbox center
    vbCenter = gtk_vbox_new(FALSE, 1);
    // add items to vbCenter
    gtk_box_pack_start(GTK_BOX(vbCenter), swTree, TRUE, TRUE, 1);
    gtk_box_pack_start(GTK_BOX(vbCenter), swIcon, TRUE, TRUE, 1);
    gtk_box_pack_start(GTK_BOX(vbCenter), spCenter, TRUE, FALSE, 1);
    gtk_box_pack_start(GTK_BOX(vbCenter), hbResultsError, TRUE, FALSE, 1);
								  
	SavedItemsModel savedItemsModel;
								 
	CenterView centerView(window, PROG_NAME, ivResults, vbCenter, spCenter, 
	                      swIcon, swTree, hbResultsError, btnSavedItems, 
	                      btnRefresh, btnUp, btnPrev, btnNext, &savedItemsModel);
	
	PlaylistsTask playlistsTask(&centerView,
	                            gtk_tree_view_get_model(
	                            GTK_TREE_VIEW(tvPlaylists)));
	                      
	ResultsController resultsController(&centerView, 
	                                    imagesCache,
	                                    imageIndices,
	                                    playlistsTask);
	
	PlayItemTask playItemTask;
	ErrorDialogs errorDialogs(window);                                    
	ActorsTask actorsTask;
	ConstantLinksTask constantLinksTask(&centerView, 
	                                    &playlistsTask,
	                                    &playItemTask,
	                                    &errorDialogs);
	                                    
	ProcessResultController processResultController(btnActors,
	                                                &actorsTask,
	                                                &constantLinksTask);
    
    // Disable all items                                                
    gtk_widget_set_sensitive(GTK_WIDGET(btnRefresh), FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(btnUp), FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(btnPrev), FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(btnNext), FALSE);
	
	// Disable listItems button only if there are no saved items
	FileUtils::listSavedFiles(savedItemsModel);
	centerView.setSensitiveSavedItems();
	// Show saved results on start if any
	if(!savedItemsModel.isEmpty()) {
		centerView.setSavedItemsActive(TRUE);
		resultsController.btnSavedItemsClicked();
	}
	
    g_signal_connect(GTK_WIDGET(btnRefresh), 
				     "clicked", 
				     G_CALLBACK(btnRefreshClicked), 
				     &resultsController);
				 
    g_signal_connect(GTK_WIDGET(btnUp),
                     "clicked", 
                     G_CALLBACK(btnUpClicked), 
                     &resultsController);
    
    g_signal_connect(btnPrev,
                     "clicked", 
                     G_CALLBACK(btnPrevClicked),
                     &resultsController);
                     
	g_signal_connect(btnNext,
				     "clicked", 
				     G_CALLBACK(btnNextClicked),
				     &resultsController);
				     
    g_signal_connect(btnResultsError,
                     "clicked",
                     G_CALLBACK(btnResultsRepeatClicked),
                     &resultsController);
				     
    g_signal_connect(tvCategories,
                     "row-activated",
                     G_CALLBACK(categoriesClicked), 
                     &resultsController);
        
    g_signal_connect(tvActors, 
                     "row-activated",
                     G_CALLBACK(actorsClicked), 
                     &resultsController);
    
    g_signal_connect(entry,
                     "activate", 
                     G_CALLBACK(entryActivated), 
                     &resultsController);
                                      
    /*g_signal_connect(btnListEpisodes,
                     "clicked",
                     G_CALLBACK(btnListEpisodesClicked),
                     &resultsHistory);*/
                     
    // IconView scroll to the bottom detection code
    GtkAdjustment *vadjustment;
    vadjustment = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(swIcon));
    g_signal_connect(vadjustment, 
                     "value-changed",
                     G_CALLBACK(swIconVScrollChanged), 
                     &resultsController); 
    
    /*ActorsHistory actorsHistory(window,
                                tvActors,
                                tvBackActors,
                                frRightBottom,
                                lbInfo,
                                frRightTop,
                                frInfo,
                                frActions,
                                spActors,
                                hbActorsError,
                                vbRight,
                                spLinks,
                                btnLinksError,
                                btnGetLinks,
                                btnListEpisodes,
                                btnSave,
                                btnDelete,
                                btnSavedItems,
                                btnActors,
                                &resultsHistory);
                                                     
    g_signal_connect(selection,
	                 "changed", 
	                 G_CALLBACK(backActorsChanged), 
	                 &actorsHistory);
	                                  
    g_signal_connect(btnActors,
                     "clicked", 
                     G_CALLBACK(btnActorsClicked),
                     &actorsHistory);
                     
    g_signal_connect(btnActorsError,
                     "clicked",
                     G_CALLBACK(btnActorsRepeatClicked), 
                     &actorsHistory);
                     
    g_signal_connect(tvPlaylists,
                     "row-activated", 
                     G_CALLBACK(playlistClicked), 
                     &actorsHistory);
                     
    g_signal_connect(btnLinksError,
                     "clicked",
                     G_CALLBACK(btnLinksErrorClicked),
                     &actorsHistory);
                     
    g_signal_connect(btnGetLinks,
                     "clicked",
                     G_CALLBACK(btnGetLinksClicked),
                     &actorsHistory);
                     
    g_signal_connect(btnSave,
                     "clicked",
                     G_CALLBACK(btnSaveClicked),
                     &actorsHistory);
                     
    g_signal_connect(btnDelete,
                     "clicked",
                     G_CALLBACK(btnDeleteClicked),
                     &actorsHistory);*/
                     
    g_signal_connect(ivResults, 
                     "item-activated", 
                     G_CALLBACK(resultActivated), 
                     &processResultController);
                     
    CategoriesModel categoriesModel;
    CategoriesParser categoriesParser(&categoriesModel);                 
    gtk_tree_view_set_model(GTK_TREE_VIEW(tvCategories), 
	                           categoriesModel.getTreeModel());                
    CategoriesView categoriesView(vbLeft,
                                  spCategories, 
                                  swCategories, 
                                  hbCategoriesError);
    CategoriesNet categoriesNet(&categoriesParser);
    CategoriesTask categoriesTask(&categoriesView, &categoriesNet);
    CategoriesController categoriesController(&categoriesTask);              
                                           
    g_signal_connect(btnCategoriesError, 
                     "clicked", 
                     G_CALLBACK(btnCategoriesRepeatClicked), 
                     &categoriesController);
                     
    g_signal_connect(GTK_WIDGET(btnCategories),
                     "clicked", 
                     G_CALLBACK(btnCategoriesClicked),
                     &categoriesController);
                     
    g_signal_connect(GTK_WIDGET(btnSavedItems), 
				     "clicked", 
				     G_CALLBACK(btnSavedItemsClicked), 
				     &resultsController);
        
    //vbRight
    gtk_box_pack_start(GTK_BOX(vbRight), frRightBottom, TRUE, TRUE, 1);
            
    //hbCenter
    gtk_box_pack_start(GTK_BOX(hbCenter), vbLeft, FALSE, FALSE, 1);
    gtk_box_pack_start(GTK_BOX(hbCenter), vbCenter, TRUE, TRUE, 1);
    gtk_box_pack_start(GTK_BOX(hbCenter), vbRight, FALSE, FALSE, 1);
    
    gtk_box_pack_start(GTK_BOX(vbox), hbCenter, TRUE, TRUE, 1);
    
    ImagesDownloader imagesDownloader(ivResults, imageIndices, imagesCache);    
    
    gtk_container_add(GTK_CONTAINER(window), vbox);
    
    gtk_window_set_default_size(GTK_WINDOW(window), 700, 400);
    
    //TODO: not show all and hide afterwards, show only what is needed
    gtk_widget_show_all(window);
    
    gtk_widget_hide(vbLeft);
    gtk_widget_hide(swCategories);
    
    gtk_widget_hide(vbRight);
    
    gtk_widget_hide(swTree);
    gtk_widget_hide(frRightBottom);
    
    gtk_widget_hide(spCenter);
    gtk_widget_set_size_request(spCenter, 32, 32);
    
    gtk_widget_hide(spCategories);
    gtk_widget_hide(hbCategoriesError);
    gtk_widget_set_size_request(spCategories, 32, 32);
    
    gtk_widget_hide(spActors);
    gtk_widget_hide(hbActorsError);
    gtk_widget_hide(frActions);
    
    gtk_widget_hide(hbResultsError);
                                   
    gtk_main();
    gdk_threads_leave ();
    
    HtmlString::cleanup();
    /* we're done with libcurl, so clean it up */ 
	curl_global_cleanup();

    return 0;
}
