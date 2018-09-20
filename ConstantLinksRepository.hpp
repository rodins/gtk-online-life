// ConstantLinksRepository.hpp

class ConstantLinksRepository {
	GThreadPool *threadPool;
	CenterView *view;
	PlaylistsRepository *playlistsRepository;
	PlayItemRepository *playItemRepository;
	ErrorDialogs *errorDialogs;
	public:
	ConstantLinksRepository(CenterView *view,
	                        PlaylistsRepository *playlistsRepository,
	                        PlayItemRepository *playItemRepository,
	                        ErrorDialogs *errorDialogs) {
        this->view = view;
        this->playlistsRepository = playlistsRepository;
        this->playItemRepository = playItemRepository;
        this->errorDialogs = errorDialogs;
		// GThreadPool for constant links
	    threadPool = g_thread_pool_new(ConstantLinksRepository::task,
	                                   this,
	                                   1, // Run one thread at the time
	                                   FALSE,
	                                   NULL); 
	}
	
	void getData(string href) {
		static string id;
	    id = PlaylistsUtils::get_href_id(href);
		g_thread_pool_push(threadPool,
		                  (gpointer)id.c_str(),
		                   NULL);
	}
	
	private:
    static void task(gpointer arg, gpointer arg2) {
		ConstantLinksRepository *repo = (ConstantLinksRepository *)arg2;
		string id((const char*)arg);
		
		// On pre execute
		gdk_threads_enter();
		// Show links spinner
		repo->view->showLoadingIndicator(FALSE);
		gdk_threads_leave();
		// Async part	
		string js = HtmlString::getConstantsPage(id);
		gdk_threads_enter();
		if(js.length() > 1000) { // Serial
			repo->view->showResultsData(); // For testing only. To be removed...
            repo->playlistsRepository->getData(js);
		}else { // Not serial
			repo->view->showResultsData();
		    if(js.length() > 500) { // Movie
			    repo->playItemRepository->getData(js);
		    }else if(js.length() > 0){ //  Links error
			    repo->errorDialogs->runLinksErrorDialog();
		    }else{
			    repo->errorDialogs->runNetErrorDialog();
		    }
		}
		gdk_threads_leave();
	}
};
