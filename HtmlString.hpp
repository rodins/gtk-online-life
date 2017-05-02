#include <string>
#include <curl/curl.h>
using namespace std;

class HtmlString {	
	
	static int my_progress_func(GtkWidget *bar,
                     double t, /* dltotal */ 
                     double d, /* dlnow */ 
                     double ultotal,
                     double ulnow)
	{
	    //printf("%d / %d (%g %%)\n", d, t, d*100.0/t);
		gdk_threads_enter();
		//gtk_progress_set_value(GTK_PROGRESS(bar), d*100.0/t);
		gtk_progress_bar_pulse(GTK_PROGRESS_BAR(bar));
		gdk_threads_leave();
		return 0;
	}
	
	
	static int writer(char *data, size_t size, size_t nmemb,
	                      string *writerData)
	{
	    if (writerData == NULL)
	       return 0;
	
	    writerData->append(data, size*nmemb);
	
	    return size *nmemb;
	}
	
	
	// Получение html странице в виде строки
	string url2string(const char *url)
	{
		CURL *curl_handle;
		string buffer;
		
		//curl_global_init(CURL_GLOBAL_ALL);
		
		/* init the curl session */
		curl_handle = curl_easy_init();
		
		if(curl_handle) {
		    
		    /* set url to get here */
			curl_easy_setopt(curl_handle, CURLOPT_URL, url);
			
			/* send all data to this function */
			curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, HtmlString::writer);
			
			curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
			
			if(_referer_link != "") {
				curl_easy_setopt(curl_handle, CURLOPT_REFERER, _referer_link.c_str());
			}
			
			curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &buffer);
			
			curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 0L);
			curl_easy_setopt(curl_handle, CURLOPT_PROGRESSFUNCTION, HtmlString::my_progress_func);
            curl_easy_setopt(curl_handle, CURLOPT_PROGRESSDATA, Bar);
			
			
			/* get it */
			curl_easy_perform(curl_handle);
			
			/* cleanup curl stuff */
			curl_easy_cleanup(curl_handle);
		    	
		}
		
		return buffer;
	}

public:
    HtmlString() {
		Bar = NULL;
	}
    
    HtmlString(GtkWidget *bar) {
		Bar = bar;
	}
	
	void setProgressBar(GtkWidget *bar) {
		Bar = bar;
	}
    
    string get_string(string url) {
		_referer_link = "";
		return url2string(url.c_str()); 
	}
	
	string get_string(string url, string referer_link) {
		_referer_link = referer_link;
		return url2string(url.c_str());
	}
	
private:
    string _referer_link;
    GtkWidget *Bar;
};