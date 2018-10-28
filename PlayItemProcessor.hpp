// PlayItemProcessor.hpp

class PlayItemProcessor {
	PlayItemPlayer *player;
    public:
    PlayItemProcessor(PlayItemPlayer *player) {
		this->player = player;
	}
    
    void process(string browserUrl) { 
        player->playLink(browserUrl);
    }
};
