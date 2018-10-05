// PlayItemActorsDialog.hpp

class PlayItemActorsDialog : public PlayItemDialog{
    ActorsController *controller;
    public:
    PlayItemActorsDialog(GtkWidget *window, 
                         PlayItemPlayer *player,
                         ActorsController *controller):PlayItemDialog(window, player) {
	    this->controller = controller;					
	}
	
	void actorsClicked() {
		controller->dialogClick();
	}
	
	void addActorsButton(GtkWidget *dialog) {
		gtk_dialog_add_button(GTK_DIALOG(dialog),
		                      "Actors",
                              LINK_RESPONSE_INFO);
	}
};
