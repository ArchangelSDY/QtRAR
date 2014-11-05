#include <QTest>
#include <QFile>

#include "../src/qtrar.h"

#include "testqtrar.h"

void TestQtRAR::entriesCount()
{
    QFETCH(QString, arcName);
    QFETCH(int, entriesCount);

    QtRAR rar(arcName);
    rar.open(QtRAR::OpenModeList);

    QVERIFY2(rar.isOpen(), "RAR is not open");
    QCOMPARE(rar.entriesCount(), entriesCount);
}

void TestQtRAR::entriesCount_data()
{
    QTest::addColumn<QString>("arcName");
    QTest::addColumn<int>("entriesCount");

    QTest::newRow("single file archive")
        << "single.rar"
        << 1;
}
