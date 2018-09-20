// CategoriesTask.hpp

class CategoriesTask {
    CategoriesView *view;
    GThreadPool *threadPool;
    CategoriesParser *parser;
    gboolean isStarted;
    public:
    CategoriesTask(CategoriesView *view, CategoriesParser *parser) {
		this->view = view;
		this->parser = parser;
	    threadPool = g_thread_pool_new(CategoriesTask::task,
	                                             this,
	                                             1, // Run one thread at the time
	                                             FALSE,
	                                             NULL);
	    isStarted = FALSE;
	}
	
	void start() {
		view->btnCategoriesClicked();
		getData();
	}
	
	void repeat() {
		getData();
	}
	
	private:
	
	void getData() {
		if(parser->isModelEmpty() && !isStarted) {
			isStarted = TRUE;
			view->showLoadingIndicator();
		    g_thread_pool_push(threadPool, (gpointer)1, NULL);
		}
	}
	
	static void task(gpointer arg, gpointer arg1) {
		CategoriesTask *task = (CategoriesTask*)arg1;
		string page = HtmlString::getCategoriesPage();
		gdk_threads_enter();
		task->isStarted = FALSE;
		if(!page.empty()) {
			task->parser->parse(page);
			task->view->showData();
		}else {
			task->view->showError();
		}
		gdk_threads_leave();
	}
};
