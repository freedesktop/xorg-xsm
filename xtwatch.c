/* $Xorg: xtwatch.c,v 1.4 2001/02/09 02:06:01 xorgcvs Exp $ */
/******************************************************************************

Copyright 1993, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.
******************************************************************************/

#include <X11/ICE/ICElib.h>
#include <X11/Intrinsic.h>
#include "xsm.h"

extern void CloseDownClient ();



Status
InitWatchProcs (appContext)

XtAppContext appContext;

{
    void _XtIceWatchProc ();

    return (IceAddConnectionWatch (_XtIceWatchProc, (IcePointer) appContext));
}


void
_XtIceWatchProc (ice_conn, client_data, opening, watch_data)

IceConn 	ice_conn;
IcePointer	client_data;
Bool		opening;
IcePointer	*watch_data;

{
    if (opening)
    {
	XtAppContext appContext = (XtAppContext) client_data;
	void _XtProcessIceMsgProc ();

	*watch_data = (IcePointer) XtAppAddInput (
	    appContext,
	    IceConnectionNumber (ice_conn),
            (XtPointer) XtInputReadMask,
	    _XtProcessIceMsgProc,
	    (XtPointer) ice_conn);
    }
    else
    {
	XtRemoveInput ((XtInputId) *watch_data);
    }
}


void
_XtProcessIceMsgProc (client_data, source, id)

XtPointer	client_data;
int 		*source;
XtInputId	*id;

{
    IceConn			ice_conn = (IceConn) client_data;
    IceProcessMessagesStatus	status;

    status = IceProcessMessages (ice_conn, NULL, NULL);

    if (status == IceProcessMessagesIOError)
    {
	List *cl;
	int found = 0;

	if (verbose)
	{
	    printf ("IO error on connection (fd = %d)\n",
	        IceConnectionNumber (ice_conn));
	    printf ("\n");
	}

	for (cl = ListFirst (RunningList); cl; cl = ListNext (cl))
	{
	    ClientRec *client = (ClientRec *) cl->thing;

	    if (client->ice_conn == ice_conn)
	    {
		CloseDownClient (client);
		found = 1;
		break;
	    }
	}
	 
	if (!found)
	{
	    /*
	     * The client must have disconnected before it was added
	     * to the session manager's running list (i.e. before the
	     * NewClientProc callback was invoked).
	     */

	    IceSetShutdownNegotiation (ice_conn, False);
	    IceCloseConnection (ice_conn);
	}
    }
}
