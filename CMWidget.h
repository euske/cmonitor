/*  CMWidget.h
 */

#ifndef _CMWidget_h
#define _CMWidget_h

/* Resources:
 * Name		     Class		RepType		Default Value
 * ----		     -----		-------		-------------
 * (from RectObj)
 * ancestorSensitive AncestorSensitive	Boolean		True
 * x		     Position		Position	0
 * y		     Position		Position	0
 * width	     Width		Dimension	text width
 * height	     Height		Dimension	text height
 * borderWidth	     BorderWidth	Dimension	1
 * sensitive	     Sensitive		Boolean		True
 *
 * (from WindowObj)
 * screen	     Screen		Screen		parent's Screen
 * depth	     Depth		int		parent's depth
 * colormap	     Colormap		Colormap	parent's colormap
 * background	     Background		Pixel		XtDefaultBackground
 * backgroundPixmap  Pixmap		Pixmap		XtUnspecifiedPixmap
 * borderColor	     BorderColor	Pixel		XtDefaultForeground
 * borderPixmap	     Pixmap		Pixmap		XtUnspecifiedPixmap
 * mappedWhenManaged MappedWhenManaged	Boolean		True
 * translations	     Translations	TranslationTable see doc or source
 * accelerators	     Accelerators	AcceleratorTable NULL
 *
 * (from Core)
 * none
 *
 * (from CMWidget)
 * scale	     Scale	        int		2
 * loadColor	     LoadColor		Pixel		"red"
 * memoryColor	     MemoryColor	Pixel		"green"
 * diskReadColor     DiskReadColor	Pixel		"cyan"
 * diskWriteColor    DiskWriteColor	Pixel		"cyan"
 * netRecvColor	     NetRecvColor	Pixel		"yellow"
 * netSendColor      NetSendColor	Pixel		"yellow"
 */


#include <X11/Intrinsic.h>
#include <X11/Xaw/Simple.h>

#define XtNscale "scale"
#define XtNupdateTicks "updateTicks"
#define XtNloadColor "loadColor"
#define XtNmemoryColor "memoryColor"
#define XtNdiskReadColor "diskReadColor"
#define XtNdiskWriteColor "diskWriteColor"
#define XtNnetRecvColor "netRecvColor"
#define XtNnetSendColor "netSendColor"
#define XtCScale "Scale"
#define XtCUpdateTicks "UpdateTicks"
#define XtCLoadColor "LoadColor"
#define XtCMemoryColor "MemoryColor"
#define XtCDiskReadColor "DiskReadColor"
#define XtCDiskWriteColor "DiskWriteColor"
#define XtCNetRecvColor "NetRecvColor"
#define XtCNetSendColor "NetSendColor"

extern WidgetClass cmWidgetClass;

typedef struct _CMWidgetClassRec *CMWidgetClass;
typedef struct _CMWidgetRec *CMWidget;

extern void CMWidgetIdleTask(Widget w);

#endif
