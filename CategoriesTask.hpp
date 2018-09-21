// CategoriesTask.hpp

class CategoriesTask {
    CategoriesView *view;
    GThreadPool *threadPool;
    CategoriesNet *net;
    gboolean isStarted;
    public:
    CategoriesTask(CategoriesView *view, CategoriesNet *net) {
		this->view = view;
		this->net = net;
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
		if(net->isEmpty() && !isStarted) {
			isStarted = TRUE;
			view->showLoadingIndicator();
		    g_thread_pool_push(threadPool, (gpointer)1, NULL);
		}
	}
	
	static void task(gpointer arg, gpointer arg1) {
		CategoriesTask *task = (CategoriesTask*)arg1;
		task->net->getData();
		gdk_threads_enter();
		task->isStarted = FALSE;
		if(!task->net->isEmpty()) {
			task->view->showData();
		}else {
			task->view->showError();
		}
		gdk_threads_leave();
	}
};
