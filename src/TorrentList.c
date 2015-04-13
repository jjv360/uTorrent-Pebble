#include "TorrentList.h"
#include "TorrentInfo.h"
#include "Resources.h"
	
/****************************************************** VARS *******************************************************/	
struct TorrentListClass {
	
	// Views
	Window* window;
	BitmapLayer* bigIcon;
	TextLayer* statusText;
	TextLayer* emptyText;
	WindowHandlers handlers;
	MenuLayer* list;
	MenuLayerCallbacks listCallbacks;
	
	
	// Vars
	int numTorrents;
	TorrentInfo torrents[32];
	AppTimer* updateTimer;
	TorrentInfo* visibleTorrent;
	
} TorrentList;













/******************************************************** Torrent Functions ************************************************/
TorrentInfo* TorrentList_getTorrent(char* hash) {
	
	// Find torrent
	for (int i = 0 ; i < TorrentList.numTorrents ; i++) {
		
		// Compare hash
		if (strcmp(TorrentList.torrents[i].hash, hash) == 0)
			return &(TorrentList.torrents[i]);
		
	}
	
	// Check number of torrents
	if (TorrentList.numTorrents >= 32)
		return NULL;
	
	// Doesn't exist, create a blank one
	int i = TorrentList.numTorrents;
	TorrentList.numTorrents++;
	TorrentList.torrents[i].hash[0] = 0;
	TorrentList.torrents[i].name[0] = 0;
	TorrentList.torrents[i].active = false;
	TorrentList.torrents[i].size = 0;
	TorrentList.torrents[i].downloaded = 0;
	TorrentList.torrents[i].uploaded = 0;
	TorrentList.torrents[i].downloadSpeed = 0;
	TorrentList.torrents[i].uploadSpeed = 0;
	
	// Set hash
	strcpy(TorrentList.torrents[i].hash, hash);
	return &(TorrentList.torrents[i]);
	
}













/******************************************************** List Functions ************************************************/
int16_t TorrentList_getCellHeight(struct MenuLayer* menu_layer, MenuIndex* cell_index, void* callback_context) {
	return 44;
}

int16_t TorrentList_getNumRows(struct MenuLayer* menu_layer, uint16_t section_index, void* callback_context) {
	return TorrentList.numTorrents;
}

void TorrentList_drawRow(GContext* ctx, const Layer* cell_layer, MenuIndex* cell_index, void* callback_context) {
	
	// Check size of list
	if (cell_index->row >= TorrentList.numTorrents)
		return;
	
	// Get torrent
	TorrentInfo* torrent = &(TorrentList.torrents[cell_index->row]);
	
	// Draw cell
	if (torrent->size <= 0 && torrent->active) {
		
		// Getting metadata
		menu_cell_basic_draw(ctx, cell_layer, torrent->name, "Fetching info", Resources.downloadingIcon);
		return;
		
	} else if (torrent->size > 0 && torrent->downloaded >= torrent->size && torrent->active) {
		
		// Seeding
		menu_cell_basic_draw(ctx, cell_layer, torrent->name, "Seeding", Resources.completeIcon);
		return;
		
	} else if (torrent->downloaded >= torrent->size) {
		
		// Stopped and finished
		menu_cell_basic_draw(ctx, cell_layer, torrent->name, "Complete", Resources.completeIcon);
		return;
		
	} else if (torrent->active) {
		
		// Downloading
		menu_cell_basic_draw(ctx, cell_layer, torrent->name, "", Resources.downloadingIcon);
		
	} else {
		
		// Stopped
		menu_cell_basic_draw(ctx, cell_layer, torrent->name, "", Resources.stoppedIcon);
		
	}
	
	// Set progress bar border color
	#ifdef PBL_COLOR
		graphics_context_set_stroke_color(ctx, GColorDarkGray);
	#endif

	// Draw progress bar border
	int width = 144;
	int height = 44;
	int barX = 32+4;
	int barY = height-8-7;
	int barWidth = width-barX-5;
	int barHeight = 8;
	graphics_draw_line(ctx, GPoint(barX+1, barY), GPoint(barX + barWidth - 1, barY));
	graphics_draw_line(ctx, GPoint(barX+1, barY + barHeight-1), GPoint(barX + barWidth - 1, barY + barHeight-1));
	graphics_draw_line(ctx, GPoint(barX, barY+1), GPoint(barX, barY + barHeight - 2));
	graphics_draw_line(ctx, GPoint(barX + barWidth, barY+1), GPoint(barX + barWidth, barY + barHeight - 2));

	// Get progress size
	double percent = (double) torrent->downloaded / torrent->size;
	if (percent <= 0) return;
	int innerBarWidth = percent * (barWidth-2);

	// Check if using color
	#ifdef PBL_COLOR

		// Get colors
		GColor shade1 = GColorIslamicGreen;
		GColor shade2 = GColorDarkGreen;

		// Draw inner progress bar
		graphics_context_set_fill_color(ctx, shade1);
		graphics_fill_rect(ctx, GRect(barX + 1, barY + 1, innerBarWidth, (barHeight-2)/2), 0, GCornerNone);
		graphics_context_set_fill_color(ctx, shade2);
		graphics_fill_rect(ctx, GRect(barX + 1, barY + 1 +  (barHeight-2)/2, innerBarWidth,  (barHeight-2)/2), 0, GCornerNone);

	#else

		// Draw inner progress bar
		graphics_fill_rect(ctx, GRect(barX + 1, barY + 0, innerBarWidth, barHeight), 0, GCornerNone);

	#endif
	
}

