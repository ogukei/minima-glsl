
#include <CoreGraphics/CoreGraphics.h>
#include "private.h"

#include <time.h>

bool make(CGSConnectionID c, CGRect bounds, CGSWindowID *ref) {
	CGSRegion region;
	if (CGSNewRegionWithRect(&bounds, &region) != kCGSErrorSuccess) {
		return false;
	}
	CGSWindowID window;
	if (CGSNewWindow(c, kCGSBufferedBackingType, 0, 0, region, &window) != kCGSErrorSuccess) {
		return false;
	}
	if (CGSSetWindowOpacity(c, window, kCGSFalse) != kCGSErrorSuccess) {
		return false;
	}
	*ref = window;
	return true;
}

int main(void) {
	CGSConnectionID connection = CGSMainConnectionID();
	CGRect bounds = CGRectMake(0, 0, 1000, 1000);
	CGSWindowID window = 0;
	make(connection, bounds, &window);
	CGContextRef context = CGWindowContextCreate(connection, window, 0);
	CGContextSetCompositeOperation(context, kCGCompositeCopy);
	CGContextSetRGBFillColor(context, 0.0, 1.0, 0.0, 0.5);
	CGContextFillRect(context, bounds);
	CGContextFlush(context);
	CGSOrderWindow(connection, window, kCGSWindowOrderingAbove, 0);

	sleep(1);
	return 0;
}

