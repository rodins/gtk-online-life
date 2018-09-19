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
			// TODO: save position
			backStack.push_back(model);
			view->setSensitivePrev();
			view->setTooltipPrev(model.getTitle());
		}
	}
	
	void saveToForwardStack(ResultsModel &model) {
		if(!model.isEmpty()) {
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
		if(!backStack.empty()) {
			view->setTooltipPrev(backStack.back().getTitle());
		}else {
		    view->setTooltipNext("Move back in history");
		    view->setSensitivePrev(FALSE);	
	    }
	    return model;
	}
	
	ResultsModel restoreFromForwardStack() {
		ResultsModel model = forwardStack.back();
		forwardStack.pop_back();
		if(!forwardStack.empty()) {
			view->setTooltipPrev(forwardStack.back().getTitle());
		}else {
		    view->setTooltipNext("Move forward in history");
		    view->setSensitiveNext(FALSE);	
	    }
	    return model;
	}
	
	
	void updatePrevNextButtons() {
		view->setSensitivePrev(!backStack.empty());
		view->setSensitiveNext(!forwardStack.empty());
	}
};
