// FileUtils.hpp
#include <glib.h>
#include <glib/gstdio.h>
#include <fstream>

string homeAppDir = string(g_get_home_dir()) + "/.gtk_online_life";

class FileUtils {
	public:
	
    static string readFromFile(string filename) { 
		filename = homeAppDir + "/" + filename;
		ifstream in(filename.c_str());
		if(in) {
			return string((istreambuf_iterator<char>(in)), istreambuf_iterator<char>());
		}
		return "";
	}
	
	static void writeToFile(string filename, string input) {
		g_mkdir_with_parents(homeAppDir.c_str(), S_IRWXU); 
		filename = homeAppDir + "/" + filename;
		ofstream out(filename.c_str());
		if(out) {
			out << input;
		}
	}
};
