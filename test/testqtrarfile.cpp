#include <QTest>

#include "../src/qtrar.h"
#include "../src/qtrarfile.h"
#include "../src/qtrarfileinfo.h"

#include "testqtrarfile.h"

Q_DECLARE_METATYPE(Qt::CaseSensitivity)

void TestQtRARFile::initTestCase()
{
    m_rar = new QtRAR("multiple.rar");
    m_rar->open(QtRAR::OpenModeExtract);
}

void TestQtRARFile::cleanupTestCase()
{
    delete m_rar;
    m_rar = 0;
}

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
    QCOMPARE(f.arcName(), arcName);
    QCOMPARE(f.fileName().compare(fileName, caseSensitivity), 0);
    QCOMPARE(f.caseSensitivity(), caseSensitivity);

    if (isOpen) {
        QCOMPARE(f.actualFileName().compare(fileName, Qt::CaseInsensitive), 0);

        QtRARFileInfo info;
        QVERIFY2(f.fileInfo(&info), "get file info failed");
        QCOMPARE(info.packSize, csize);
        QCOMPARE(info.unpSize, usize);
        QCOMPARE(f.csize(), csize);
        QCOMPARE(f.usize(), usize);
        QCOMPARE(f.isSequential(), false);

        QCOMPARE(f.bytesAvailable(), usize);
        QCOMPARE(f.atEnd(), false);
        QCOMPARE(f.pos(), 0);

        QByteArray actualContent = f.readAll();
        QCOMPARE(actualContent, content);
        QCOMPARE(actualContent.size(), f.size());
        QCOMPARE(actualContent.size(), f.usize());

        QCOMPARE(f.bytesAvailable(), 0);
        QCOMPARE(f.atEnd(), true);
        QCOMPARE(f.pos(), usize);
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
    QTest::newRow("UTF-8 content")
        << "multiple-with-utf8.rar"
        << "中文.txt"
        << Qt::CaseInsensitive
        << true
        << QByteArray("中文\n")
        << Q_INT64_C(7)
        << Q_INT64_C(18);
}

void TestQtRARFile::constructor()
{
    QFETCH(QtRARFile *, f);
    QFETCH(bool, isOpen);
    QFETCH(bool, isRARInternal);
    QFETCH(QByteArray, content);

    QCOMPARE(f->open(QIODevice::ReadOnly), isOpen);
    QCOMPARE(f->isOpen(), isOpen);
    QCOMPARE(f->rar() == 0, isRARInternal);

    if (f->isOpen()) {
        QCOMPARE(f->readAll(), content);
    }

    f->close();
}

void TestQtRARFile::constructor_data()
{
    QTest::addColumn<QtRARFile *>("f");
    QTest::addColumn<bool>("isOpen");
    QTest::addColumn<bool>("isRARInternal");
    QTest::addColumn<QByteArray>("content");

    QtRARFile *fInitWithAllNull = new QtRARFile(this);
    fInitWithAllNull->setArchiveName("multiple.rar");
    fInitWithAllNull->setFileName("qt.txt");
    QTest::newRow("init with all null")
        << fInitWithAllNull
        << true
        << true
        << QByteArray("rar\n");

    QtRARFile *fInitWithArcNameOnly = new QtRARFile("multiple.rar", this);
    fInitWithArcNameOnly->setFileName("QT.txt", Qt::CaseInsensitive);
    QTest::newRow("init with only archive name")
        << fInitWithArcNameOnly
        << true
        << true
        << QByteArray("rar\n");

    QtRARFile *fInitWithRAR = new QtRARFile(m_rar, this);
    fInitWithRAR->setFileName("qt.txt");
    QTest::newRow("init with rar given")
        << fInitWithRAR
        << true
        << false
        << QByteArray("rar\n");

    QTest::newRow("cannot open: empty archive name")
        << new QtRARFile(this)
        << false
        << true
        << QByteArray();
    QTest::newRow("cannot open: empty file name")
        << new QtRARFile("multiple.rar", this)
        << false
        << true
        << QByteArray();
    QTest::newRow("cannot open: rar given but file name empty")
        << new QtRARFile(m_rar, this)
        << false
        << false
        << QByteArray();
}

void TestQtRARFile::change()
{
    QFETCH(QString, oldArc);
    QFETCH(QString, oldFileName);
    QFETCH(bool, oldIsOpen);
    QFETCH(QByteArray, oldContent);
    QFETCH(QString, newArc);
    QFETCH(QString, newFileName);
    QFETCH(bool, newIsOpen);
    QFETCH(QByteArray, newContent);

    QtRARFile f(oldArc, oldFileName);
    QCOMPARE(f.open(QIODevice::ReadOnly), oldIsOpen);
    QCOMPARE(f.isOpen(), oldIsOpen);

    if (f.isOpen()) {
        QCOMPARE(f.readAll(), oldContent);
    }
    QCOMPARE(f.pos(), oldContent.size());

    f.close();

    f.setArchiveName(newArc);
    f.setFileName(newFileName);
    QCOMPARE(f.open(QIODevice::ReadOnly), newIsOpen);
    QCOMPARE(f.isOpen(), newIsOpen);
    QCOMPARE(f.pos(), 0);

    if (f.isOpen()) {
        QCOMPARE(f.readAll(), newContent);
        QCOMPARE(f.pos(), newContent.size());
    }
}

void TestQtRARFile::change_data()
{
    QTest::addColumn<QString>("oldArc");
    QTest::addColumn<QString>("oldFileName");
    QTest::addColumn<bool>("oldIsOpen");
    QTest::addColumn<QByteArray>("oldContent");
    QTest::addColumn<QString>("newArc");
    QTest::addColumn<QString>("newFileName");
    QTest::addColumn<bool>("newIsOpen");
    QTest::addColumn<QByteArray>("newContent");

    QTest::newRow("same archive, file name change")
        << "multiple.rar"
        << "qt.txt"
        << true
        << QByteArray("rar\n")
        << "multiple.rar"
        << "qt2.txt"
        << true
        << QByteArray("rar2\n");
    QTest::newRow("different archive")
        << "multiple.rar"
        << "qt2.txt"
        << true
        << QByteArray("rar2\n")
        << "single.rar"
        << "qt.txt"
        << true
        << QByteArray("rar\n");
    QTest::newRow("found -> not found")
        << "multiple.rar"
        << "qt.txt"
        << true
        << QByteArray("rar\n")
        << "single.rar"
        << "QT.txt"
        << false
        << QByteArray();
    QTest::newRow("not found -> found")
        << "multiple.rar"
        << "QT.txt"
        << false
        << QByteArray("")
        << "single.rar"
        << "qt.txt"
        << true
        << QByteArray("rar\n");
}
