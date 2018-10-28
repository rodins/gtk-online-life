// PlayItemPlayer.hpp

class PlayItemPlayer {
    string command;
    public:
    PlayItemPlayer() {
		// TODO: add more browsers and error dialog if browser is not detected
		if(system("which chromium-browser") == 0) { // Raspberry Pi option
			command = "chromium-browser ";
		}
	}
	
	void playLink(string link) {
		string finalCommand = command + link + " &";
		system(finalCommand.c_str());
	}
};
