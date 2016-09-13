#ifndef FCITX_CLOUDPINYIN_H
#define FCITX_CLOUDPINYIN_H
#include <fcitx-config/fcitx-config.h>
#include <fcitx/instance.h>


typedef struct {
    FcitxInstance* owner;

    boolean should_show;

    boolean is_showing;
} FcitxTouchScreen;

void touchscreen_handle_showing(FcitxTouchScreen* ts);


void debug_printf(const char* fmt, ...);
#endif
