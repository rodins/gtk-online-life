// PlayItemPlayer.hpp

class PlayItemPlayer {
    string command;
    public:
    PlayItemPlayer() {
		detectBrowser();
	}
	
	bool isPlayerFound() {
		return !command.empty();
	}
	
	void playLink(string link) {
		string finalCommand = command + link + " &";
		system(finalCommand.c_str());
	}
	
	private:
	
	void detectBrowser() {
		const unsigned SIZE = 3;
		const string BROWSERS[] = { "chromium-browser", "firefox", "seamonkey" };
		for(unsigned i = 0; i < SIZE; i++) {
			string whichCommand = string("which ") + BROWSERS[i];
			if(system(whichCommand.c_str()) == 0) {
				command = BROWSERS[i] + " ";
				break;
			}
		} 
	}
};
