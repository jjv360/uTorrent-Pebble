#include "TorrentInfo.h"
#include "TorrentList.h"
#include "Resources.h"

/***************************************** Vars ***************************************/
	
struct TorrentInfoClass {
	
	// Views
	Window* window;
	WindowHandlers handlers;
	BitmapLayer* icon;
	TextLayer* title;
	TextLayer* sizeLbl;
	TextLayer* size;
	TextLayer* downLbl;
	TextLayer* down;
	TextLayer* upLbl;
	TextLayer* up;
	TextLayer* speedLbl;
	TextLayer* speed;
	TextLayer* etaLbl;
	TextLayer* eta;
	
	// Vars
	double progress;
	char sizeBfr[64];
	char downloadedBfr[64];
	char uploadedBfr[64];
	char speedBfr[64];
	char etaBfr[64];
	
} TorrentInfoWindow;
	
	

	
	
	
	
	
	
	
	
	
	

	
	
/***************************************** Window functions ***************************************/

void formatBytes(char* bfr, long long bytes, bool speed) {
	
	if (bytes < 1024)
		snprintf(bfr, 64, "%lli B%s", bytes, (speed ? "/s" : ""));
	else if (bytes < 1024*1024)
		snprintf(bfr, 64, "%lli KB%s", bytes/1024, (speed ? "/s" : ""));
	else if (bytes < 1024*1024*1024)
		snprintf(bfr, 64, "%lli MB%s", bytes/1024/1024, (speed ? "/s" : ""));
	else
		snprintf(bfr, 64, "%lli GB%s", bytes/1024/1024/1024, (speed ? "/s" : ""));
	
}
	
void TorrentInfo_show(TorrentInfo* torrent) {
	
	// Show window
	if (window_stack_get_top_window() != TorrentInfoWindow.window)
		window_stack_push(TorrentInfoWindow.window, true);
	
	// Set icon
	if (torrent->downloaded >= torrent->size)
		bitmap_layer_set_bitmap(TorrentInfoWindow.icon, Resources.completeIcon);
	else if (torrent->active)
		bitmap_layer_set_bitmap(TorrentInfoWindow.icon, Resources.downloadingIcon);
	else
		bitmap_layer_set_bitmap(TorrentInfoWindow.icon, Resources.stoppedIcon);
	
	// Set title
	text_layer_set_text(TorrentInfoWindow.title, torrent->name);
	
	// Set size
	formatBytes(TorrentInfoWindow.sizeBfr, torrent->size, false);
	text_layer_set_text(TorrentInfoWindow.size, TorrentInfoWindow.sizeBfr);
	
	// Set down
	formatBytes(TorrentInfoWindow.downloadedBfr, torrent->downloaded, false);
	text_layer_set_text(TorrentInfoWindow.down, TorrentInfoWindow.downloadedBfr);
	
	// Set up
	formatBytes(TorrentInfoWindow.uploadedBfr, torrent->uploaded, false);
	text_layer_set_text(TorrentInfoWindow.up, TorrentInfoWindow.uploadedBfr);
	
	// Set speed
	formatBytes(TorrentInfoWindow.speedBfr, torrent->downloadSpeed + torrent->uploadSpeed, true);
	text_layer_set_text(TorrentInfoWindow.speed, TorrentInfoWindow.speedBfr);
	
	// Set ETA
	text_layer_set_text(TorrentInfoWindow.eta, "-");
	
	// Redraw window and progress bar
	TorrentInfoWindow.progress = (double) torrent->downloaded / torrent->size;
	layer_mark_dirty(window_get_root_layer(TorrentInfoWindow.window));
	
}

