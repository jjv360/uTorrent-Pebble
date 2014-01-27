//
// Torrent list

#ifndef _TORRENTLIST_H_
#define _TORRENTLIST_H_

#include <pebble.h>

typedef struct TorrentInfoStruct {
	char hash[64];
	char name[64];
	bool active;
	long long size;
	long long downloaded;
	long long uploaded;
	int downloadSpeed;
	int uploadSpeed;
} TorrentInfo;

void TorrentList_init();
void TorrentList_show();
void TorrentList_deinit();

#endif