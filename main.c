
#define GL_SILENCE_DEPRECATION

#include <CoreGraphics/CoreGraphics.h>
#include <CoreVideo/CoreVideo.h>
#include <OpenGL/CGLCurrent.h>
#include <OpenGL/gl3.h>

#include "private.h"

#include <time.h>

static bool MakeCGLContext(CGSConnectionID, CGSWindowID, CGRect, CGLContextObj *);
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

static bool MakeCGLContext(CGSConnectionID connection, CGSWindowID window, CGRect bounds, CGLContextObj *ref) {
	CGLPixelFormatAttribute attributes[] = {
		kCGLPFAOpenGLProfile, (CGLPixelFormatAttribute)kCGLOGLPVersion_3_2_Core,
		kCGLPFAColorSize, 24,
		kCGLPFAAlphaSize, 8,
		kCGLPFADoubleBuffer,
		kCGLPFAAccelerated,
		kCGLPFASampleBuffers, 1,
		kCGLPFASamples, 4,
		(CGLPixelFormatAttribute)0
	};
	// Context Allocation
	CGLPixelFormatObj format;
	GLint nvirt; // number of virtual screens
 	if (CGLChoosePixelFormat(attributes, &format, &nvirt) != kCGLNoError) {
		return false;	 
	}
	CGLContextObj context;
	if (CGLCreateContext(format, NULL, &context) != kCGLNoError) {
		return false;
	}
	CGLDestroyPixelFormat(format);
	format = NULL;
	// Context Setup
	CGLSetCurrentContext(context);
	// V-Sync
	GLint v_sync_enabled = 1;
	CGLSetParameter(context, kCGLCPSwapInterval, &v_sync_enabled);
	// Opaque
	GLint opaque = 0;
	CGLSetParameter(context, kCGLCPSurfaceOpacity, &opaque);
	// Surface Setup
	CGSSurfaceID surface;
	if (CGSAddSurface(connection, window, &surface) != kCGSErrorSuccess) {
		return false;
	}
	CGSSetSurfaceBounds(connection, window, surface, bounds);
	CGSOrderSurface(connection, window, surface, 1, 0);
	CGLSetSurface(context, connection, window, surface);
	// Checks
	GLint drawable;
	if (CGLGetParameter(context, kCGLCPHasDrawable, &drawable) != kCGLNoError) {
		return false;
	}
	if (drawable == 0) {
		return false;
	}
	CGLSetCurrentContext(NULL);
	*ref = context;
	return true;
}