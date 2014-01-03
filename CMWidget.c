/*  CMWidget.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/StringDefs.h>
#include <X11/IntrinsicP.h>
#include <X11/Xaw/SimpleP.h>
#include "CMWidget.h"

typedef struct _NetContext {
  unsigned long _net_recv0, _net_recv1;
  unsigned long _net_send0, _net_send1;
  int net_recv, net_send;
} NetContext;

typedef struct _DiskContext {
  unsigned long _disk_read0, _disk_read1;
  unsigned long _disk_write0, _disk_write1;
  int disk_read, disk_write;
} DiskContext;

static int GetLogValue(unsigned long v)
{
  int i = 0;
  while (v) {
    v >>= 1;
    i++;
  }
  return i;
}

static void UpdateNetContext(NetContext* context, const char* path)
{
  FILE* fp = fopen(path, "r");
  char buf[256];
  if (fp != NULL) {
    int i = 0;
    context->_net_recv0 = context->_net_recv1;
    context->_net_recv1 = 0;
    context->_net_send0 = context->_net_send1;
    context->_net_send1 = 0;
    while (fgets(buf, sizeof(buf), fp) != NULL) {
      if (2 <= i) {
	unsigned long xu, brecv = 0, bsend = 0;
	char xw[256];
	if (sscanf(buf, "%s %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu",
		   xw,  &brecv, &xu, &xu, &xu, &xu, &xu, &xu, &xu,  &bsend, &xu, &xu, &xu, &xu, &xu, &xu, &xu)) {
	  context->_net_recv1 += brecv;
	  context->_net_send1 += bsend;
	}
      }
      i++;
    }
    fclose(fp);
  }
  if (context->_net_recv0 == 0) {
    context->_net_recv0 = context->_net_recv1;
  }
  if (context->_net_send0 == 0) {
    context->_net_send0 = context->_net_send1;
  }
  context->net_recv = GetLogValue(context->_net_recv1 - context->_net_recv0);
  context->net_send = GetLogValue(context->_net_send1 - context->_net_send0);
}

static void UpdateDiskContext(DiskContext* context, const char* path)
{
  FILE* fp = fopen(path, "r");
  char buf[256];
  if (fp != NULL) {
    context->_disk_read0 = context->_disk_read1;
    context->_disk_read1 = 0;
    context->_disk_write0 = context->_disk_write1;
    context->_disk_write1 = 0;
    while (fgets(buf, sizeof(buf), fp) != NULL) {
      unsigned long xu, bread = 0, bwrite = 0;
      char xw[256];
      if (sscanf(buf, "%lu %lu %s %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu",
		 &xu, &xu, xw, &xu, &xu,  &bread, &xu, &xu,  &bwrite, &xu, &xu, &xu, &xu, &xu)) {
	context->_disk_read1 += bread;
	context->_disk_write1 += bwrite;
      }
    }
    fclose(fp);
  }
  if (context->_disk_read0 == 0) {
    context->_disk_read0 = context->_disk_read1;
  }
  if (context->_disk_write0 == 0) {
    context->_disk_write0 = context->_disk_write1;
  }
  context->disk_read = GetLogValue(context->_disk_read1 - context->_disk_read0);
  context->disk_write = GetLogValue(context->_disk_write1 - context->_disk_write0);
}

static int GetLoadAverage(const char* path)
{
  FILE* fp = fopen(path, "r");
  float v = 0;
  char buf[256];
  if (fp != NULL) {
    while (fgets(buf, sizeof(buf), fp) != NULL) {
      sscanf(buf, "%f", &v);
      break;
    }
    fclose(fp);
  }
  return v*100;
}

static int GetMemoryPercentage(const char* path)
{
  FILE* fp = fopen(path, "r");
  unsigned long total = 0, unused = 0;
  char buf[256];
  if (fp != NULL) {
    while (fgets(buf, sizeof(buf), fp) != NULL) {
      char name[256];
      unsigned long v;
      if (sscanf(buf, "%s %lu", name, &v)) {
	if (!strcmp(name, "MemTotal:")) {
	  total += v;
	} else if (!strcmp(name, "MemFree:") ||
		   !strcmp(name, "Buffers:") ||
		   !strcmp(name, "Cached:")) {
	  unused += v;
	}
      }
    }
    fclose(fp);
  }
  return (0 == total)? 0 : (100 * (total-unused) / total);
}


/* CMWidget Instance */

/* New fields for the widget record */
typedef struct _CMWidgetPart {
  NetContext netContext;
  DiskContext diskContext;

  int scale;
  int updateTicks;
  Pixel loadColor;
  Pixel memoryColor;
  Pixel diskReadColor;
  Pixel diskWriteColor;
  Pixel netRecvColor;
  Pixel netSendColor;

  int loadValue;
  int memoryValue;
  int diskReadValue;
  int diskWriteValue;
  int netRecvValue;
  int netSendValue;
} CMWidgetPart;

