// PlayItemPlayer.hpp

class PlayItemPlayer {
    string player;
    string command;
    public:
    PlayItemPlayer() {
		if(system("which mpv") == 0) {
			player = "mpv";
			command = player + " --cache=2048 ";
		}
		if(system("which mplayer") == 0) {
			player = "mplayer";
			command = player + " -cache 2048 ";
		}
		if(system("which omxplayer") == 0) { // Raspberry Pi option
			player = "omxplayer";
			command = "lxterminal -e " + player + " -b ";
		}
	}
	
	string getPlayer() {
		return player;
	}
	
	void playLink(string link) {
		string finalCommand = command + link + " &";
		system(finalCommand.c_str());
	}
};
