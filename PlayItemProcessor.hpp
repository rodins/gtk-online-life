// PlayItemProcessor.hpp

class PlayItemProcessor {
	PlayItemPlayer *player;
	ErrorDialogs *errorDialogs;
    public:
    PlayItemProcessor(PlayItemPlayer *player, ErrorDialogs *errorDialogs) {
		this->player = player;
		this->errorDialogs = errorDialogs;
	}
    
    void process(string browserUrl) { 
		if(player->isPlayerFound()) {
			player->playLink(browserUrl);
		}else {
		    errorDialogs->runBrowserErrorDialog();	
		}
    }
};