void TorrentList_itemSelected(struct MenuLayer* menu_layer, MenuIndex* cell_index, void* callback_context) {
	
	// Check size of list
	if (cell_index->row >= TorrentList.numTorrents)
		return;
	
	// Get torrent
	TorrentList.visibleTorrent = &(TorrentList.torrents[cell_index->row]);
	
	// Show torrent info window
	TorrentInfo_show(TorrentList.visibleTorrent);
	
}












/******************************************************** Timer ************************************************/
void TorrentList_timerTick(void* data) {
	
	// Send update request
	DictionaryIterator* iter;
 	app_message_outbox_begin(&iter);
	dict_write_cstring(iter, 1, "update");
	app_message_outbox_send();
	
	// Register timer again
	TorrentList.updateTimer = app_timer_register(15 * 1000, TorrentList_timerTick, NULL);
	
}












/******************************************************** Bluetooth Connection ************************************************/
void TorrentList_connectionHandler(bool connected) {
	
	// Check connection
	if (!connected) {
		text_layer_set_text(TorrentList.statusText, "Please connect phone");
		return;
	}
	
	// Send update request
	text_layer_set_text(TorrentList.statusText, "Connecting to phone...");
	DictionaryIterator* iter;
 	app_message_outbox_begin(&iter);
	dict_write_cstring(iter, 1, "update");
	app_message_outbox_send();
	
}












/******************************************************** App Message ************************************************/

void TorrentList_msgDropped(AppMessageResult reason, void* context) {
	
	APP_LOG(APP_LOG_LEVEL_WARNING, "Dropped");
	
}

void TorrentList_msgFailed(DictionaryIterator* iterator, AppMessageResult reason, void* context) {
	
	APP_LOG(APP_LOG_LEVEL_WARNING, "Message failed to send");
	
}

