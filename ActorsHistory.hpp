//ActorsHistory.hpp
#include "Actors.hpp"

class ActorsHistory {
    Actors actors, prevActors;
    map <string, Actors> backActors;
    
    GtkWidget *tvActors, *tvBackActors;
    GtkWidget *frRightBottom;
    GdkPixbuf *icon;
    GtkWidget *lbInfo;
    
    GtkWidget *frRightTop, *frInfo;
    GtkWidget *spActors;
    GtkWidget *hbActorsError;
    GtkWidget *vbRight;
    public:
    
    ActorsHistory(GtkWidget *a, GtkWidget *pa, GtkWidget* fr, GtkWidget* li,
                  GtkWidget *frt, GtkWidget *fri, GtkWidget *sa, GtkWidget *hba,
                  GtkWidget *vbr) {
		tvActors = a;
		tvBackActors = pa;
		frRightBottom = fr;
		lbInfo = li;
		
		frRightTop = frt;
		frInfo = fri;
		spActors = sa;
		hbActorsError = hba;
		vbRight = vbr;
		
		icon = create_pixbuf("link_16.png");
	} 
	
	string onPreExecute() {
	    showSpActors();
	    return actors.getUrl();
	}
	
	void onPostExecute(string &page) {
		if(!page.empty()) {
			actors.parse(page);
			// Save to back actors map
			if(backActors.count(prevActors.getTitle()) == 0 
			        && prevActors.getCount() > 0
			        && prevActors.getTitle() != actors.getTitle()) {
			    backActors[prevActors.getTitle()] = prevActors;
			    backActorsListAdd(prevActors.getTitle());	
			}
			updateActors();
			showActors();
		}else {
		    showActorsError();
		}
	}
    
    void saveActors(string title, string href) {
		if(!actors.getTitle().empty()) {
			prevActors = actors;
		}
		actors.setTitle(title);
		actors.setUrl(href);
	}
    
	void changed(GtkTreeSelection *treeselection) {
		GtkTreeIter iter;
		GtkTreeModel *model;
		gchar *value;
		
		if (gtk_tree_selection_get_selected(treeselection, 
		                                    &model, 
		                                    &iter)) {
			gtk_tree_model_get(model, &iter, TITLE_COLUMN, &value,  -1);
			if(backActors.count(actors.getTitle()) == 0) {
				backActors[actors.getTitle()] = actors; // save prev actors
				backActorsListAdd(actors.getTitle());
			}
			actors = backActors[string(value)]; // set new actors
			updateActors();
			g_free(value);
	    }
	}
	
	void rbActorsClicked(GtkWidget *widget) {
		//Toggle visibility of actors list (vbRight)
		if(!gtk_widget_get_visible(vbRight)) {
			if(actors.getCount() > 0 && 
			  gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(widget))) {
				gtk_widget_set_visible(vbRight, TRUE);
			}
		}else {
			gtk_widget_set_visible(vbRight, FALSE);
		}
	}
	
	static void actorsTask(gpointer args, gpointer args2) {
		ActorsHistory *actorsHistory = (ActorsHistory*)args2;
		// On pre execute
		gdk_threads_enter();
		string link = actorsHistory->onPreExecute();
		gdk_threads_leave();
		string page = HtmlString::getActorsPage(link);
		// On post execute
		gdk_threads_enter();
		actorsHistory->onPostExecute(page);
		gdk_threads_leave();
	}
	
	private:
	
	void showSpActors() {
		gtk_widget_set_visible(frInfo, FALSE);
		gtk_widget_set_visible(frRightTop, FALSE);
		gtk_widget_set_visible(hbActorsError, FALSE);
		gtk_widget_set_visible(spActors, TRUE);
		gtk_spinner_start(GTK_SPINNER(spActors));
		gtk_widget_set_visible(vbRight, TRUE);
	}
	
	void showActors() {
		gtk_widget_set_visible(frInfo, TRUE);
		gtk_widget_set_visible(frRightTop, TRUE);
		gtk_widget_set_visible(hbActorsError, FALSE);
		gtk_widget_set_visible(spActors, FALSE);
		gtk_spinner_stop(GTK_SPINNER(spActors));
	}
	
	void showActorsError() {
		gtk_widget_set_visible(frInfo, FALSE);
		gtk_widget_set_visible(frRightTop, FALSE);
		gtk_widget_set_visible(hbActorsError, TRUE);
		gtk_widget_set_visible(spActors, FALSE);
		gtk_spinner_stop(GTK_SPINNER(spActors));
	}
	
	void updateActors() {
		gtk_label_set_text(GTK_LABEL(lbInfo), actors.getTitle().c_str());
		GtkTreeModel *model;
		model = actors.getModel();
	    gtk_tree_view_set_model(GTK_TREE_VIEW(tvActors), model);
		//g_object_unref(model); // do not free, used for actors history
	}

	void backActorsListAdd(string title) {
		gtk_widget_set_visible(frRightBottom, TRUE);
		
		GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(
		    GTK_TREE_VIEW(tvBackActors)));
		GtkTreeIter iter;
		
		gtk_list_store_append(store, &iter);
	    gtk_list_store_set(store, 
	                       &iter, 
	                       IMAGE_COLUMN, 
	                       icon,
	                       TITLE_COLUMN, 
	                       title.c_str(),
	                       -1);
	}
};