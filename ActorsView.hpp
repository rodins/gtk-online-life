// ActorsView.hpp

class ActorsView {
	GtkWidget *vbRight;
	GtkToolItem *btnActors;
    GtkWidget *spActors;
    GtkWidget *lbInfo;
    GtkWidget *tvActors;
    GtkWidget *hbActorsError;
    GtkWidget *frActors;
    ActorsModel *model;
    public:
    ActorsView(GtkWidget *vbRight,
               GtkToolItem *btnActors,
               GtkWidget *spActors, 
               GtkWidget *frActors,
               GtkWidget *hbActorsError,
               GtkWidget *lbInfo, 
               GtkWidget *tvActors) {
	    this->vbRight = vbRight;
	    this->btnActors = btnActors;			   
	    this->spActors = spActors;
	    this->frActors = frActors;
	    this->hbActorsError = hbActorsError;
	    this->lbInfo = lbInfo;
	    this->tvActors = tvActors;		   
	}
	
	void setModel(ActorsModel *model) {
		this->model = model;
		gtk_tree_view_set_model(GTK_TREE_VIEW(tvActors),
		                                 model->getTreeModel());
	}
	
	gboolean isBtnActorsActive() {
		return gtk_toggle_tool_button_get_active(
		                       GTK_TOGGLE_TOOL_BUTTON(btnActors));
	}
	
	void onActorsClick(gboolean isEmpty) {
		gtk_widget_set_visible(vbRight, isBtnActorsActive() && !isEmpty);
	}
	
	void showLoadingIndicator() {
		setModelInfo();
		gtk_widget_show(vbRight);
		gtk_widget_show(spActors);
		gtk_widget_hide(frActors);
		gtk_widget_hide(hbActorsError);
		gtk_spinner_start(GTK_SPINNER(spActors));
	}
	
	void showData() {
		setModelInfo();
		gtk_widget_hide(spActors);
		gtk_widget_show(frActors);
		gtk_widget_hide(hbActorsError);
		gtk_spinner_stop(GTK_SPINNER(spActors));
	}
	
	void showError() {
		gtk_widget_hide(spActors);
		gtk_widget_hide(frActors);
		gtk_widget_show(hbActorsError);
		gtk_spinner_stop(GTK_SPINNER(spActors));
	}
	
	private:
	
	void setModelInfo() {
		gtk_label_set_text(GTK_LABEL(lbInfo), model->getInfo().c_str());
	}
};
