/**
 * Copyright (c) 2011 ~ 2012 Deepin, Inc.
 *               2011 ~ 2012 snyh
 *
 * Author:      snyh <snyh@snyh.org>
 * Maintainer:  snyh <snyh@snyh.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 **/
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <limits.h>
#include <stdio.h>
#include <X11/extensions/shape.h>
#include "X_misc.h"

#define FALSE 0


static void* get_window_property(Display* dsp, Window w, Atom pro, unsigned long* items);
static void set_wmspec_window_type_hint(Display* dpy, Window w, Atom type);

#define X_FETCH_32(data, i) *((unsigned long*)data + i)
#define X_FETCH_16(data, i) *((short*)data + i)
#define X_FETCH_8(data, i) *((char*)data + i)

        
void set_wmspec_dock_hint(Display* dpy, Window w)
{
    Atom type = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DOCK", False);
    set_wmspec_window_type_hint(dpy, w, type);
}

void set_wmspec_window_type_hint(Display* dpy, Window w, Atom type)
{
    Atom p = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", FALSE);

    XChangeProperty (dpy, w, p, XA_ATOM, 32, PropModeReplace, (char*)&type, 1);
}


void get_workarea_size(Display* dpy, int* x, int* y, int* width, int* height)
{
    Atom property = XInternAtom(dpy, "_NET_WORKAREA", False);
    unsigned long items;
    unsigned long* data = get_window_property(dpy, DefaultRootWindow(dpy), property, &items);

    if (data && items == 4) {
        *x = X_FETCH_32(data, 0);
        *y = X_FETCH_32(data, 1);
        *width = X_FETCH_32(data, 2);
        *height = X_FETCH_32(data, 3);
        XFree(data);
    } else {
        *x = *y = *width = *height = 0;
    }
}

enum {
	STRUT_LEFT = 0,
	STRUT_RIGHT = 1,
	STRUT_TOP = 2,
	STRUT_BOTTOM = 3,
	STRUT_LEFT_START = 4,
	STRUT_LEFT_END = 5,
	STRUT_RIGHT_START = 6,
	STRUT_RIGHT_END = 7,
	STRUT_TOP_START = 8,
	STRUT_TOP_END = 9,
	STRUT_BOTTOM_START = 10,
	STRUT_BOTTOM_END = 11
};



void set_struct_partial(Display* display, Window window, uint32_t orientation, uint32_t strut, uint32_t strut_start, uint32_t strut_end)
{
    unsigned long   struts [12] = { 0, };
    
    static Atom net_wm_strut_partial      = None;
    if (net_wm_strut_partial == None)
        net_wm_strut_partial = XInternAtom (display, "_NET_WM_STRUT_PARTIAL", False);

    switch (orientation) {
        case ORIENTATION_LEFT:
            struts [STRUT_LEFT] = strut;
            struts [STRUT_LEFT_START] = strut_start;
            struts [STRUT_LEFT_END] = strut_end;
            break;
        case ORIENTATION_RIGHT:
            struts [STRUT_RIGHT] = strut;
            struts [STRUT_RIGHT_START] = strut_start;
            struts [STRUT_RIGHT_END] = strut_end;
            break;
        case ORIENTATION_TOP:
            struts [STRUT_TOP] = strut;
            struts [STRUT_TOP_START] = strut_start;
            struts [STRUT_TOP_END] = strut_end;
            break;
        case ORIENTATION_BOTTOM:
            struts [STRUT_BOTTOM] = strut;
            struts [STRUT_BOTTOM_START] = strut_start;
            struts [STRUT_BOTTOM_END] = strut_end;
            break;
    }

    XChangeProperty (display, window, net_wm_strut_partial,
            XA_CARDINAL, 32, PropModeReplace,
            (unsigned char *) &struts, 12);
}


void* get_window_property(Display* dsp, Window w, Atom pro, unsigned long* items)
{
    Atom act_type;
    int act_format;
    unsigned long bytes_after;
    unsigned char* p_data = NULL;

    int result = XGetWindowProperty(dsp, w, pro,
            0, ULONG_MAX, FALSE,
            AnyPropertyType, &act_type,
            &act_format,
            items,
            &bytes_after,
            (void*)&p_data);

    if (result != Success) {
        dprintf(2, "get_window_property error... %d\n", (int)w);
        return NULL;
    } else {
        return p_data;
    }
}
