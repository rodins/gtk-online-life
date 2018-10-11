// PlayItemPlayer.hpp

class PlayItemPlayer {
    string player;
    string command;
    public:
    PlayItemPlayer() {
		if(system("which omxplayer") == 0) { // Raspberry Pi option
			player = "omxplayer";
			command = "lxterminal -e " + player + " -b ";
		}else if(system("which mpv") == 0) {
			player = "mpv";
			command = player + " --cache=2048 ";
		}else if(system("which mplayer") == 0) {
			player = "mplayer";
			command = player + " -cache 2048 ";
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