void TorrentInfo_load() {
	
	// Create icon view
	int width = 144;
	int height = 168-16;
	TorrentInfoWindow.icon = bitmap_layer_create(GRect(0, 0, 44, 44));
	layer_add_child(window_get_root_layer(TorrentInfoWindow.window), bitmap_layer_get_layer(TorrentInfoWindow.icon));
	
	// Create title layer
	TorrentInfoWindow.title = text_layer_create(GRect(44, 0, width-44, 44));
	text_layer_set_overflow_mode(TorrentInfoWindow.title, GTextOverflowModeWordWrap);
	text_layer_set_font(TorrentInfoWindow.title, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
	layer_add_child(window_get_root_layer(TorrentInfoWindow.window), text_layer_get_layer(TorrentInfoWindow.title));
	
	// Create size label layer
	TorrentInfoWindow.sizeLbl = text_layer_create(GRect(10, 60+16*0, 80, 16));
	text_layer_set_text(TorrentInfoWindow.sizeLbl, "Size:");
	text_layer_set_font(TorrentInfoWindow.sizeLbl, fonts_get_system_font(FONT_KEY_GOTHIC_14));
	layer_add_child(window_get_root_layer(TorrentInfoWindow.window), text_layer_get_layer(TorrentInfoWindow.sizeLbl));
	
	TorrentInfoWindow.size = text_layer_create(GRect(width-60, 60+16*0, 50, 16));
	text_layer_set_text(TorrentInfoWindow.size, "0 KB");
	text_layer_set_text_alignment(TorrentInfoWindow.size, GTextAlignmentRight);
	text_layer_set_font(TorrentInfoWindow.size, fonts_get_system_font(FONT_KEY_GOTHIC_14));
	layer_add_child(window_get_root_layer(TorrentInfoWindow.window), text_layer_get_layer(TorrentInfoWindow.size));
	
	// Create down label layer
	TorrentInfoWindow.downLbl = text_layer_create(GRect(10, 60+16*1, 80, 16));
	text_layer_set_text(TorrentInfoWindow.downLbl, "Downloaded:");
	text_layer_set_font(TorrentInfoWindow.downLbl, fonts_get_system_font(FONT_KEY_GOTHIC_14));
	layer_add_child(window_get_root_layer(TorrentInfoWindow.window), text_layer_get_layer(TorrentInfoWindow.downLbl));
	
	TorrentInfoWindow.down = text_layer_create(GRect(width-60, 60+16*1, 50, 16));
	text_layer_set_text(TorrentInfoWindow.down, "0 KB");
	text_layer_set_text_alignment(TorrentInfoWindow.down, GTextAlignmentRight);
	text_layer_set_font(TorrentInfoWindow.down, fonts_get_system_font(FONT_KEY_GOTHIC_14));
	layer_add_child(window_get_root_layer(TorrentInfoWindow.window), text_layer_get_layer(TorrentInfoWindow.down));
	
	// Create up label layer
	TorrentInfoWindow.upLbl = text_layer_create(GRect(10, 60+16*2, 80, 16));
	text_layer_set_text(TorrentInfoWindow.upLbl, "Uploaded:");
	text_layer_set_font(TorrentInfoWindow.upLbl, fonts_get_system_font(FONT_KEY_GOTHIC_14));
	layer_add_child(window_get_root_layer(TorrentInfoWindow.window), text_layer_get_layer(TorrentInfoWindow.upLbl));
	
	TorrentInfoWindow.up = text_layer_create(GRect(width-60, 60+16*2, 50, 16));
	text_layer_set_text(TorrentInfoWindow.up, "0 KB");
	text_layer_set_text_alignment(TorrentInfoWindow.up, GTextAlignmentRight);
	text_layer_set_font(TorrentInfoWindow.up, fonts_get_system_font(FONT_KEY_GOTHIC_14));
	layer_add_child(window_get_root_layer(TorrentInfoWindow.window), text_layer_get_layer(TorrentInfoWindow.up));
	
	// Create down speed label layer
	TorrentInfoWindow.speedLbl = text_layer_create(GRect(10, 60+16*3, 80, 16));
	text_layer_set_text(TorrentInfoWindow.speedLbl, "Speed:");
	text_layer_set_font(TorrentInfoWindow.speedLbl, fonts_get_system_font(FONT_KEY_GOTHIC_14));
	layer_add_child(window_get_root_layer(TorrentInfoWindow.window), text_layer_get_layer(TorrentInfoWindow.speedLbl));
	
	TorrentInfoWindow.speed = text_layer_create(GRect(width-60, 60+16*3, 50, 16));
	text_layer_set_text(TorrentInfoWindow.speed, "0 KB");
	text_layer_set_text_alignment(TorrentInfoWindow.speed, GTextAlignmentRight);
	text_layer_set_font(TorrentInfoWindow.speed, fonts_get_system_font(FONT_KEY_GOTHIC_14));
	layer_add_child(window_get_root_layer(TorrentInfoWindow.window), text_layer_get_layer(TorrentInfoWindow.speed));
	
	// Create eta label layer
	TorrentInfoWindow.etaLbl = text_layer_create(GRect(10, 60+16*4, 80, 16));
	text_layer_set_text(TorrentInfoWindow.etaLbl, "ETA:");
	text_layer_set_font(TorrentInfoWindow.etaLbl, fonts_get_system_font(FONT_KEY_GOTHIC_14));
	layer_add_child(window_get_root_layer(TorrentInfoWindow.window), text_layer_get_layer(TorrentInfoWindow.etaLbl));
	
	TorrentInfoWindow.eta = text_layer_create(GRect(width-60, 60+16*4, 50, 16));
	text_layer_set_text(TorrentInfoWindow.eta, "0 KB");
	text_layer_set_text_alignment(TorrentInfoWindow.eta, GTextAlignmentRight);
	text_layer_set_font(TorrentInfoWindow.eta, fonts_get_system_font(FONT_KEY_GOTHIC_14));
	layer_add_child(window_get_root_layer(TorrentInfoWindow.window), text_layer_get_layer(TorrentInfoWindow.eta));
	
}

void TorrentInfo_draw(struct Layer* layer, GContext* ctx) {
	
	// Fill with white
	int width = 144;
	int height = 168-16;
	graphics_context_set_fill_color(ctx, GColorWhite);
	graphics_fill_rect(ctx, GRect(0, 0, width, height), 0, GCornerNone);
	
	// Draw progress bar
	graphics_context_set_fill_color(ctx, GColorBlack);
	graphics_draw_round_rect(ctx, GRect(8, 44+8, width-16, 8), 2);
	
	// Draw progress
	int barWidth = TorrentInfoWindow.progress * (width-16);
	graphics_fill_rect(ctx, GRect(8, 44+8, barWidth, 8), 2, GCornersAll);
	
}

void TorrentInfo_unload() {
	
	// Unload stuff
	bitmap_layer_destroy(TorrentInfoWindow.icon);
	
}

void TorrentInfo_appear() {
	
	
	
}

void TorrentInfo_disappear() {
	
	
	
}
	
void TorrentInfo_init() {
	
	// Create window
	TorrentInfoWindow.window = window_create();
	layer_set_update_proc(window_get_root_layer(TorrentInfoWindow.window), TorrentInfo_draw);
	
	// Set window handlers
	TorrentInfoWindow.handlers.load = TorrentInfo_load;
	TorrentInfoWindow.handlers.unload = TorrentInfo_unload;
	TorrentInfoWindow.handlers.appear = TorrentInfo_appear;
	TorrentInfoWindow.handlers.disappear = TorrentInfo_disappear;
	window_set_window_handlers(TorrentInfoWindow.window, TorrentInfoWindow.handlers);
	
}
	
void TorrentInfo_deinit() {
	
	// Destroy window
	window_destroy(TorrentInfoWindow.window);
	
}