// PlayItemDialog.hpp

class PlayItemDialog {
	GtkWidget *window;
	PlayItemPlayer *player;
    public: 
    PlayItemDialog(GtkWidget *window, PlayItemPlayer *player) {
		this->window = window;
		this->player = player;
	}
	// TODO: remove these form interface
	virtual void actorsClicked() {}
	
	virtual void addActorsButton(GtkWidget *dialog) {}
    
    void create(PlayItem *playItem, 
	            string sizeFile, 
	            string sizeDownload) {
		
		GtkWidget *label;
		
		// Create dialog, add buttons to it
		GtkWidget *dialog,  
		          *content_area;
		
		dialog = gtk_dialog_new_with_buttons ("Links dialog",
                                  GTK_WINDOW(window),
                                  (GtkDialogFlags)(GTK_DIALOG_MODAL|
                                                   GTK_DIALOG_DESTROY_WITH_PARENT),
                                  NULL);
        string msg;
		if(!player->getPlayer().empty()) {
			msg = "Play in " + player->getPlayer() + ". Copy to clipboard";
		}else {
			msg = "Mplayer or mpv is not detected. Copy to clipboard";
		}
		
		addActorsButton(dialog);                                     
                           
        if(!sizeFile.empty()) {
			string sizeTitle = "FLV (" + sizeFile + ")";
			GtkWidget *btnFlv = gtk_dialog_add_button(GTK_DIALOG(dialog),
		                               sizeTitle.c_str(),
                                       LINK_RESPONSE_PLAY);
            gtk_widget_set_tooltip_text(btnFlv, msg.c_str());
		}
        
        if(!sizeDownload.empty()) {
			// If btnFlv is not available, change button title to "Play"
			string title;
			if(sizeFile.empty()) {
			    title = "PLAY";	
			}else {
				title = "MP4";
			}
		    string sizeTitle = title + " (" + sizeDownload + ")";
			GtkWidget *btnMp4 = gtk_dialog_add_button(GTK_DIALOG(dialog),
		                      sizeTitle.c_str(),
                              LINK_RESPONSE_DOWNLOAD);
            gtk_widget_set_tooltip_text(btnMp4, msg.c_str());
		}
        
        gtk_dialog_add_button(GTK_DIALOG(dialog),
		                      "Cancel",
                              LINK_RESPONSE_CANCEL);
                                              
        content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
        
	    label = gtk_label_new (playItem->comment.c_str());
	    //gtk_widget_set_size_request(label, 390, -1);
        gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
        //gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_CENTER);
	    
	    /* Add the label, and show everything we've added to the dialog. */
	    gtk_box_pack_start(GTK_BOX(content_area), label, TRUE, FALSE, 5);
	    gtk_widget_show_all(content_area);
		                   
		int response_id = gtk_dialog_run(GTK_DIALOG(dialog));
		
		// For pasting with "paste" or ctrl-v
        GtkClipboard* clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
        // For pasting with middle mouse button (in urxvt)
        GtkClipboard* clipboardX = gtk_clipboard_get(GDK_SELECTION_PRIMARY);
        switch(response_id) {
			case LINK_RESPONSE_PLAY:
			    gtk_clipboard_set_text(clipboard,
			                           playItem->file.c_str(),
			                           playItem->file.size());
			    gtk_clipboard_set_text(clipboardX,
			                           playItem->file.c_str(),
			                           playItem->file.size());
			                           
			    player->playLink(playItem->file);
			break;
			case LINK_RESPONSE_DOWNLOAD:
			    gtk_clipboard_set_text(clipboard,
			                           playItem->download.c_str(),
			                           playItem->download.size());
			    gtk_clipboard_set_text(clipboardX,
			                           playItem->download.c_str(),
			                           playItem->download.size());
			    player->playLink(playItem->download);
			break;
			case LINK_RESPONSE_INFO:
			    actorsClicked();
			break;
			case LINK_RESPONSE_CANCEL:
			    // Do nothing.
			break;
		}
		gtk_widget_destroy(dialog);
	}
};
