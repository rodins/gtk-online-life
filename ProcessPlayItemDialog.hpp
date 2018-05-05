// ProcessPlayItemDialog.hpp

enum LinkResponse {
	LINK_RESPONSE_PLAY,
	LINK_RESPONSE_DOWNLOAD,
	LINK_RESPONSE_CANCEL
};

struct LinkSizeTaskArgs {
	string linkFile, linkDownload;
	GtkWidget *btnFlv;
	GtkWidget *btnMp4;
	GtkWidget *dialog;
};

struct DialogResponseArgs {
	string linkFile, linkDownload, player;
};

class ProcessPlayItemDialog {
	GtkWidget* window;
	PlayItem playItem;
	GThreadPool *linksSizeThreadPool;
	
	GtkWidget *btnFlv,
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
                                              
        btnFlv = gtk_dialog_add_button(GTK_DIALOG(dialog),
		                               "FLV",
                                       LINK_RESPONSE_PLAY);                   
        gtk_widget_set_sensitive(btnFlv, FALSE);
        if(!sizeFile.empty()) {
			string buttonText = string(
			gtk_button_get_label(GTK_BUTTON(btnFlv))); 
			string sizeTitle = buttonText + " (" + sizeFile + ")";
			gtk_button_set_label(GTK_BUTTON(btnFlv), sizeTitle.c_str());
			gtk_widget_set_sensitive(btnFlv, TRUE);
		}
        
        btnMp4 = gtk_dialog_add_button(GTK_DIALOG(dialog),
		                      "MP4",
                              LINK_RESPONSE_DOWNLOAD);
        gtk_widget_set_sensitive(btnMp4, FALSE);
        if(!sizeDownload.empty()) {
			string buttonText = string(
			gtk_button_get_label(GTK_BUTTON(btnMp4))); 
			string sizeTitle = buttonText + " (" + sizeDownload + ")";
			gtk_button_set_label(GTK_BUTTON(btnMp4), sizeTitle.c_str());
			gtk_widget_set_sensitive(btnMp4, TRUE);
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
        
		string msg;
		if(!playItem.player.empty()) {
			msg = "Play in " + playItem.player + ". Copy to clipboard";
		}else {
			msg = "Mplayer or mpv not detected. Copy to clipboard";
		}
		gtk_widget_set_tooltip_text(btnFlv, msg.c_str());
		gtk_widget_set_tooltip_text(btnMp4, msg.c_str());
		
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
	
	static void linksSizeTask(gpointer args1, gpointer args2) {
		LinkSizeTaskArgs *args = (LinkSizeTaskArgs*)args1;
		string sizeFile = HtmlString::getSizeOfLink(args->linkFile);
		string sizeDownload = HtmlString::getSizeOfLink(args->linkDownload);
		gdk_threads_enter();
		
		if(!sizeFile.empty()) {
			string buttonText = string(gtk_button_get_label(GTK_BUTTON(args->btnFlv))); 
			string sizeTitle = buttonText + " (" + sizeFile + ")";
			gtk_button_set_label(GTK_BUTTON(args->btnFlv), sizeTitle.c_str());
			gtk_widget_set_sensitive(args->btnFlv, TRUE);
		}
		
		if(!sizeDownload.empty()) {
			string buttonText = string(gtk_button_get_label(GTK_BUTTON(args->btnMp4))); 
			string sizeTitle = buttonText + " (" + sizeDownload + ")";
			gtk_button_set_label(GTK_BUTTON(args->btnMp4), sizeTitle.c_str());
			gtk_widget_set_sensitive(args->btnMp4, TRUE);
		}
		
		gtk_dialog_run(GTK_DIALOG(args->dialog));
		gdk_threads_leave();
	    g_free(args);
	}
};
