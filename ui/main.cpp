#include <QApplication>
#include <QLabel>
#include <QWebView>
#include <QMainWindow>
#include <QDesktopWidget>
#include <stdlib.h>

#include <QDebug>
#include <signal.h>

extern "C" {
#include "X_misc.h"
}

class MainWindow: public QDesktopWidget {
};

class Page: public QWebPage {
public:
    void javaScriptAlert(QWebFrame* frame, const QString& msg) {
        QString cmd = "xdotool type " + msg;
        QByteArray ba = cmd.toLatin1();

        int ret = system(ba.data());

        qDebug() << cmd << ret;
    }
};

const int VK_HEIGHT = 300;
const int VK_WIDTH = 1000;
const int dock_height = 48;

void handle_show(QMainWindow& qw)
{
    QRect rec = QApplication::desktop()->screenGeometry();

    
    qw.resize(rec.width(), VK_HEIGHT);
    qw.move(0, rec.height()-VK_HEIGHT-dock_height);
    
    Display* dpy = XOpenDisplay("");
    Window w = qw.effectiveWinId();


    qDebug() << "HEHE" << rec;


    set_wmspec_dock_hint(dpy, w);
        
    set_struct_partial(dpy, w, ORIENTATION_BOTTOM, dock_height+VK_HEIGHT, 0, rec.width());
    XCloseDisplay(dpy);
}

int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    QMainWindow window(0, Qt::Window);
    window.setAttribute(Qt::WA_TranslucentBackground, true);

    
    QWebView view(&window);
    view.setStyleSheet("background:transparent");

    QRect rec = QApplication::desktop()->screenGeometry();
    view.resize(VK_WIDTH, VK_HEIGHT);
    Page page;

    view.setPage(&page);
    view.load(QUrl("qrc:///index.mini.html"));
    view.move(view.parentWidget()->geometry().center().x(), 0);

    window.show();
    
    handle_show(window);
    return a.exec();
}
