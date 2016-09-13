#include <errno.h>
#include <unistd.h>
#include <systemd/sd-journal.h>
#include <glib.h>
#include  <X11/extensions/XInput2.h>


#include <fcitx/fcitx.h>
#include <fcitx/module.h>
#include <fcitx/hook.h>
#include <fcitx-utils/log.h>
#include <fcitx/candidate.h>
#include <fcitx-config/xdg.h>
#include <fcitx/module/x11/fcitx-x11.h>

#include "module.h"
#include "myxi.h"
#include "keyboard.h"

static void* TouchScreenCreate(FcitxInstance* instance);
static void TouchScreenDestroy(void* arg);
static void TouchScreenReloadConfig(void* arg);

static void TouchScreenHookForFocus(void *arg);

CONFIG_DESC_DEFINE(GetTouchScreenConfigDesc, "fcitx-touchscreen.desc")

FCITX_DEFINE_PLUGIN(fcitx_touchscreen, module, FcitxModule) = {
    .Create = TouchScreenCreate,
    .Destroy = TouchScreenDestroy,
    .ReloadConfig = TouchScreenReloadConfig
};


void* TouchScreenCreate(FcitxInstance* instance)
{

    FcitxTouchScreen *ts = fcitx_utils_new(FcitxTouchScreen);
    ts->owner = instance;
    ts->is_showing = false;
    ts->should_show = false;
    
    init_watch_xi2(ts);

    FcitxKeyFilterHook hook;
    hook.arg = ts;
    hook.func = touchscreen_handle_showing;
    FcitxInstanceRegisterPreInputFilter(instance, hook) ;

    
    FcitxIMEventHook hook2;
    hook2.arg = ts;
    hook2.func = touchscreen_handle_showing;
    FcitxInstanceRegisterInputFocusHook(instance, hook2);
    FcitxInstanceRegisterInputUnFocusHook(instance, hook2);

    return ts;
}

void TouchScreenDestroy(void* arg)
{
    FcitxTouchScreen* ts = (FcitxTouchScreen*) arg;
    free(ts);
}

void TouchScreenReloadConfig(void* arg)
{
}


void touchscreen_handle_showing(FcitxTouchScreen* ts)
{
    FcitxContextState state = FcitxInstanceGetCurrentState(ts->owner);
    if (!ts->should_show || state == IS_CLOSED) {
        close_virtual_keyboard(ts);
    } else if (ts->should_show) {
        open_virtual_keyboard(ts);
    }
    debug_printf("set_should_show: should_show:%d is_showing:%d state:%d\n",
                     ts->should_show,
                     ts->is_showing,
                     FcitxInstanceGetCurrentState(ts->owner));
}

void debug_printf(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);

//    vprintf(fmt, args);
    
//    sd_journal_print(LOG_DEBUG, format, args);
    
    va_end(args);
}
