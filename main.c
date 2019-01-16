
#include <CoreGraphics/CoreGraphics.h>
#include <CoreVideo/CoreVideo.h>
#include "private.h"

#include <time.h>

static bool MakeWindow(CGSConnectionID, CGRect, CGSWindowID *);
static CVReturn DisplayLinkCallback(CVDisplayLinkRef, const CVTimeStamp *, const CVTimeStamp *, 
	CVOptionFlags, CVOptionFlags *, void *);

typedef struct {
	CGSConnectionID connection;
	CGSWindowID window;
	CGRect bounds;
	CGContextRef context;
}
Environment;

int main(void) {
	CGSConnectionID connection = CGSMainConnectionID();
	CGRect frame = CGRectMake(0, 0, 400, 400);
	CGSWindowID window = 0;
	MakeWindow(connection, frame, &window);

	CGRect bounds = CGRectMake(0, 0, CGRectGetWidth(frame), CGRectGetHeight(frame));
	CGContextRef context = CGWindowContextCreate(connection, window, 0);
	CGContextSetCompositeOperation(context, kCGCompositeCopy);
	CGContextSetRGBFillColor(context, 0.0, 1.0, 0.0, 0.5);
	CGContextFillRect(context, bounds);
	CGContextFlush(context);
	CGSOrderWindow(connection, window, kCGSWindowOrderingAbove, 0);

	Environment *environment = &(Environment) {
		.connection = connection,
		.window = window,
		.bounds = bounds,
		.context = context
	};

	CVDisplayLinkRef display;
	CVDisplayLinkCreateWithCGDisplay(CGMainDisplayID(), &display);
	CVDisplayLinkSetOutputCallback(display, &DisplayLinkCallback, environment);
	CVDisplayLinkStart(display);
	sleep(1);
	return 0;
}


static CVReturn DisplayLinkCallback(CVDisplayLinkRef display, 
	const CVTimeStamp *time_now, const CVTimeStamp *time_output, 
	CVOptionFlags inputs, CVOptionFlags *outputs, void *context) {
	Environment *environment = context;

	return 0;
}

static bool MakeWindow(CGSConnectionID connection, CGRect frame, CGSWindowID *ref) {
	CGSRegion region;
	if (CGSNewRegionWithRect(&frame, &region) != kCGSErrorSuccess) {
		return false;
	}
	CGSWindowID window;
	if (CGSNewWindow(connection, kCGSBufferedBackingType, 0, 0, region, &window) != kCGSErrorSuccess) {
		CGSReleaseRegion(region);
		return false;
	}
	CGSReleaseRegion(region);
	if (CGSSetWindowOpacity(connection, window, kCGSFalse) != kCGSErrorSuccess) {
		return false;
	}
	*ref = window;
	return true;
}