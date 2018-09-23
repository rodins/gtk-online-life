// ConstantLinksRepository.hpp

class ConstantLinksTask {
	GThreadPool *threadPool;
	CenterView *view;
	PlaylistsTask *playlistsTask;
	PlayItemProcessor *playItemProcessor;
	ErrorDialogs *errorDialogs;
	string title;
	public:
	ConstantLinksTask(CenterView *view,
	                  PlaylistsTask *playlistsTask,
	                  PlayItemProcessor *playItemProcessor,
	                  ErrorDialogs *errorDialogs) {
        this->view = view;
        this->playlistsTask = playlistsTask;
        this->playItemProcessor = playItemProcessor;
        this->errorDialogs = errorDialogs;
		// GThreadPool for constant links
	    threadPool = g_thread_pool_new(ConstantLinksTask::task,
	                                   this,
	                                   1, // Run one thread at the time
	                                   FALSE,
	                                   NULL); 
	}
	
	void start(string title, string href) {
		this->title = title;
		static string id;
	    id = PlaylistsUtils::get_href_id(href);
		g_thread_pool_push(threadPool,
		                  (gpointer)id.c_str(),
		                   NULL);
	}
	
	private:
    static void task(gpointer arg, gpointer arg2) {
		ConstantLinksTask *task = (ConstantLinksTask*)arg2;
		string id((const char*)arg);
		
		// On pre execute
		gdk_threads_enter();
		// Show links spinner
		task->view->showLoadingIndicator(FALSE);
		gdk_threads_leave();
		// Async part	
		string js = HtmlString::getConstantsPage(id);
		gdk_threads_enter();
		if(js.length() > 1000) { // Serial
            task->playlistsTask->start(task->title, js);
		}else { // Not serial
			task->view->showResultsData();
		    if(js.length() > 500) { // Movie
			    task->playItemProcessor->process(js);
		    }else if(js.length() > 0){ //  Links error
			    task->errorDialogs->runLinksErrorDialog();
		    }else{
			    task->errorDialogs->runNetErrorDialog();
		    }
		}
		gdk_threads_leave();
	}
};
