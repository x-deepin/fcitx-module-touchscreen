#include <signal.h>
#include <glib.h>

#include "keyboard.h"


    
static    pid_t child = 0;
static    int kill_source = 0;
static    int launch_source = 0;

#define TOUCHSCREEN_HELPER "/usr/bin/fcitx-touchscreen-helper"

void launch_child()
{
    static char* argv[] = {TOUCHSCREEN_HELPER, 0};
    g_spawn_async(0, argv, 0,
                  0, 0, 0,
                  &child, 0);
    debug_printf("LAUNCH>>>>>>%d\n", child);
    return;
}

void kill_child()
{
    if (child == 0) {
        debug_printf("Error: can't find a child to kill");
        return ;
    }
    
//    kill(child, SIGKILL);
    system("killall "TOUCHSCREEN_HELPER);
    debug_printf("KILL>>>>>>%d\n", child);
    child = 0;
    return ;
}

void close_virtual_keyboard(FcitxTouchScreen* ts)
{
    if (kill_source)
        return;
    
    ts->is_showing = false;
    if (launch_source) {
        FcitxInstanceRemoveTimeoutById(ts->owner, launch_source);
        launch_source = 0;
    }
    
    kill_source = FcitxInstanceAddTimeout(ts->owner, 100, kill_child, 0);
}

void open_virtual_keyboard(FcitxTouchScreen* ts)
{
    if (launch_source)
        return;
    
    ts->is_showing = true;
    if (kill_source) {
        FcitxInstanceRemoveTimeoutById(ts->owner, kill_source);
        kill_source = 0;
    }
    launch_source = FcitxInstanceAddTimeout(ts->owner, 100, launch_child, 0);
}


