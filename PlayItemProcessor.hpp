// PlayItemProcessor.hpp

class PlayItemProcessor {
    LinksSizeTask *sizeTask;
    public:
    PlayItemProcessor(LinksSizeTask *sizeTask) {
		this->sizeTask = sizeTask;
	}
    
    void process(string js) {
        static PlayItem playItem = PlaylistsUtils::parse_play_item(js, FALSE);
        process(&playItem);
    }
    
    void process(PlayItem *playItem) {
		sizeTask->start(playItem);
	}
};
