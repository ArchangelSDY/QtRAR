#include <QCoreApplication>
#include <QTest>

#include "testqtrar.h"

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    int err = 0;
    {
        TestQtRAR testQtRAR;
        err = qMax(err, QTest::qExec(&testQtRAR, app.arguments()));
    }

    if (err != 0) {
        qWarning("There were errors in some of the tests above.");
    }

    return err;
}
