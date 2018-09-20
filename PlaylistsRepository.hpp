// PlaylistsRepository.hpp

class PlaylistsRepository {
    CenterView *view;
    public:
    PlaylistsRepository(CenterView *view) {
		this->view = view;
	}
    
    void getData(string js) {
		string playlist_link = PlaylistsUtils::get_txt_link(js);
		if(!playlist_link.empty()) { // Playlists found
			cout << "Playlists: Not yet implemented" << endl;
		}
	}
};
