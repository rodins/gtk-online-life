// FileUtils.hpp
#include <glib.h>
#include <glib/gstdio.h>
#include <fstream>

gchar *homeAppSavesDir = 
    g_build_filename(g_get_home_dir(), 
                     ".gtk_online_life", 
                     "saves", 
                     NULL);
                     
gchar *homeAppSavedImagesDir = 
    g_build_filename(g_get_home_dir(),
                     ".gtk_online_life",
                     "saved_images",
                     NULL);

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
	
	static GdkPixbuf* readImageFromFile(string title) {
		gchar *filename =
		    g_build_filename(homeAppSavedImagesDir,
		                     title.c_str(),
		                     NULL);
		return gdk_pixbuf_new_from_file(filename, NULL);
	}
	
	static void writeToFile(string title, string input) {
		g_mkdir_with_parents(homeAppSavesDir, S_IRWXU); 
		gchar *filename = g_build_filename(homeAppSavesDir, title.c_str(), NULL);
		ofstream out(filename);
		if(out) {
			out << input;
		}
	}
	
	static void writeImageToFile(string title, GdkPixbuf *pixbuf) {
		g_mkdir_with_parents(homeAppSavedImagesDir, S_IRWXU);
		gchar *filename = g_build_filename(homeAppSavedImagesDir,
		                                   title.c_str(),
		                                   NULL);
		gdk_pixbuf_save(pixbuf,
		                filename,
		                "png",
		                NULL,
		                NULL);
	}
	
	static bool isTitleSaved(string title) {
		gchar *filename = g_build_filename(homeAppSavesDir, title.c_str(), NULL);
		return g_file_test(filename, G_FILE_TEST_EXISTS);
	}
	
	static bool isImageSaved(string title) {
		gchar *filename =
		    g_build_filename(homeAppSavedImagesDir,
		                     title.c_str(),
		                     NULL);
		    return g_file_test(filename, G_FILE_TEST_EXISTS);
	}
	
	static void removeFile(string title) {
		gchar *filename = g_build_filename(homeAppSavesDir, title.c_str(), NULL);
		g_remove(filename);
	}
	
	static void removeImageFile(string title) {
		gchar *filename =
		    g_build_filename(homeAppSavedImagesDir,
		                     title.c_str(),
		                     NULL);
		g_remove(filename);
	}
	
	static void listSavedFiles(SavedItemsModel &model) {	
		GdkPixbuf *icon;
		GDir *dir;
		const gchar *filename;
		string href;
		
		model.clear();
		
		dir = g_dir_open(homeAppSavesDir, 0, NULL);
		if(dir != NULL) {
			while ((filename = g_dir_read_name(dir))) {
					if(FileUtils::isImageSaved(filename)) {
						icon = FileUtils::readImageFromFile(filename);
					}else {
						icon = IconsFactory::getBlankIcon();
					}
					if(FileUtils::isTitleSaved(filename)) {
						href = FileUtils::readFromFile(filename);
					}else {
						href = "";
					}
			        model.add(filename, icon, href);                                
			}
			g_dir_close(dir);
		}
	}
};
