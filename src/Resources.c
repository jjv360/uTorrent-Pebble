#include "Resources.h"
	
struct _Resources Resources;

void resources_load() {
	
	// Load images
	Resources.bigIcon = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BIG_ICON);
	Resources.downloadingIcon = gbitmap_create_with_resource(RESOURCE_ID_DOWNLOAD_ICON);
	Resources.completeIcon = gbitmap_create_with_resource(RESOURCE_ID_COMPLETE_ICON);
	Resources.stoppedIcon = gbitmap_create_with_resource(RESOURCE_ID_STOP_ICON);
	
}