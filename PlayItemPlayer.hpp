// PlayItemPlayer.hpp

class PlayItemPlayer {
    string command;
    public:
    PlayItemPlayer() {
		// TODO: add more browsers and error dialog if browser is not detected
		if(system("which chromium-browser") == 0) { // Raspberry Pi option
			command = "chromium-browser ";
		}else if(system("which firefox") == 0) {
			command = "firefox ";
		}
	}
	
	bool isPlayerFound() {
		return !command.empty();
	}
	
	void playLink(string link) {
		string finalCommand = command + link + " &";
		system(finalCommand.c_str());
	}
};
