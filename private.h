
#ifndef CGS_PRIVATE_H
#define CGS_PRIVATE_H

#include <Carbon/Carbon.h>

// Core Graphics Private APIs
typedef int CGSConnectionID;
typedef int CGSWindowID;
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

extern CGSConnectionID CGSMainConnectionID(void);

extern CGError CGSNewRegionWithRect(CGRect *, CGSRegion *);
extern CGError CGSReleaseRegion(CGSRegion);

extern CGError CGSNewWindow(CGSConnectionID, int backing, float, float, CGSRegion, CGSWindowID *);
extern CGError CGSSetWindowOpacity(CGSConnectionID, CGSWindowID, CGSBoolean opacity);
extern CGError CGSOrderWindow(CGSConnectionID, CGSWindowID, int ordering, CGSWindowID relative);

extern CGContextRef CGWindowContextCreate(CGSConnectionID, CGSWindowID, void *);

// CGContext
extern CGError CGContextSetCompositeOperation(CGContextRef, int);

#endif
