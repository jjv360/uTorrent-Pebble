#include <pebble.h>
#include "TorrentList.h"
#include "Resources.h"
	

int main(void) {
	
	// Load resources
	resources_load();
	  
	// Setup and show torrent list
	TorrentList_init();
	TorrentList_show();
	
	// Do app event loop
	app_event_loop();
	
	// Destroy torrent list
	TorrentList_deinit();
	
}
