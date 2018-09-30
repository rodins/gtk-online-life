// ActorsHistoryModel.hpp

class ActorsHistoryModel {
	GtkWidget *frRightBottom;
	map <string, ActorsModel> backActors;
    GtkListStore *store;
    GdkPixbuf *icon;
    public:
    ActorsHistoryModel(GtkWidget *frRightBottom, GtkListStore *store) {
		this->frRightBottom = frRightBottom;
		this->store = store;
		this->icon = IconsFactory::getLinkIcon();
	}
	
	void saveActorsModel(ActorsModel model) {
		if(!model.isEmpty()) {
			if(backActors.count(model.getTitle()) == 0) {
			    addToStore(model.getTitle());	
			    backActors[model.getTitle()] = model;
			    gtk_widget_show(frRightBottom);
			}
		}
	}
	
	ActorsModel getActorsModel(string key) {
		return backActors[key];
	}
	
	private:
	
	void addToStore(string title) {
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
