// IconsFactory.hpp

class IconsFactory {
	
	static GdkPixbuf* create_pixbuf(const gchar * filename) {
    
	    GdkPixbuf *pixbuf;
	    GError *error = NULL;
	    pixbuf = gdk_pixbuf_new_from_file(filename, &error);
	    
	    if (!pixbuf) {
	        
	        fprintf(stderr, "%s\n", error->message);
	        g_error_free(error);
	        
	    }
	    
	    return pixbuf;
	}
	
	public:
	
    static GdkPixbuf* getLinkIcon() {
		return create_pixbuf("images/link_16.png");
	}
	
	static GdkPixbuf* getFolderIcon() {
		return create_pixbuf("images/folder_16.png");
	}
	
	static GdkPixbuf* getAppIcon() {
		return create_pixbuf("images/online_life.png");
	}
	
	static GdkPixbuf* getBlankIcon() {
		return create_pixbuf("images/blank.png");
	}
	
	static GdkPixbuf* getFloppyIcon() {
		return create_pixbuf("images/floppy_16.png");
	}
	
	static GdkPixbuf* getBookmarkIcon() {
		return create_pixbuf("images/bookmark_16.png");
	}
	
	static GdkPixbuf* getBookmarkIcon24() {
		return create_pixbuf("images/bookmark_24.png");
	}
};
