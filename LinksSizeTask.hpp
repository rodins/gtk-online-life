// LinksSizeTask.hpp
#include "LinksSizeNet.hpp"

class LinksSizeTask {
    GThreadPool *pool;
    PlayItemDialog *dialog;
    LinksSizeNet net;
    public:
    LinksSizeTask(PlayItemDialog *dialog) {
		this->dialog = dialog;
	    pool = g_thread_pool_new(LinksSizeTask::task,
	                                   this,
	                                   2, // Run two threads at the time
	                                   FALSE,
	                                   NULL);
    }
    
    void start(PlayItem *playItem) {
		g_thread_pool_push(pool, (gpointer)playItem, NULL);
	}
    
    private:
    static void task(gpointer arg1, gpointer arg2) {
		LinksSizeTask *task = (LinksSizeTask*)arg2;
		PlayItem *playItem = (PlayItem*)arg1;
		string sizeFile, sizeDownload;
		if(playItem->file == playItem->download) {
			sizeDownload = task->net.getLinkSize(playItem->download);
		}else {
			sizeFile = task->net.getLinkSize(playItem->file);
		    sizeDownload = task->net.getLinkSize(playItem->download);
		}
		
		gdk_threads_enter();
		task->dialog->create(playItem, sizeFile, sizeDownload);
		gdk_threads_leave();
	}
};