/* Full instance record declaration */
typedef struct _CMWidgetRec {
  CorePart core;
  SimplePart simple;
  CMWidgetPart cmwidget;
} CMWidgetRec;

/* CMWidget Resources */
#define offset(f) XtOffsetOf(CMWidgetRec, f)
static XtResource resources[] = {
  /* core */
  {
    XtNwidth, XtCWidth, XtRDimension, sizeof(Dimension),
    offset(core.width), XtRString, "100",
  },
  {
    XtNheight, XtCHeight, XtRDimension, sizeof(Dimension),
    offset(core.height), XtRString, "2",
  },

  /* self */
  {
    XtNscale, XtCScale, XtRInt, sizeof(int),
    offset(cmwidget.scale), XtRString, "2",
  },
  {
    XtNupdateTicks, XtCUpdateTicks, XtRInt, sizeof(int),
    offset(cmwidget.updateTicks), XtRString, "1000",
  },
  {
    XtNloadColor, XtCLoadColor, XtRPixel, sizeof(Pixel),
    offset(cmwidget.loadColor), XtRString, "red",
  },
  {
    XtNmemoryColor, XtCMemoryColor, XtRPixel, sizeof(Pixel),
    offset(cmwidget.memoryColor), XtRString, "gray",
  },
  {
    XtNdiskReadColor, XtCDiskReadColor, XtRPixel, sizeof(Pixel),
    offset(cmwidget.diskReadColor), XtRString, "yellow",
  },
  {
    XtNdiskWriteColor, XtCDiskWriteColor, XtRPixel, sizeof(Pixel),
    offset(cmwidget.diskWriteColor), XtRString, "orange",
  },
  {
    XtNnetRecvColor, XtCNetRecvColor, XtRPixel, sizeof(Pixel),
    offset(cmwidget.netRecvColor), XtRString, "green",
  },
  {
    XtNnetSendColor, XtCNetSendColor, XtRPixel, sizeof(Pixel),
    offset(cmwidget.netSendColor), XtRString, "darkgreen",
  },
};
#undef offset

/* CMWidget Actions */
static XtActionsRec actionsList[] = {};

/* CMWidget Methods */
static void ClassInitialize(void);
static void Initialize(Widget request, Widget new, ArgList args, Cardinal *num_args);
static void Destroy(Widget w);
static void Realize(Widget w, XtValueMask* valueMask, XSetWindowAttributes* attributes);
static void Redisplay(Widget w, XEvent *event, Region region);
static void Resize(Widget w);

/* CMWidget Class Definition */
typedef struct _CMWidgetClassRec {
  CoreClassPart core_class;
  SimpleClassPart simple_class;
} CMWidgetClassRec;

#define SuperClass ((SimpleWidgetClass)&simpleClassRec)
CMWidgetClassRec cmWidgetClassRec = {
    /* core */
    {
	(WidgetClass) SuperClass, /* superclass             */    
	"CMWidget",		  /* class_name             */
	sizeof(CMWidgetRec),	  /* size                   */
	ClassInitialize,	  /* class_initialize       */
	NULL,			  /* class_part_initialize  */
	FALSE,			  /* class_inited           */
	Initialize,		  /* initialize             */
	NULL,			  /* initialize_hook        */
	Realize,		  /* realize                */
	actionsList,		  /* actions                */
	XtNumber(actionsList),	  /* num_actions            */
	resources,		  /* resources              */
	XtNumber(resources),	  /* resource_count         */
	NULLQUARK,		  /* xrm_class              */
	FALSE,			  /* compress_motion        */
	TRUE,			  /* compress_exposure      */
	TRUE,			  /* compress_enterleave    */
	FALSE,			  /* visible_interest       */
	Destroy,		  /* destroy                */
	Resize,			  /* resize                 */
	Redisplay,		  /* expose                 */
	NULL,			  /* set_values             */
	NULL,			  /* set_values_hook        */
	XtInheritSetValuesAlmost, /* set_values_almost      */
	NULL,			  /* get_values_hook        */
	XtInheritAcceptFocus,	  /* accept_focus           */
	XtVersion,		  /* version                */
	NULL,			  /* callback_private       */
	NULL,			  /* tm_table               */
	XtInheritQueryGeometry,	  /* query_geometry         */
	XtInheritDisplayAccelerator, /* display_accelerator    */
	NULL			     /* extension              */
    },
    /* simple */
    {
	XtInheritChangeSensitive,         /* change_sensitive       */
    },
};

WidgetClass cmWidgetClass = (WidgetClass)&cmWidgetClassRec;

static void set_timer(Widget w);
static void idle_task(XtPointer data, XtIntervalId* id);

static void set_timer(Widget w)
{
  CMWidget self = (CMWidget)w;
  XtAppAddTimeOut(XtWidgetToApplicationContext(w), 
		  self->cmwidget.updateTicks, idle_task, (XtPointer)w);
}

