// ActorsTask.hpp

class ActorsTask {
    GThreadPool *pool;
    ActorsView *view;
    ActorsModel *model;
    ActorsNet *net;
    public:
    ActorsTask(ActorsView *view, ActorsNet *net) {
		this->view = view;
		this->net = net;
		// GThreadPool for actors
	    pool = g_thread_pool_new(ActorsTask::task,
	                                   this,
	                                   1, // Run one thread at the time
	                                   FALSE,
	                                   NULL);
	}
	
    void start() {
        g_thread_pool_push(pool, (gpointer)1, NULL);
    }
    
    static void task(gpointer args, gpointer args2) {
		ActorsTask *task = (ActorsTask*)args2;
		// On pre execute
		gdk_threads_enter();
		task->view->showLoadingIndicator();
		gdk_threads_leave();
		CURLcode res = task->net->getData();
		// On post execute
		gdk_threads_enter();
		if(res == CURLE_WRITE_ERROR) { // This error means OK
			task->view->showData();
		}else {
		    task->view->showError();	
		}
		gdk_threads_leave();
	}
};
