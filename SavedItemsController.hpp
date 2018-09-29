// SavedItemsController

class SavedItemsController {
    SavedItemsView *view;
    ActorsModel *actorsModel;
    SavedItemsModel *savedItemsModel;
    public:
    SavedItemsController(SavedItemsView *view, 
                         SavedItemsModel *savedItemsModel) {
		this->view = view;
		this->savedItemsModel = savedItemsModel;
		this->actorsModel = NULL;
	}
	
	void setModel(ActorsModel *model) {
		actorsModel = model;
	}
	
	void showSaveOrDelete() {
		if(FileUtils::isTitleSaved(actorsModel->getTitle())) {
			view->showDelete();
		}else {
			view->showSave();
		}	
	}
	
	void btnSaveClicked() {
		if(actorsModel != NULL) {
			FileUtils::writeToFile(actorsModel->getTitle(), 
			                       actorsModel->getUrl());
			FileUtils::writeImageToFile(actorsModel->getTitle(),
			                            actorsModel->getPixbuf());
			FileUtils::listSavedFiles(*savedItemsModel);
			view->showDelete();
		}
	}
	
	void btnDeleteClicked() {
		if(actorsModel != NULL) {
			FileUtils::removeFile(actorsModel->getTitle());
			FileUtils::removeImageFile(actorsModel->getTitle());
			FileUtils::listSavedFiles(*savedItemsModel);
			view->showSave();
		}
	}
};