void TorrentList_msgReceived(DictionaryIterator* received, void* context) {
	
	// Get message type
	Tuple* action = dict_find(received, 1);
	if (!action)
		return;
	
	// Check message type
	if (strcmp(action->value->cstring, "invalid_settings") == 0) {
		
		// User needs to set settings in the app
		text_layer_set_text(TorrentList.statusText, "Check settings on phone");		
		
	} else if (strcmp(action->value->cstring, "invalid_login") == 0) {
		
		// Updating the torrent list has begun
		text_layer_set_text(TorrentList.statusText, "Authentication failed");		
		
	} else if (strcmp(action->value->cstring, "connection_failed") == 0) {
		
		// Updating the torrent list has begun
		text_layer_set_text(TorrentList.statusText, "Couldn't connect to uTorrent");		
		
	} else if (strcmp(action->value->cstring, "update_started") == 0) {
		
		// Updating the torrent list has begun
		text_layer_set_text(TorrentList.statusText, "Connecting to uTorrent...");		
		
	} else if (strcmp(action->value->cstring, "listing_torrents") == 0) {
		
		// Updating the torrent list has begun
		text_layer_set_text(TorrentList.statusText, "Getting torrent list...");		
		
	} else if (strcmp(action->value->cstring, "update_finished") == 0) {
		
		// Finished getting torrents, hide loading screen
		layer_set_hidden(bitmap_layer_get_layer(TorrentList.bigIcon), true);
		layer_set_hidden(text_layer_get_layer(TorrentList.statusText), true);
		layer_set_hidden(menu_layer_get_layer(TorrentList.list), false);
		
		// Update list
		menu_layer_reload_data(TorrentList.list);
		
		// Check if there are any torrents
		if (TorrentList.numTorrents == 0) {
			
			// Show empty label
			layer_set_hidden(text_layer_get_layer(TorrentList.emptyText), false);
			return;
			
		}
		
		// Check if should update info view
		if (TorrentList.visibleTorrent && TorrentInfo_isVisible())
			TorrentInfo_show(TorrentList.visibleTorrent);
		
	} else if (strcmp(action->value->cstring, "set_name") == 0) {
		
		// Get hash
		Tuple* hash = dict_find(received, 2);
		if (!action)
			return;
		
		// Get torrent
		TorrentInfo* torrent = TorrentList_getTorrent(hash->value->cstring);
		if (!torrent)
			return;
		
		// Get value
		Tuple* value = dict_find(received, 3);
		if (!value)
			return;
		
		// Set value
		strcpy(torrent->name, value->value->cstring);
		
	} else if (strcmp(action->value->cstring, "set_active") == 0) {
		
		// Get hash
		Tuple* hash = dict_find(received, 2);
		if (!action)
			return;
		
		// Get torrent
		TorrentInfo* torrent = TorrentList_getTorrent(hash->value->cstring);
		if (!torrent)
			return;
		
		// Get value
		Tuple* value = dict_find(received, 3);
		if (!value)
			return;
		
		// Set value
		torrent->active = value->value->int32 != 0;
		
	} else if (strcmp(action->value->cstring, "set_size") == 0) {
		
		// Get hash
		Tuple* hash = dict_find(received, 2);
		if (!action)
			return;
		
		// Get torrent
		TorrentInfo* torrent = TorrentList_getTorrent(hash->value->cstring);
		if (!torrent)
			return;
		
		// Get value
		Tuple* value = dict_find(received, 3);
		if (!value)
			return;
		
		// Set value
		torrent->size = value->value->int32;
		torrent->size *= 1024;
		
	} else if (strcmp(action->value->cstring, "set_downloaded") == 0) {
		
		// Get hash
		Tuple* hash = dict_find(received, 2);
		if (!action)
			return;
		
		// Get torrent
		TorrentInfo* torrent = TorrentList_getTorrent(hash->value->cstring);
		if (!torrent)
			return;
		
		// Get value
		Tuple* value = dict_find(received, 3);
		if (!value)
			return;
		
		// Set value
		torrent->downloaded = value->value->int32;
		torrent->downloaded *= 1024;
		
	} else if (strcmp(action->value->cstring, "set_uploaded") == 0) {
		
		// Get hash
		Tuple* hash = dict_find(received, 2);
		if (!action)
			return;
		
		// Get torrent
		TorrentInfo* torrent = TorrentList_getTorrent(hash->value->cstring);
		if (!torrent)
			return;
		
		// Get value
		Tuple* value = dict_find(received, 3);
		if (!value)
			return;
		
		// Set value
		torrent->uploaded = value->value->int32;
		torrent->uploaded *= 1024;
		
	} else if (strcmp(action->value->cstring, "set_downloadSpeed") == 0) {
		
		// Get hash
		Tuple* hash = dict_find(received, 2);
		if (!action)
			return;
		
		// Get torrent
		TorrentInfo* torrent = TorrentList_getTorrent(hash->value->cstring);
		if (!torrent)
			return;
		
		// Get value
		Tuple* value = dict_find(received, 3);
		if (!value)
			return;
		
		// Set value
		torrent->downloadSpeed = value->value->int32;
		
	} else if (strcmp(action->value->cstring, "set_uploadSpeed") == 0) {
		
		// Get hash
		Tuple* hash = dict_find(received, 2);
		if (!action)
			return;
		
		// Get torrent
		TorrentInfo* torrent = TorrentList_getTorrent(hash->value->cstring);
		if (!torrent)
			return;
		
		// Get value
		Tuple* value = dict_find(received, 3);
		if (!value)
			return;
		
		// Set value
		torrent->uploadSpeed = value->value->int32;
		
	} else if (strcmp(action->value->cstring, "set_timeLeft") == 0) {
		
		// Get hash
		Tuple* hash = dict_find(received, 2);
		if (!action)
			return;
		
		// Get torrent
		TorrentInfo* torrent = TorrentList_getTorrent(hash->value->cstring);
		if (!torrent)
			return;
		
		// Get value
		Tuple* value = dict_find(received, 3);
		if (!value)
			return;
		
		// Set value
		torrent->timeLeft = value->value->int32;
		
	} else {
		
		APP_LOG(APP_LOG_LEVEL_WARNING, "Unknown action received: %s", action->value->cstring);
		
	}
	
}















/**************************************************** Window functions ***********************************************/

void TorrentList_show() {
	
	// Show window
	window_stack_push(TorrentList.window, true);
	
}