static void idle_task(XtPointer data, XtIntervalId* id)
{
  CMWidget self = (CMWidget)data;

  /* loadavg */
  self->cmwidget.loadValue = GetLoadAverage("/proc/loadavg")/20;

  /* meminfo */
  self->cmwidget.memoryValue = GetMemoryPercentage("/proc/meminfo")/4;

  /* disk */
  UpdateDiskContext(&self->cmwidget.diskContext, "/proc/diskstats");
  self->cmwidget.diskReadValue = self->cmwidget.diskContext.disk_read/4;
  self->cmwidget.diskWriteValue = self->cmwidget.diskContext.disk_write/4;

  /* net */
  UpdateNetContext(&self->cmwidget.netContext, "/proc/net/dev");
  self->cmwidget.netRecvValue = self->cmwidget.netContext.net_recv/4;
  self->cmwidget.netSendValue = self->cmwidget.netContext.net_send/4;

#ifdef DEBUG
  printf(" load=%d, memory=%d, disk=%d/%d, net=%d/%d\n", 
	 self->cmwidget.loadValue, self->cmwidget.memoryValue,
	 self->cmwidget.diskReadValue, self->cmwidget.diskWriteValue,
	 self->cmwidget.netRecvValue, self->cmwidget.netSendValue);
#endif

  Redisplay((Widget)self, NULL, None);
  set_timer((Widget)data);
}


/* ClassInitialize */
static void ClassInitialize(void)
{
}

/* Initialize */
static void Initialize(Widget request,
		       Widget new,
		       ArgList args,
		       Cardinal *num_args)
{
  CMWidget self = (CMWidget)new;
  //fprintf(stderr, "initialize\n");
  memset(&(self->cmwidget.diskContext), 0, sizeof(DiskContext));
  memset(&(self->cmwidget.netContext), 0, sizeof(NetContext));
  self->cmwidget.loadValue = 0;
  self->cmwidget.memoryValue = 0;
  self->cmwidget.diskReadValue = 0;
  self->cmwidget.diskWriteValue = 0;
  self->cmwidget.netRecvValue = 0;
  self->cmwidget.netSendValue = 0;
}

/* Destroy */
static void Destroy(Widget w)
{
  //CMWidget self = (CMWidget)new;
  //fprintf(stderr, "destroy\n");
}

/* Realize */
static void Realize(Widget w,
		    XtValueMask* valueMask,
		    XSetWindowAttributes* attributes)
{
  //CMWidget self = (CMWidget)new;
  //fprintf(stderr, "realize\n");
  (*SuperClass->core_class.realize)(w, valueMask, attributes);
  set_timer(w);
}

/* Resize */
static void Resize(Widget w)
{
  //CMWidget self = (CMWidget)new;
  //fprintf(stderr, "resize\n");
}

/* Redisplay */
static void drawBar(Widget w, int x, Pixel p, int size)
{
  XGCValues gcv;
  gcv.foreground = p;
  GC gc = XtGetGC(w, GCForeground, &gcv);
  XFillRectangle(XtDisplay(w), XtWindow(w), gc, x, 0, size, w->core.height);
  XtReleaseGC(w, gc);
}
static void Redisplay(Widget w,
		      XEvent *event, 
		      Region region)
{
  CMWidget self = (CMWidget)w;
  //fprintf(stderr, "redisplay\n");
  XClearWindow(XtDisplay(w), XtWindow(w));
  {
    int scale = self->cmwidget.scale;
    int x = 0;
    if (self->cmwidget.loadValue) {
      drawBar(w, x, self->cmwidget.loadColor, scale * self->cmwidget.loadValue);
      x += scale * self->cmwidget.loadValue;
      x += scale;
    }
    if (self->cmwidget.memoryValue) {
      drawBar(w, x, self->cmwidget.memoryColor, scale * self->cmwidget.memoryValue);
      x += scale * self->cmwidget.memoryValue;
      x += scale;
    }
    if (self->cmwidget.diskReadValue || self->cmwidget.diskWriteValue) {
      drawBar(w, x, self->cmwidget.diskReadColor, scale * self->cmwidget.diskReadValue);
      x += scale * self->cmwidget.diskReadValue;
      drawBar(w, x, self->cmwidget.diskWriteColor, scale * self->cmwidget.diskWriteValue);
      x += scale * self->cmwidget.diskWriteValue;
      x += scale;
    }
    if (self->cmwidget.netRecvValue || self->cmwidget.netSendValue) {
      drawBar(w, x, self->cmwidget.netRecvColor, scale * self->cmwidget.netRecvValue);
      x += scale * self->cmwidget.netRecvValue;
      drawBar(w, x, self->cmwidget.netSendColor, scale * self->cmwidget.netSendValue);
      x += scale * self->cmwidget.netSendValue;
      x += scale;
    }
  }
}
