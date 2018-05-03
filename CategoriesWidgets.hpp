// CategoriesWidgets.hpp
#include "CategoriesParser.hpp"

class CategoriesWidgets {  
    GtkWidget *vbLeft;
    GtkWidget *swCategories;
    
    GtkWidget *spCategories;
    GtkWidget *hbCategoriesError;
    
    GtkWidget *tvCategories;
    
    GThreadPool *categoriesThreadPool;
    public:
    
    CategoriesWidgets(GtkWidget *vb_left, 
                      GtkWidget *sw_left_top,
                      GtkWidget *sp_categories, 
                      GtkWidget *hb_categories_error,
                      GtkWidget *tv_categories) {
	    vbLeft = vb_left;
	    swCategories = sw_left_top;
	    spCategories = sp_categories;
	    hbCategoriesError = hb_categories_error;
	    tvCategories = tv_categories;
	    
	    // GThreadPool for categories
	    categoriesThreadPool = g_thread_pool_new(CategoriesWidgets::categoriesTask,
	                                             this,
	                                             1, // Run one thread at the time
	                                             FALSE,
	                                             NULL);	  
	}
	
	void newThread() {
		g_thread_pool_push(categoriesThreadPool, (gpointer)1, NULL);
	}
	
	void showCategories() {
		//Get categories model
		GtkTreeModel *model = gtk_tree_view_get_model(
		                          GTK_TREE_VIEW(tvCategories));
		if(model == NULL) {
			//Starting new thread to get categories from the net  
            newThread();
		}
	}
	
	void btnCategoriesClicked() {
		if(!gtk_widget_get_visible(vbLeft)) {
			gtk_widget_show(vbLeft);
			showCategories();
		}else {
			gtk_widget_hide(vbLeft);
		}
	}
    
    void onPostExecute(string &page) {
		if(!page.empty()) {
			CategoriesParser categoriesParser(page);
			
			gtk_widget_set_visible(hbCategoriesError, FALSE);
			gtk_widget_set_visible(spCategories, FALSE);
			gtk_spinner_stop(GTK_SPINNER(spCategories));
			gtk_widget_set_visible(swCategories, TRUE);
			
			GtkTreeModel *model;
			model = categoriesParser.getModel();
		    gtk_tree_view_set_model(GTK_TREE_VIEW(tvCategories), model);
			g_object_unref(model);
		}else {
			showCategoriesError();
		}
	}
    
	static void categoriesTask(gpointer arg, gpointer arg1) {
		CategoriesWidgets *categoriesWidgets = (CategoriesWidgets*)arg1;
		// On pre execute
		gdk_threads_enter();
		categoriesWidgets->showSpCategories();
		gdk_threads_leave();
		string page = HtmlString::getCategoriesPage();
		gdk_threads_enter();
		categoriesWidgets->onPostExecute(page);
		gdk_threads_leave();
	}
	
	private:
	
	void showSpCategories() {
		gtk_widget_set_visible(vbLeft, TRUE);
		gtk_widget_set_visible(swCategories, FALSE);
		gtk_widget_set_visible(hbCategoriesError, FALSE);
		gtk_widget_set_visible(spCategories, TRUE);
		gtk_spinner_start(GTK_SPINNER(spCategories));
	}
	
	void showCategoriesError() {
		gtk_widget_set_visible(vbLeft, TRUE);
		gtk_widget_set_visible(swCategories, FALSE);
		gtk_widget_set_visible(hbCategoriesError, TRUE);
		gtk_widget_set_visible(spCategories, FALSE);
		gtk_spinner_stop(GTK_SPINNER(spCategories));
	}
};
