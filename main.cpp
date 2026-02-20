#include "src/main_window.h"

#include <QApplication>
#include <QTranslator>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    main_window w;
    w.show();
    QTranslator q_transalor;
    q_transalor.load("./asset/qtbase_zh_CN.qm");
    a.installTranslator(&q_transalor);
    return QApplication::exec();
}