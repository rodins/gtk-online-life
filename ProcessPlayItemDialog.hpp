// ProcessPlayItemDialog.hpp

enum LinkResponse {
	LINK_RESPONSE_PLAY,
	LINK_RESPONSE_DOWNLOAD,
	LINK_RESPONSE_INFO,
	LINK_RESPONSE_CANCEL
};

struct DialogResponseArgs {
	string linkFile, linkDownload, player;
};

class ProcessPlayItemDialog {
	const string FLV = "FLV", MP4 = "MP4", PLAY = "Play", INFO = "Info";
	GtkWidget* window;
	PlayItem playItem;
	GThreadPool *linksSizeThreadPool;
	
	GtkWidget *btnFlv	,
	          *btnMp4,
	          *label;
	
	public:
	
	ProcessPlayItemDialog(GtkWidget* window,
	                      PlayItem playItem,
	                      string sizeFile, 
	                      string sizeDownload) {
		this->window = window;
		this->playItem = playItem;            
	    createDialog(sizeFile, sizeDownload);
	}
	
	private:
	
	void createDialog(string sizeFile, string sizeDownload) {
		// Create dialog, add buttons to it
		GtkWidget *dialog,  
		          *content_area;
		
		dialog = gtk_dialog_new_with_buttons ("Links dialog",
                                              GTK_WINDOW(window),
                                              (GtkDialogFlags)(GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT),
                                              NULL);
        string msg;
		if(!playItem.player.empty()) {
			msg = "Play in " + playItem.player + ". Copy to clipboard";
		}else {
			msg = "Mplayer or mpv not detected. Copy to clipboard";
		}
		
		                                      
                           
        if(!sizeFile.empty()) {
			string sizeTitle = FLV + " (" + sizeFile + ")";
			btnFlv = gtk_dialog_add_button(GTK_DIALOG(dialog),
		                               sizeTitle.c_str(),
                                       LINK_RESPONSE_PLAY);
            gtk_widget_set_tooltip_text(btnFlv, msg.c_str());
		}
        
        if(!sizeDownload.empty()) {
			// If btnFlv is not available, change button title to "Play"
			string title;
			if(sizeFile.empty()) {
			    title = PLAY;	
			}else {
				title = MP4;
			}
		    string sizeTitle = title + " (" + sizeDownload + ")";
			btnMp4 = gtk_dialog_add_button(GTK_DIALOG(dialog),
		                      sizeTitle.c_str(),
                              LINK_RESPONSE_DOWNLOAD);
            gtk_widget_set_tooltip_text(btnMp4, msg.c_str());
		}
        
        gtk_dialog_add_button(GTK_DIALOG(dialog),
		                      "Cancel",
                              LINK_RESPONSE_CANCEL);
                                              
        content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
        
	    label = gtk_label_new (playItem.comment.c_str());
	    //gtk_widget_set_size_request(label, 390, -1);
        gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
        //gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_CENTER);
	    
	    /* Add the label, and show everything we've added to the dialog. */
	    gtk_box_pack_start(GTK_BOX(content_area), label, TRUE, FALSE, 5);
	    gtk_widget_show_all(content_area);
		
		DialogResponseArgs *rargs = new DialogResponseArgs();
        rargs->linkFile = playItem.file;
        rargs->linkDownload = playItem.download;
        rargs->player = playItem.player;
                                              
        /* Ensure that the dialog box is destroyed when the user responds. */
		g_signal_connect(dialog,
						 "response",
					     G_CALLBACK (ProcessPlayItemDialog::dialogResponse),
					     rargs);
		                   
		gtk_dialog_run(GTK_DIALOG(dialog));
	}
	
	static void playLink(string player, string link) {
		string command = player + link + " &";
		system(command.c_str());
	}
	
	static void dialogResponse(GtkWidget *dialog,
	                           gint response_id, 
	                           gpointer user_data) {
		DialogResponseArgs *args = (DialogResponseArgs*)user_data;
		gtk_widget_destroy(dialog);
		
		// For pasting with "paste" or ctrl-v
        GtkClipboard* clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
        // For pasting with middle mouse button (in urxvt)
        GtkClipboard* clipboardX = gtk_clipboard_get(GDK_SELECTION_PRIMARY);
        switch(response_id) {
			case LINK_RESPONSE_PLAY:
			    gtk_clipboard_set_text(clipboard,
			                           args->linkFile.c_str(),
			                           args->linkFile.size());
			    gtk_clipboard_set_text(clipboardX,
			                           args->linkFile.c_str(),
			                           args->linkFile.size());
			                           
			    playLink(args->player, args->linkFile);
			break;
			case LINK_RESPONSE_DOWNLOAD:
			    gtk_clipboard_set_text(clipboard,
			                           args->linkDownload.c_str(),
			                           args->linkDownload.size());
			    gtk_clipboard_set_text(clipboardX,
			                           args->linkDownload.c_str(),
			                           args->linkDownload.size());
			    playLink(args->player, args->linkDownload);
			break;
			case LINK_RESPONSE_CANCEL:
			    // Do nothing. Dialog should already be destroyed.
			break;
		}
		g_free(args);
	}
	
};
