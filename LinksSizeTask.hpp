// LinksSizeTask.hpp
#include "LinksSizeNet.hpp"

class LinksSizeTask {
    GThreadPool *pool;
    GtkToolItem *btnActors;
    PlayItemDialog *dialog, *noActorsDialog, *actorsDialog;
    LinksSizeNet net;
    public:
    LinksSizeTask(GtkToolItem *btnActors, PlayItemDialog *dialog) {
		this->btnActors = btnActors;
		this->noActorsDialog = dialog;
		this->actorsDialog = NULL;
	    pool = g_thread_pool_new(LinksSizeTask::task,
	                                   this,
	                                   2, // Run two threads at the time
	                                   FALSE,
	                                   NULL);
    }
    
    void start(PlayItem *playItem) {
		g_thread_pool_push(pool, (gpointer)playItem, NULL);
	}
	
	void setActorsDialog(PlayItemDialog *actorsDialog) {
		this->actorsDialog = actorsDialog;
	}
    
    private:
    
    void switchDialog() {
		if(!gtk_toggle_tool_button_get_active(
		                       GTK_TOGGLE_TOOL_BUTTON(btnActors))) {
			dialog = actorsDialog;					   
		}else {
			dialog = noActorsDialog;
		}
	}
    
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
		task->switchDialog();
		task->dialog->create(playItem, sizeFile, sizeDownload);
		gdk_threads_leave();
	}
};
