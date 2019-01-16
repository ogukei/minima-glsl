
#ifndef CGS_PRIVATE_H
#define CGS_PRIVATE_H

#include <CoreGraphics/CGBase.h>
#include <OpenGL/CGLTypes.h>

// Core Graphics Private APIs
typedef int CGSConnectionID;
typedef int CGSWindowID;
typedef int CGSSurfaceID;
typedef int CGSBoolean;
typedef void *CGSRegion;

// CGSBoolean
#define kCGSFalse 0
#define kCGSTrue 1

// CGSError
#define kCGSErrorSuccess 0

// enum
#define kCGSBufferedBackingType 2
#define kCGSWindowOrderingAbove 1
#define kCGCompositeCopy 1 // CGCompositeOperation
#define kCGSIgnoreMouseEventsTag 512

// Connection
extern CGSConnectionID CGSMainConnectionID(void);

// Display
extern CGDirectDisplayID CGSMainDisplayID(void);
extern CGError CGSGetDisplayBounds(CGDirectDisplayID, CGRect *);

// Region
extern CGError CGSNewRegionWithRect(CGRect *, CGSRegion *);
extern CGError CGSReleaseRegion(CGSRegion);

// Window
extern CGError CGSNewWindow(CGSConnectionID, int backing, float, float, CGSRegion, CGSWindowID *);
extern CGError CGSSetWindowOpacity(CGSConnectionID, CGSWindowID, CGSBoolean opacity);
extern CGError CGSOrderWindow(CGSConnectionID, CGSWindowID, int ordering, CGSWindowID relative);
extern CGError CGSSetWindowLevel(CGSConnectionID, CGSWindowID, CGWindowLevel);
extern CGContextRef CGWindowContextCreate(CGSConnectionID, CGSWindowID, void *);

// CGContext
extern CGError CGContextSetCompositeOperation(CGContextRef, int);

// Surface
extern CGError CGSAddSurface(CGSConnectionID, CGSWindowID, CGSSurfaceID *);
extern CGError CGSRemoveSurface(CGSConnectionID, CGSWindowID, CGSSurfaceID *);
extern CGError CGSSetSurfaceBounds(CGSConnectionID, CGSWindowID, CGSSurfaceID, CGRect);
extern CGError CGSOrderSurface(CGSConnectionID, CGSWindowID, CGSSurfaceID, int, int);

// CGL
extern CGLError CGLSetSurface(CGLContextObj, CGSConnectionID, CGSWindowID, CGSSurfaceID);

// Tags
extern CGError CGSSetWindowTags(CGSConnectionID, CGSWindowID, int *tags, int always32);

#endif
