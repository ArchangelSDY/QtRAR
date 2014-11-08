#include <QTest>

#include "../src/qtrarfile.h"
#include "../src/qtrarfileinfo.h";

#include "testqtrarfile.h"

Q_DECLARE_METATYPE(Qt::CaseSensitivity)

void TestQtRARFile::openAndRead()
{
    QFETCH(QString, arcName);
    QFETCH(QString, fileName);
    QFETCH(Qt::CaseSensitivity, caseSensitivity);
    QFETCH(bool, isOpen);
    QFETCH(QByteArray, content);
    QFETCH(qint64, usize);
    QFETCH(qint64, csize);

    QtRARFile f(arcName, fileName, caseSensitivity);
    QCOMPARE(f.open(QtRARFile::ReadOnly), isOpen);
    QCOMPARE(f.openMode(), isOpen ? QIODevice::ReadOnly : QIODevice::NotOpen);

    if (isOpen) {
        QtRARFileInfo info;
        QVERIFY2(f.fileInfo(&info), "get file info failed");
        QCOMPARE(info.packSize, csize);
        QCOMPARE(info.unpSize, usize);
        QCOMPARE(f.csize(), csize);
        QCOMPARE(f.usize(), usize);

        QByteArray actualContent = f.readAll();
        QCOMPARE(actualContent, content);
        QCOMPARE(actualContent.size(), f.size());
        QCOMPARE(actualContent.size(), f.usize());
    }

    f.close();
}

void TestQtRARFile::openAndRead_data()
{
    QTest::addColumn<QString>("arcName");
    QTest::addColumn<QString>("fileName");
    QTest::addColumn<Qt::CaseSensitivity>("caseSensitivity");
    QTest::addColumn<bool>("isOpen");
    QTest::addColumn<QByteArray>("content");
    QTest::addColumn<qint64>("usize");
    QTest::addColumn<qint64>("csize");

    QTest::newRow("first file in archive")
        << "multiple.rar"
        << "qt2.txt"
        << Qt::CaseSensitive
        << true
        << QByteArray("rar2\n")
        << Q_INT64_C(5)
        << Q_INT64_C(15);
    QTest::newRow("second file in archive")
        << "multiple.rar"
        << "qt.txt"
        << Qt::CaseSensitive
        << true
        << QByteArray("rar\n")
        << Q_INT64_C(4)
        << Q_INT64_C(13);
    QTest::newRow("file not found")
        << "multiple.rar"
        << "QT.txt"
        << Qt::CaseSensitive
        << false
        << QByteArray()
        << Q_INT64_C(4)
        << Q_INT64_C(13);
    QTest::newRow("case insensitive")
        << "multiple.rar"
        << "QT.txt"
        << Qt::CaseInsensitive
        << true
        << QByteArray("rar\n")
        << Q_INT64_C(4)
        << Q_INT64_C(13);
}
