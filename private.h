
#ifndef CGS_PRIVATE_H
#define CGS_PRIVATE_H

#include <Carbon/Carbon.h>
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

// Connection
extern CGSConnectionID CGSMainConnectionID(void);

// Region
extern CGError CGSNewRegionWithRect(CGRect *, CGSRegion *);
extern CGError CGSReleaseRegion(CGSRegion);

// Window
extern CGError CGSNewWindow(CGSConnectionID, int backing, float, float, CGSRegion, CGSWindowID *);
extern CGError CGSSetWindowOpacity(CGSConnectionID, CGSWindowID, CGSBoolean opacity);
extern CGError CGSOrderWindow(CGSConnectionID, CGSWindowID, int ordering, CGSWindowID relative);

// Window - CG
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

#endif
