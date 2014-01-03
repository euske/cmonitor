/*  cmonitor.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/StringDefs.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/Xmu/Editres.h>
#include <X11/Xlocale.h> 
#include "CMWidget.h"

int main(int argc, char* argv[])
{
  static char *fallback_resources[] = {
    NULL
  };
  static XrmOptionDescRec options[] = {
    {"-scale", "*Scale", XrmoptionSepArg, NULL},
    {"-update", "*UpdateTicks", XrmoptionSepArg, NULL},
  };

  XtAppContext app_context;
  Widget toplevel;
  
  XtSetLanguageProc(NULL, NULL, NULL);
  
  toplevel = XtVaAppInitialize(&app_context, "CMonitor", 
			       options, XtNumber(options),
			       &argc, argv, fallback_resources,
			       NULL, NULL);
  if (argc != 1) {
    printf("usage: %s [XtOptions] [-update ticks]\n", argv[0]);
    exit(1);
  }

  XtVaCreateManagedWidget("cmwidget", cmWidgetClass, toplevel,
			  NULL, NULL);
  XtAddEventHandler(toplevel, (EventMask)0, True,
		    _XEditResCheckMessages, NULL);
  XtRealizeWidget(toplevel);
  XtAppMainLoop(app_context);
  
  /*NOTREACHED*/
  return 0;
}
