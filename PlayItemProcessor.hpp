// PlayItemProcessor.hpp

class PlayItemProcessor {
    LinksSizeTask *sizeTask;
    PlayItem playItem;
    public:
    PlayItemProcessor(LinksSizeTask *sizeTask) {
		this->sizeTask = sizeTask;
	}
    
    void process(string title, string js) { // For dynamic links
        playItem = PlaylistsUtils::parse_play_item(js, FALSE);
        if(playItem.comment.size() < 2) { // Set title for trailers
			playItem.comment = title;
		}
        process(&playItem);
    }
    
    void process(string js) { // For constant links
        playItem = PlaylistsUtils::parse_play_item(js, FALSE);
        process(&playItem);
    }
    
    void process(PlayItem *pi) {
		sizeTask->start(pi);
	}
};
