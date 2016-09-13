#include <X11/extensions/XInput2.h>
#include <glib.h>
#include <fcitx/module/x11/fcitx-x11.h>

#include "myxi.h"
#include "module.h"
    
static int xi_opcode = 0;
static void build_device_info(Display* dpy);
gboolean device_is_keyboard(int id);
gboolean device_is_touchscreen(int id);



// Add a filter to watch all XI events, and maintain the FcitxTouchScreen->should_show
// the should_show is true if there has at least one touchscreen device and hasn't any keyboard device source
// after the touchscreen device in the last XI event's source device queue.
boolean watch_xi2_event(FcitxTouchScreen* ts, XEvent *ev)
{
    Display *dpy = FcitxX11GetDisplay(ts->owner);

    if (ev->xcookie.type == GenericEvent &&
        ev->xcookie.extension == xi_opcode &&
        XGetEventData(dpy, &(ev->xcookie)))
    {
        XIDeviceEvent *xi_ev = (XIDeviceEvent*)(ev->xcookie.data);
        
        switch(ev->xcookie.evtype)
        {
        case XI_ButtonPress:
        case XI_KeyPress:
            if (device_is_touchscreen(xi_ev->deviceid) && !ts->should_show) {
                ts->should_show = true;
                FcitxInstanceAddTimeout(ts->owner, 10, touchscreen_handle_showing, ts);
                debug_printf("ShouldShow:%d", true);
            } else if (device_is_keyboard(xi_ev->deviceid) && ts->should_show) {
                ts->should_show = false;
                FcitxInstanceAddTimeout(ts->owner, 10, touchscreen_handle_showing, ts);
                debug_printf("ShouldShow:%d\n", false);
            }
            break;
        case XI_DeviceChanged:
            build_device_info(dpy);
            break;
        }
        XFreeEventData(dpy, &(ev->xcookie));
    }
}

void init_watch_xi2(FcitxTouchScreen* ts)
{
    Display *dpy = FcitxX11GetDisplay(ts->owner);
    int event, error;
    if (!XQueryExtension(dpy, "XInputExtension", &xi_opcode, &event, &error))
        debug_printf("X Input extension not available.\n");
    
    int major = 2, minor = 0;
    if (XIQueryVersion(dpy, &major, &minor) == BadRequest) {
        debug_printf("XI2 not available. Server supports %d.%d\n", major, minor);
        return -1;
    }


    XIEventMask eventmask;
    unsigned char mask[1] = { 0 }; /* the actual mask */

    eventmask.deviceid = XIAllDevices;
    eventmask.mask_len = sizeof(mask); /* always in bytes */
    eventmask.mask = mask;
/* now set the mask */
    XISetMask(mask, XI_ButtonPress);
    XISetMask(mask, XI_KeyPress);

    
    Window root = XDefaultRootWindow(dpy);
    XISelectEvents(dpy, root, &eventmask, 1);


    build_device_info(dpy);
    FcitxX11AddXEventHandler(ts->owner, watch_xi2_event, ts);
}




typedef struct _DInfo {
    int id;
    gboolean keyboard;
    gboolean touchscreen;
} DInfo;

static GHashTable* infos = 0;
void update_device_info(int id, gboolean keyboard, gboolean touchscreen)
{
    DInfo* info = g_new(DInfo, 1);
    info->id = id;
    info->keyboard = keyboard;
    info->touchscreen = touchscreen;
    
    g_hash_table_insert(infos, id, info);
}
gboolean device_is_keyboard(int id)
{
    DInfo *info = g_hash_table_lookup(infos, id);
    if (info == 0) {
        return FALSE;
    }
    return info->keyboard;
}
gboolean device_is_touchscreen(int id)
{
    DInfo *info = g_hash_table_lookup(infos, id);
    if (info == 0) {
        return FALSE;
    }
    return info->touchscreen;
}

void build_device_info(Display* dpy)
{
    if (infos == 0) {
        infos = g_hash_table_new_full(0, 0, 0, g_free);
    } else {
        g_hash_table_remove_all(infos);
    }


    int n = 0;
    XIDeviceInfo* xinfos = XIQueryDevice(dpy, XIAllDevices, &n);
    for (int i=0; i<n; i++) {
        gboolean is_keyboard = FALSE;
        gboolean is_touchscreen = FALSE;
    
        XIDeviceInfo info = xinfos[i];
        if (!info.enabled || strstr(info.name, "Virtual core XTEST"))
            continue;

        switch (info.use) {
        case XISlavePointer:
        {
            for (int j=0; j<info.num_classes; j++) {
                XIAnyClassInfo* cinfo = info.classes[j];
                if (is_touchscreen || cinfo->type != XITouchClass)
                    continue;

                XITouchClassInfo* tinfo = (XITouchClassInfo*)cinfo;
                is_touchscreen = tinfo->mode == XIDirectTouch;
            }
            break;
        }
        case XISlaveKeyboard:
            is_keyboard = TRUE;
            break;
        }
        if (is_keyboard || is_touchscreen)
            update_device_info(info.deviceid, is_keyboard, is_touchscreen);
    }

    XIFreeDeviceInfo(xinfos);
}


const char* show_type(int t)
{
    switch (t) {
    case XIMasterPointer:
        return "XIMasterPointer";
    case XIMasterKeyboard:
        return "XIMasterKeyboard";
    case XISlavePointer:
        return "XISlavePointer";
    case XISlaveKeyboard:
        return "XISlaveKeyboard";
    case XIFloatingSlave:
        return "XIFloatingSlave";
    }

}


void debug_device_info(Display* dpy)
{
    int n = 0;
    XIDeviceInfo* infos = XIQueryDevice(dpy, XIAllDevices, &n);

    for (int i=0; i<n; i++) {
        XIDeviceInfo info = infos[i];
        debug_printf("Dev:%s(%d)\n", info.name, info.deviceid);
        debug_printf("T: %s, A: %d, E: %d \n", show_type(info.use), info.attachment, info.enabled);
        debug_printf("And it support %d feature as below\n\t", info.num_classes);
        for (int j=0; j<info.num_classes; j++) {
            XIAnyClassInfo* cinfo = info.classes[j];
            switch(cinfo->type) {
            case XIKeyClass:
                debug_printf(" XIKeyClass ");
                break;
            case XIButtonClass:
                debug_printf(" XIButtonClass ");
                break;
            case XIValuatorClass:
                debug_printf(" XIValuatorClass ");
                break;
            case XIScrollClass:
                debug_printf(" XIScrollClass ");
                break;
            case XITouchClass:
            {
                XITouchClassInfo* tinfo = (XITouchClassInfo*)cinfo;
                debug_printf(" XITouchClass %s N:%d", tinfo->mode == XIDirectTouch ? "Direct" : "Indirect",
                        tinfo->num_touches);
                break;
            }
            }
        }
        debug_printf("\n");
    }

    XIFreeDeviceInfo(infos);
}
