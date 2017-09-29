//ImagesDownloader.hpp

struct ArgsStruct {
	GdkPixbufLoader* loader;
	GtkTreeIter iter;
	GtkWidget *ivResults;
};

class ImagesDownloader {
    GtkWidget *ivResults;
    GThreadPool *imagesThreadPool;
    set<int> *imageIndexes;
    
    public:
    
    ImagesDownloader(GtkWidget *ir, set<int> *ii) {
		ivResults = ir;
		imageIndexes = ii;
		
		// GThreadPool for downloading images
	    imagesThreadPool = g_thread_pool_new(ImagesDownloader::imageDownloadTask,
	                                         this, 
	                                         -1,
	                                         FALSE,
	                                         NULL);
	}
	
	void newThread(int index) {
		if(imageIndexes->count(index) == 0) {
			imageIndexes->insert(index);
			g_thread_pool_push(imagesThreadPool, 
			    (gpointer)(long)(index+1),
			     NULL);
		}
	}
	
	private:
	
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
				(GTK_ICON_VIEW(args->ivResults)));
			gtk_list_store_set(store, &args->iter, IMAGE_COLUMN, pixbufOrig, -1);  
			gdk_threads_leave();
		}
		
		return realsize;
	}
	
	static void imageDownloadTask(gpointer arg, gpointer arg1) {
		ImagesDownloader *imagesDownloader = (ImagesDownloader*)arg1;
		gint index = (gint)(long)arg;
		index--;
		
		gdk_threads_enter();
		GtkTreeModel* model = gtk_icon_view_get_model(
		     GTK_ICON_VIEW(imagesDownloader->ivResults)
		);
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
				imagesDownloader->getPixbufFromUrl(link, iter, index);
			}
		}
	}
	
	void getPixbufFromUrl(string url, GtkTreeIter iter, int index) {
		CURL *curl_handle;
		CURLcode res;
		GdkPixbuf *pixbuf;
		
		GdkPixbufLoader* loader = gdk_pixbuf_loader_new();
		struct ArgsStruct args;
		args.loader = loader;
		args.iter = iter;
		args.ivResults = ivResults;
		
		/* init the curl session */ 
		curl_handle = curl_easy_init();
		
		/* remove crash bug */
		curl_easy_setopt(curl_handle, CURLOPT_NOSIGNAL, 1);
		
		/* specify URL to get */ 
		curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
		
		/* send all data to this function  */ 
		curl_easy_setopt(curl_handle, 
		                 CURLOPT_WRITEFUNCTION, 
		                 ImagesDownloader::WriteMemoryCallback);
		
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
	        imageIndexes->erase(index);
		}else { 
	        GError *error = NULL;
			gboolean ok = gdk_pixbuf_loader_close(loader, &error);
			if(!ok) {
		        fprintf(stderr, "On close: %s\n", error->message);
		        // In case of error remove index from imageIndexes 
		        // to have a chance to reload image
		        imageIndexes->erase(index);
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
};