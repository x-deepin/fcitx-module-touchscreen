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

#include "touchscreen.h"
#include "myxi.h"

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
    ts->child = 0;
    ts->launch_source = 0;
    ts->kill_source = 0;
    
    ts->owner = instance;
    
    init_watch_xi2(ts);

    FcitxIMEventHook hook;
    hook.arg = ts;
    hook.func = TouchScreenHookForFocus;
    FcitxInstanceRegisterInputFocusHook(instance, hook);
    FcitxInstanceRegisterInputUnFocusHook(instance, hook);

    return ts;
}

void TouchScreenDestroy(void* arg)
{
    sd_journal_print(LOG_DEBUG, "HUHU DESTROY\n");
    FcitxTouchScreen* ts = (FcitxTouchScreen*) arg;
    
    /* char c = 1; */
    /* write(ts->pipeNotify, &c, sizeof(char)); */
    /* pthread_join(ts->pid, NULL); */
    /* pthread_mutex_destroy(&ts->pendingQueueLock); */
    /* pthread_mutex_destroy(&ts->finishQueueLock); */
    /* FcitxConfigFree(&ts->config.config); */
    free(ts);
}

void TouchScreenReloadConfig(void* arg)
{
    /* FcitxTouchScreen* ts = (FcitxTouchScreen*) arg; */
    
    sd_journal_print(LOG_DEBUG, "HUHU RELOAD\n");
    /* CloudPinyinSource previousSource = ts->config.source; */
    /* CloudPinyinConfigLoad(&cloudpinyin->config); */
    /* if (previousSource != cloudpinyin->config.source) */
    /* { */
    /*     cloudpinyin->initialized = false; */
    /*     cloudpinyin->key[0] = '\0'; */
    /* } */
}


static void TouchScreenHookForFocus(void *arg)
{
    FcitxTouchScreen* ts = (FcitxTouchScreen*) arg;

    
    FcitxContextState state =  FcitxInstanceGetCurrentState(ts->owner);
    switch (state) {
    case IS_CLOSED:
        close_virtual_keyboard(ts);
        break;
    case IS_INACTIVE:
    case IS_ACTIVE:
        if (ts->should_show)
            open_virtual_keyboard(ts);
        break;
    }
}



