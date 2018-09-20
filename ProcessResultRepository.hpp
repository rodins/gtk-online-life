// ProcessResultRepository.hpp

class ProcessResultRepository {
	ActorsRepository *actorsRepository;
	ConstantLinksRepository *constantLinksRepository;
    gboolean isActorsActive;
    public:
    ProcessResultRepository(GtkToolItem *btnActors,
                            ActorsRepository *actorsRepository,
                            ConstantLinksRepository *constantLinksRepository) {
		this->actorsRepository = actorsRepository;
		this->constantLinksRepository = constantLinksRepository;
		isActorsActive = gtk_toggle_tool_button_get_active(
		                       GTK_TOGGLE_TOOL_BUTTON(btnActors));
	}
	
	void getData(string title, string href, GdkPixbuf *pixbuf) {
		if(isActorsActive) {
			actorsRepository->getData(title, href, pixbuf);
		}else {
			constantLinksRepository->getData(href);
		}
	}
    
};
