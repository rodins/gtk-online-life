//ActorsHistory.hpp
#include "Actors.hpp"

class ActorsHistory {
    Actors actors, prevActors;
    map <string, Actors> backActors;
    
    GtkWidget *tvActors, *tvBackActors;
    GtkWidget *frRightBottom;
    GdkPixbuf *icon;
    GtkWidget *lbInfo;
    public:
    
    ActorsHistory(GtkWidget *a, GtkWidget *pa, GtkWidget* fr, GtkWidget* li) {
		tvActors = a;
		tvBackActors = pa;
		frRightBottom = fr;
		lbInfo = li;
		icon = create_pixbuf("link_16.png");
	} 
    
    void saveActors(string title, string href) {
		if(!actors.getTitle().empty()) {
			prevActors = actors;
		}
		actors.setTitle(title);
		actors.setUrl(href);
	}
    
    void newActors(string &page) {
		actors.parse(page);
		// Save to back actors map
		if(backActors.count(prevActors.getTitle()) == 0 
		        && prevActors.getCount() > 0
		        && prevActors.getTitle() != actors.getTitle()) {
		    backActors[prevActors.getTitle()] = prevActors;
		    backActorsListAdd(prevActors.getTitle());	
		}
		updateActors();
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
	
	string getUrl() {
		return actors.getUrl();
	}
	
	int getCount() {
		return actors.getCount();
	}
	
	private:
	
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