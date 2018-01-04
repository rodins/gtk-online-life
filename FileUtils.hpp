// FileUtils.hpp
#include <glib.h>
#include <glib/gstdio.h>
#include <fstream>

string homeAppSavesDir = string(g_get_home_dir()) + "/.gtk_online_life/saves";

class FileUtils {
	public:
	
    static string readFromFile(string title) { 
		string filename = homeAppSavesDir + "/" + title;
		ifstream in(filename.c_str());
		if(in) {
			return string((istreambuf_iterator<char>(in)), istreambuf_iterator<char>());
		}
		return "";
	}
	
	static void writeToFile(string title, string input) {
		g_mkdir_with_parents(homeAppSavesDir.c_str(), S_IRWXU); 
		string filename = homeAppSavesDir + "/" + title;
		ofstream out(filename.c_str());
		if(out) {
			out << input;
		}
	}
	
	static bool isTitleSaved(string title) {
		string filename = homeAppSavesDir + "/" + title;
		return g_file_test(filename.c_str(), G_FILE_TEST_EXISTS);
	}
	
	static void removeFile(string title) {
		string filename = homeAppSavesDir + "/" + title;
		g_remove(filename.c_str());
	}
	
	static void listSavedFiles(GtkWidget *tvSavedItems) {
		GtkListStore *storeSavedItems = GTK_LIST_STORE(gtk_tree_view_get_model(
		                                GTK_TREE_VIEW(tvSavedItems)));
		gtk_list_store_clear(storeSavedItems);
		GdkPixbuf *icon = IconsFactory::getLinkIcon();
		GtkTreeIter iter;
		GDir *dir;
		const gchar *filename;
		
		dir = g_dir_open(homeAppSavesDir.c_str(), 0, NULL);
		if(dir != NULL) {
			while ((filename = g_dir_read_name(dir))) {
				gtk_list_store_append(storeSavedItems, &iter);
			    gtk_list_store_set(storeSavedItems, 
			                       &iter, 
			                       IMAGE_COLUMN, 
			                       icon,
			                       TITLE_COLUMN, 
			                       filename,
			                       -1);                                    
			}
			g_dir_close(dir);
		}
	}
};
