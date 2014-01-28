//
// Resource file

#ifndef _RESOURCES_H_
#define _RESOURCES_H_

#include <pebble.h>

struct _Resources {
	
	// Images
	GBitmap* bigIcon;
	GBitmap* downloadingIcon;
	GBitmap* completeIcon;
	GBitmap* stoppedIcon;
	GBitmap* pauseSmall;
	GBitmap* playSmall;
	
};

extern struct _Resources Resources;

void resources_load();

#endif