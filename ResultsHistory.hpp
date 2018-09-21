// ResultsHistory.hpp

class ResultsHistory {
    vector<ResultsModel> backStack, forwardStack;
    CenterView *view;
    public:
    ResultsHistory(CenterView *view) {
		this->view = view;
	}
	
	void saveToBackStack(ResultsModel &model) {
		if(!model.isEmpty()) {
			model.setPosition(view->getPosition());
			backStack.push_back(model);
			view->setSensitivePrev();
			view->setTooltipPrev(model.getTitle());
		}
	}
	
	void saveToForwardStack(ResultsModel &model) {
		if(!model.isEmpty()) {
			model.setPosition(view->getPosition());
			forwardStack.push_back(model);
			view->setSensitiveNext();
			view->setTooltipNext(model.getTitle());
		}
	}
	
	void clearForwardStack() {
		forwardStack.clear();
		view->setTooltipNext("Move forward in history");	
	}
	
	ResultsModel restoreFromBackStack() {
		ResultsModel model = backStack.back();
		backStack.pop_back();
		setPrevTooltip();
	    return model;
	}
	
	ResultsModel restoreFromForwardStack() {
		ResultsModel model = forwardStack.back();
		forwardStack.pop_back();
		if(!forwardStack.empty()) {
			view->setTooltipNext(forwardStack.back().getTitle());
		}else {
		    view->setTooltipNext("Move forward in history");
		    view->setSensitiveNext(FALSE);	
	    }
	    return model;
	}
	
	void removeBackStackDuplicate(string title) {
		// Linear search for title
		int eraseIndex = -1;
		// If back stack has title, remove results with it.
		for(unsigned i = 0; i < backStack.size(); i++) {
			if(backStack[i].getTitle() == title) {
				eraseIndex = i;
				break;
			}
		}
		
		if(eraseIndex != -1) {
			backStack.erase(backStack.begin() + eraseIndex);
			setPrevTooltip();
		}
	}
	
	void updatePrevNextButtons() {
		view->setSensitivePrev(!backStack.empty());
		view->setSensitiveNext(!forwardStack.empty());
	}
	
	private:
	
	void setPrevTooltip() {
		if(!backStack.empty()) {
			view->setTooltipPrev(backStack.back().getTitle());
		}else {
		    view->setTooltipPrev("Move back in history");
		    view->setSensitivePrev(FALSE);	
	    }
	}
};
