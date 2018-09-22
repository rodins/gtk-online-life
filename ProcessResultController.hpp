// ProcessResultController.hpp

class ProcessResultController {
	ActorsTask *actorsTask;
	ConstantLinksTask *constantLinksTask;
    gboolean isActorsActive;
    public:
    ProcessResultController(GtkToolItem *btnActors,
                            ActorsTask *actorsTask,
                            ConstantLinksTask *constantLinksTask) {
		this->actorsTask = actorsTask;
		this->constantLinksTask = constantLinksTask;
		isActorsActive = gtk_toggle_tool_button_get_active(
		                       GTK_TOGGLE_TOOL_BUTTON(btnActors));
	}
	
	void onClick(string title, string href, GdkPixbuf *pixbuf) {
		if(isActorsActive) {
			actorsTask->start(title, href, pixbuf);
		}else {
			constantLinksTask->start(title, href);
		}
	}
    
};
