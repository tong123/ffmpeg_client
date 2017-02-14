#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "wifi-client.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QDebug>
#include <QQuickView>
#include <QDesktopWidget>
#include <qqmlprivate.h>
#include "image-item.h"

#include <QQmlApplicationEngine>
#include <QQuickView>
#include <QApplication>
#include <QDesktopWidget>
#include <QQmlContext>
#include <QDebug>
#include <QFont>
#include "image-widget.h"
//#include "testwidget.h"
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QQmlApplicationEngine engine;
    int n_width = QApplication::desktop()->width();
    int n_height = QApplication::desktop()->height();
    qDebug()<<n_width<<n_height;
    WifiClient *mp_client = new WifiClient;
    engine.rootContext()->setContextProperty( "wifi_client",  mp_client );
    qmlRegisterType<CImageItem>( "ImageItem", 1, 0, "ImageItem" );
    ImageWidget w( mp_client );
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
//    w.show();
//    TestWidget w_temp;
//    w_temp.show();
    return app.exec();
}
