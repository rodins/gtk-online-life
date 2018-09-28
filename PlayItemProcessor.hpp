// PlayItemProcessor.hpp

class PlayItemProcessor {
    LinksSizeTask *sizeTask;
    PlayItem playItem;
    public:
    PlayItemProcessor(LinksSizeTask *sizeTask) {
		this->sizeTask = sizeTask;
	}
    
    void process(string js) {
        playItem = PlaylistsUtils::parse_play_item(js, FALSE);
        process(&playItem);
    }
    
    void process(PlayItem *pi) {
		sizeTask->start(pi);
	}
};
