//
// Torrent info

#ifndef _TORRENTINFO_H_
#define _TORRENTINFO_H_

#include <pebble.h>
#include "TorrentList.h"

void TorrentInfo_init();
void TorrentInfo_show(TorrentInfo* torrent);
bool TorrentInfo_isVisible();
void TorrentInfo_deinit();

#endif