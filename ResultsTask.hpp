// ResultsTask.hpp

class ResultsTask {
	CenterView *view;
    GThreadPool *threadPool;
    ResultsNet *net;
    bool isPage;
    bool isThreadStarted;
    ResultsHistory *history;
    string title;
    public:
    ResultsTask(CenterView *view, ResultsNet *net, ResultsHistory *history) {
		this->view = view;
		this->net = net;
		this->history = history;
		isPage = FALSE;
		isThreadStarted = FALSE;
		threadPool = g_thread_pool_new(ResultsTask::task,
	                                   this,
	                                   1, // Run one thread at the time
	                                   FALSE,
	                                   NULL);
	}
	
	void setTitle(string title) {
		this->title = title;
		isPage = FALSE;
	}
	
	gboolean isStarted() {
		return isThreadStarted;
	}
	
	void newMode() {
		isPage = FALSE;
	}
	
	void appendMode() {
		isPage = TRUE;
	}
	
	void start() {
		if(!isThreadStarted) {
			isThreadStarted = TRUE;
			view->showLoadingIndicator(isPage);
			net->setMode(isPage); // needed to select url
			net->resetFirstItem(); // needed to hide loading indicator
		    g_thread_pool_push(threadPool,
		                      (gpointer)1,
		                       NULL);
		}
	}
	
	private:
	void onPostExecute(CURLcode res) {
		isThreadStarted = FALSE;
		if(res == CURLE_OK) {
			if(net->isEmpty()) {
				//view->showEmpty();
				view->showResultsData();
			}else {
				view->setSensitiveReferesh();
				if(!isPage) {
					history->removeBackStackDuplicate(title);
				}
			}
		}else {
			view->showError(isPage);
		}
		history->updatePrevNextButtons();
	}
	
	static void task(gpointer arg, gpointer arg1) {
		// On pre execute
		gdk_threads_enter();
		ResultsTask *task = (ResultsTask*)arg1;
	    gdk_threads_leave();
	    // async part
		CURLcode res = task->net->getResultsFromNet();
		gdk_threads_enter();
		task->onPostExecute(res);                                 
		gdk_threads_leave();
	}  
};
