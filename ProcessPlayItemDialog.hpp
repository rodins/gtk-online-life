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

class ProcessPlayItemDialog {
	
	GtkWidget* window;
	PlayItem playItem;
	GThreadPool *linksSizeThreadPool;
	
	GtkWidget *btnFlv,
	          *btnMp4,
	          *label;
	
	public:
	
	ProcessPlayItemDialog(GtkWidget* window, PlayItem playItem) {
		this->window = window;
		this->playItem = playItem;
		// GThreadPool for links sizes
	    linksSizeThreadPool = g_thread_pool_new(ProcessPlayItemDialog::linksSizeTask,
	                                   NULL,
	                                   2, // Run two threads at the time
	                                   FALSE,
	                                   NULL);
	                                   
	    createDialog();
	}
	
	private:
	
	static string detectPlayer() {
		if(system("which mpv") == 0) {
			return "mpv ";
		}
		if(system("which mplayer") == 0) {
			return "mplayer -cache 2048 ";
		}
		return "";
	}
	
	void createDialog() {
		// Create dialog, add buttons to it, get size information and display it
		GtkWidget *dialog,  
		          *content_area;
		
		dialog = gtk_dialog_new_with_buttons ("Play and copy links",
                                              GTK_WINDOW(window),
                                              (GtkDialogFlags)(GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT),
                                              NULL);
                                              
        btnFlv = gtk_dialog_add_button(GTK_DIALOG(dialog),
		                      "FLV",
                              LINK_RESPONSE_PLAY);                   
        gtk_widget_set_sensitive(btnFlv, FALSE);
        
        btnMp4 = gtk_dialog_add_button(GTK_DIALOG(dialog),
		                      "MP4",
                              LINK_RESPONSE_DOWNLOAD);
        gtk_widget_set_sensitive(btnMp4, FALSE);
        
        gtk_dialog_add_button(GTK_DIALOG(dialog),
		                      "Cancel",
                              LINK_RESPONSE_CANCEL);
                                              
        /* Ensure that the dialog box is destroyed when the user responds. */
		g_signal_connect(dialog,
						 "response",
					     G_CALLBACK (ProcessPlayItemDialog::dialogResponse),
					     this);
                                              
        content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
        
	    label = gtk_label_new (playItem.comment.c_str());
	    
	    /* Add the label, and show everything we've added to the dialog. */
	    gtk_box_pack_start(GTK_BOX(content_area), label, TRUE, FALSE, 5);
	    gtk_widget_show_all(content_area);
        
        playItem.player = detectPlayer();
		string msg;
		if(!playItem.player.empty()) {
			msg = "Play in " + playItem.player + ". Copy to clipboard";
		}else {
			msg = "Mplayer or mpv not detected. Copy to clipboard";
		}
		gtk_widget_set_tooltip_text(btnFlv, msg.c_str());
		gtk_widget_set_tooltip_text(btnMp4, msg.c_str());
		
		LinkSizeTaskArgs *args = new LinkSizeTaskArgs();
		args->linkFile = playItem.file;
		args->linkDownload = playItem.download;
		args->btnFlv = btnFlv;
		args->btnMp4 = btnMp4;
		args->dialog = dialog;
		
		g_thread_pool_push(linksSizeThreadPool, 
		                   (gpointer)args, 
		                   NULL);
	}
	
	void playLink(string link) {
		string command = playItem.player + link + " &";
		system(command.c_str());
	}
	
	static void dialogResponse(GtkWidget *dialog,
	                           gint response_id, 
	                           gpointer user_data) {
		ProcessPlayItemDialog *ppid = (ProcessPlayItemDialog*)user_data;
		gtk_widget_destroy(dialog);
		
		// For pasting with "paste" or ctrl-v
        GtkClipboard* clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
        // For pasting with middle mouse button (in urxvt)
        GtkClipboard* clipboardX = gtk_clipboard_get(GDK_SELECTION_PRIMARY);
        switch(response_id) {
			case LINK_RESPONSE_PLAY:
			    gtk_clipboard_set_text(clipboard,
			                           ppid->playItem.file.c_str(),
			                           ppid->playItem.file.size());
			    gtk_clipboard_set_text(clipboardX,
			                           ppid->playItem.file.c_str(),
			                           ppid->playItem.file.size());
			                           
			    ppid->playLink(ppid->playItem.file);
			break;
			case LINK_RESPONSE_DOWNLOAD:
			    gtk_clipboard_set_text(clipboard,
			                           ppid->playItem.download.c_str(),
			                           ppid->playItem.download.size());
			    gtk_clipboard_set_text(clipboardX,
			                           ppid->playItem.download.c_str(),
			                           ppid->playItem.download.size());
			    ppid->playLink(ppid->playItem.download);
			break;
			case LINK_RESPONSE_CANCEL:
			    // Do nothing. Dialog should already be destroyed.
			break;
		}
	}
	
	static void linksSizeTask(gpointer args1, gpointer args2) {
		LinkSizeTaskArgs *args = (LinkSizeTaskArgs*)args1;
		string sizeFile = HtmlString::getSizeOfLink(args->linkFile);
		string sizeDownload = HtmlString::getSizeOfLink(args->linkDownload);
		gdk_threads_enter();
		
		if(!sizeFile.empty()) {
			string buttonText = string(gtk_button_get_label(GTK_BUTTON(args->btnFlv))); 
			string sizeTitle = buttonText + " (" + sizeFile + " Mb)";
			gtk_button_set_label(GTK_BUTTON(args->btnFlv), sizeTitle.c_str());
			gtk_widget_set_sensitive(args->btnFlv, TRUE);
		}
		
		if(!sizeDownload.empty()) {
			string buttonText = string(gtk_button_get_label(GTK_BUTTON(args->btnMp4))); 
			string sizeTitle = buttonText + " (" + sizeDownload + " Mb)";
			gtk_button_set_label(GTK_BUTTON(args->btnMp4), sizeTitle.c_str());
			gtk_widget_set_sensitive(args->btnMp4, TRUE);
		}
		gtk_dialog_run(GTK_DIALOG(args->dialog));
		gdk_threads_leave();
	    g_free(args);
	}
};
