// FileUtils.hpp
#include <glib.h>
#include <glib/gstdio.h>
#include <fstream>

gchar *homeAppSavesDir = g_build_filename(g_get_home_dir(), ".gtk_online_life", "saves", NULL);

class FileUtils {
	public:
	
    static string readFromFile(string title) { 
		gchar *filename = g_build_filename(homeAppSavesDir, title.c_str(), NULL);
		ifstream in(filename);
		if(in) {
			return string((istreambuf_iterator<char>(in)), istreambuf_iterator<char>());
		}
		return "";
	}
	
	static void writeToFile(string title, string input) {
		g_mkdir_with_parents(homeAppSavesDir, S_IRWXU); 
		gchar *filename = g_build_filename(homeAppSavesDir, title.c_str(), NULL);
		ofstream out(filename);
		if(out) {
			out << input;
		}
	}
	
	static bool isTitleSaved(string title) {
		gchar *filename = g_build_filename(homeAppSavesDir, title.c_str(), NULL);
		return g_file_test(filename, G_FILE_TEST_EXISTS);
	}
	
	static void removeFile(string title) {
		gchar *filename = g_build_filename(homeAppSavesDir, title.c_str(), NULL);
		g_remove(filename);
	}
	
	/*static void countSavedFiles(GtkToolItem *btnSavedItems) {
		GDir *dir;
		int count = 0;
		const gchar *filename;
		dir = g_dir_open(homeAppSavesDir, 0, NULL);
		if(dir != NULL) {
			while ((filename = g_dir_read_name(dir))) {
				count++;
			}
			g_dir_close(dir);
		}
		// Disable/enable show/hide list saved items button
		if(count > 0) {
			gtk_widget_set_sensitive(GTK_WIDGET(btnSavedItems), TRUE);
		}else {
			gtk_widget_set_sensitive(GTK_WIDGET(btnSavedItems), FALSE);
		}
	}*/
	
	static void listSavedFiles(GtkWidget *ivResults, 
	                           GtkToolItem *btnSavedItems) {
		GtkListStore *storeSavedItems = GTK_LIST_STORE(
		                                gtk_icon_view_get_model(
		                                GTK_ICON_VIEW(ivResults)));
		gboolean isActive = gtk_toggle_tool_button_get_active(
		                    GTK_TOGGLE_TOOL_BUTTON(btnSavedItems));	
		
		GdkPixbuf *icon;
		GtkTreeIter iter;
		GDir *dir;
		const gchar *filename;
		
		int count = 0;
		// If not active just count items
		if(isActive) {
			gtk_list_store_clear(storeSavedItems);
			icon = IconsFactory::getBlankIcon();
		}
		
		dir = g_dir_open(homeAppSavesDir, 0, NULL);
		if(dir != NULL) {
			while ((filename = g_dir_read_name(dir))) {
				if(isActive) {
				    gtk_list_store_append(storeSavedItems, &iter);
				    gtk_list_store_set(storeSavedItems, 
				                       &iter, 
				                       ICON_IMAGE_COLUMN, 
				                       icon,
				                       ICON_TITLE_COLUMN, 
				                       filename,
				                       ICON_HREF, 
                                       "",
                                       ICON_IMAGE_LINK, 
                                       "",
				                       -1);
				}  
			    count++;                                  
			}
			g_dir_close(dir);
		}
		// Disable/enable show/hide list saved items button
		if(count > 0) {
			gtk_widget_set_sensitive(GTK_WIDGET(btnSavedItems), TRUE);
		}else {
			gtk_widget_set_sensitive(GTK_WIDGET(btnSavedItems), FALSE);
		}
	}
};