void TorrentList_load() {
	
	// Create big icon
	int width = 144;
	int height = 168-16;
	TorrentList.bigIcon = bitmap_layer_create(GRect(width/2 - 64/2, height/2 - 64/2, 64, 64));
	bitmap_layer_set_bitmap(TorrentList.bigIcon, Resources.bigIcon);
	layer_add_child(window_get_root_layer(TorrentList.window), bitmap_layer_get_layer(TorrentList.bigIcon));
	
	// Create status text area
	TorrentList.statusText = text_layer_create(GRect(0, height-20, width, 20));
	text_layer_set_text_alignment(TorrentList.statusText, GTextAlignmentCenter);
	text_layer_set_font(TorrentList.statusText, fonts_get_system_font(FONT_KEY_GOTHIC_14));
	text_layer_set_text(TorrentList.statusText, "Connecting to phone...");
	layer_add_child(window_get_root_layer(TorrentList.window), text_layer_get_layer(TorrentList.statusText));
	
	// Create empty text area
	TorrentList.emptyText = text_layer_create(GRect(0, height/2-20/2, width, 20));
	text_layer_set_text_alignment(TorrentList.emptyText, GTextAlignmentCenter);
	text_layer_set_font(TorrentList.emptyText, fonts_get_system_font(FONT_KEY_GOTHIC_14));
	text_layer_set_text(TorrentList.emptyText, "No torrents");
	layer_add_child(window_get_root_layer(TorrentList.window), text_layer_get_layer(TorrentList.emptyText));
	layer_set_hidden(text_layer_get_layer(TorrentList.emptyText), true);
	
	// Create list
	TorrentList.list = menu_layer_create(GRect(0, 0, width, height));
	layer_add_child(window_get_root_layer(TorrentList.window), menu_layer_get_layer(TorrentList.list));
	layer_set_hidden(menu_layer_get_layer(TorrentList.list), true);
	
	// Setup list callbacks
	TorrentList.listCallbacks.get_header_height = NULL;
	TorrentList.listCallbacks.draw_header = NULL;
	TorrentList.listCallbacks.get_cell_height = TorrentList_getCellHeight;
	TorrentList.listCallbacks.draw_row = TorrentList_drawRow;
	TorrentList.listCallbacks.get_num_rows = (MenuLayerGetNumberOfRowsInSectionsCallback) TorrentList_getNumRows;
	TorrentList.listCallbacks.get_num_sections = NULL;
	TorrentList.listCallbacks.select_click = TorrentList_itemSelected;
	TorrentList.listCallbacks.select_long_click = NULL;
	TorrentList.listCallbacks.selection_changed = NULL;
	menu_layer_set_callbacks(TorrentList.list, NULL, TorrentList.listCallbacks);
	menu_layer_set_click_config_onto_window(TorrentList.list, TorrentList.window);
	
	// Register for app messages
	app_message_register_inbox_received(TorrentList_msgReceived);
	app_message_register_inbox_dropped(TorrentList_msgDropped);
	app_message_register_outbox_failed(TorrentList_msgFailed);
	
	// Start update timer
	TorrentList.updateTimer = app_timer_register(15 * 1000, TorrentList_timerTick, NULL);
	
}

void TorrentList_unload() {
	
	// Destroy stuff
	bitmap_layer_destroy(TorrentList.bigIcon);
	text_layer_destroy(TorrentList.statusText);
	text_layer_destroy(TorrentList.emptyText); 
	menu_layer_destroy(TorrentList.list);
	
	TorrentList.bigIcon = NULL;
	TorrentList.statusText = NULL;
	TorrentList.emptyText = NULL;
	TorrentList.list = NULL;
	
	// Deregister for app messages
	app_message_deregister_callbacks();
	
	// Stop updating
	if (TorrentList.updateTimer)
		app_timer_cancel(TorrentList.updateTimer);
	TorrentList.updateTimer = NULL;
	
}

void TorrentList_deinit() {
	
	// Destroy window
	window_destroy(TorrentList.window);
	
	// Destroy torrent info window
	TorrentInfo_deinit();
	
}

void TorrentList_appear() {
	
	// Check if connected to phone
	if (bluetooth_connection_service_peek())
		text_layer_set_text(TorrentList.statusText, "Connecting to phone...");
	else if (bluetooth_connection_service_peek())
		text_layer_set_text(TorrentList.statusText, "Please connect phone");
	
	// Register for bluetooth events
	bluetooth_connection_service_subscribe(TorrentList_connectionHandler);
	
	// Send update request
	DictionaryIterator* iter;
 	app_message_outbox_begin(&iter);
	dict_write_cstring(iter, 1, "update");
	app_message_outbox_send();
	
}

void TorrentList_disappear() {
	
	// Deregister for bluetooth events
	bluetooth_connection_service_unsubscribe();
	
}

void TorrentList_init() {
	
	// Init vars
	TorrentList.numTorrents = 0;
	
	// Create window
	TorrentList.window = window_create();
		
	// Set window handlers
	TorrentList.handlers.load = TorrentList_load;
	TorrentList.handlers.unload = TorrentList_unload;
	TorrentList.handlers.appear = TorrentList_appear;
	TorrentList.handlers.disappear = TorrentList_disappear;
	window_set_window_handlers(TorrentList.window, TorrentList.handlers);
	
	// Open app message inbox
	app_message_open(128, 128);
	
	// Init torrent info window
	TorrentInfo_init();

}