// CategoriesWidgets.hpp
#include "CategoriesParser.hpp"

class CategoriesWidgets {  
    GtkWidget *vbLeft;
    GtkWidget *swLeftTop;
    
    GtkWidget *spCategories;
    GtkWidget *hbCategoriesError;
    
    GtkWidget *tvCategories;
    public:
    
    CategoriesWidgets(GtkWidget *vb_left, GtkWidget *sw_left_top,
                      GtkWidget *sp_categories, GtkWidget *hb_categories_error,
                      GtkWidget *tv_categories) {
	    vbLeft = vb_left;
	    swLeftTop = sw_left_top;
	    spCategories = sp_categories;
	    hbCategoriesError = hb_categories_error;
	    tvCategories = tv_categories;		  
	}
	
	void btnCategoriesClicked() {
		if(!gtk_widget_get_visible(vbLeft)) { // Categories hidden
			//Get categories model
			GtkTreeModel *model = gtk_tree_view_get_model(
			                          GTK_TREE_VIEW(tvCategories));
			if(model == NULL) {
				//Starting new thread to get categories from the net  
	            g_thread_new(NULL, CategoriesWidgets::categoriesTask, this);
			}else {
				gtk_widget_set_visible(vbLeft, TRUE);
			}
		}else { // Categories visible
			gtk_widget_set_visible(vbLeft, FALSE);
		}
	}
    
    void onPostExecute(string &page) {
		if(!page.empty()) {
			CategoriesParser categoriesParser(page);
			
			gtk_widget_set_visible(hbCategoriesError, FALSE);
			gtk_widget_set_visible(spCategories, FALSE);
			gtk_spinner_stop(GTK_SPINNER(spCategories));
			gtk_widget_set_visible(swLeftTop, TRUE);
			
			GtkTreeModel *model;
			model = categoriesParser.getModel();
		    gtk_tree_view_set_model(GTK_TREE_VIEW(tvCategories), model);
			g_object_unref(model);
		}else {
			showCategoriesError();
		}
	}
    
	static gpointer categoriesTask(gpointer arg) {
		CategoriesWidgets * categoriesWidgets = (CategoriesWidgets *)arg;
		// On pre execute
		gdk_threads_enter();
		categoriesWidgets->showSpCategories();
		gdk_threads_leave();
		string page = HtmlString::getCategoriesPage();
		gdk_threads_enter();
		categoriesWidgets->onPostExecute(page);
		gdk_threads_leave();
		return NULL;
	}
	
	private:
	
	void showSpCategories() {
		gtk_widget_set_visible(vbLeft, TRUE);
		gtk_widget_set_visible(swLeftTop, FALSE);
		gtk_widget_set_visible(hbCategoriesError, FALSE);
		gtk_widget_set_visible(spCategories, TRUE);
		gtk_spinner_start(GTK_SPINNER(spCategories));
	}
	
	void showCategoriesError() {
		gtk_widget_set_visible(vbLeft, TRUE);
		gtk_widget_set_visible(swLeftTop, FALSE);
		gtk_widget_set_visible(hbCategoriesError, TRUE);
		gtk_widget_set_visible(spCategories, FALSE);
		gtk_spinner_stop(GTK_SPINNER(spCategories));
	}
};