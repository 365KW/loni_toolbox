#include "src/main_window.h"

#include <QApplication>
#include <QTranslator>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    QTranslator q_transalor;
    main_window w;
    w.show();
    q_transalor.load("./asset/qtbase_zh_CN.qm");
    a.installTranslator(&q_transalor);
    return QApplication::exec();
}