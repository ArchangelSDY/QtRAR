#include <QTest>
#include <QFile>

#include "../src/qtrar.h"
#include "../src/qtrarfileinfo.h"

#include "testqtrar.h"

Q_DECLARE_METATYPE(QtRAR::OpenMode)
Q_DECLARE_METATYPE(Qt::CaseSensitivity)

void TestQtRAR::openClose()
{
    QFETCH(QString, arcName);
    QFETCH(QtRAR::OpenMode, openMode);
    QFETCH(bool, isOpen);

    QtRAR rar(arcName);
    QCOMPARE(rar.mode(), QtRAR::OpenModeNotOpen);

    QCOMPARE(rar.open(openMode), isOpen);
    QCOMPARE(rar.isOpen(), isOpen);
    QCOMPARE(rar.mode(), rar.isOpen() ? openMode : QtRAR::OpenModeNotOpen);

    rar.close();
    QCOMPARE(rar.isOpen(), false);
    QCOMPARE(rar.mode(), QtRAR::OpenModeNotOpen);
}

void TestQtRAR::openClose_data()
{
    QTest::addColumn<QString>("arcName");
    QTest::addColumn<QtRAR::OpenMode>("openMode");
    QTest::addColumn<bool>("isOpen");

    QTest::newRow("single file archive")
        << "single.rar"
        << QtRAR::OpenModeList
        << true;
    QTest::newRow("multiple files archive")
        << "multiple.rar"
        << QtRAR::OpenModeList
        << true;
    QTest::newRow("UTF-8 archive name")
        << "中文.rar"
        << QtRAR::OpenModeList
        << true;
    QTest::newRow("archive not found")
        << "notfound.rar"
        << QtRAR::OpenModeList
        << false;
    QTest::newRow("extract open mode")
        << "single.rar"
        << QtRAR::OpenModeExtract
        << true;
}

void TestQtRAR::error()
{
    QFETCH(QString, arcName);
    QFETCH(bool, errorExists);

    QtRAR rar(arcName);
    rar.open(QtRAR::OpenModeList);
    QCOMPARE(rar.error() != 0, errorExists);
}

void TestQtRAR::error_data()
{
    QTest::addColumn<QString>("arcName");
    QTest::addColumn<bool>("errorExists");

    QTest::newRow("normal archive")
        << "multiple.rar"
        << false;
    QTest::newRow("archive not found")
        << "notfound.rar"
        << true;
}

void TestQtRAR::archiveName()
{
    QFETCH(QString, arcName);

    QtRAR rar(arcName);
    QCOMPARE(rar.archiveName(), arcName);
}

void TestQtRAR::archiveName_data()
{
    QTest::addColumn<QString>("arcName");

    QTest::newRow("single file archive")
        << "single.rar";
    QTest::newRow("multiple files archive")
        << "multiple.rar";
    QTest::newRow("UTF-8 archive name")
        << "中文.rar";
}

void TestQtRAR::setArchiveName()
{
    QFETCH(QString, oldArc);
    QFETCH(bool, oldIsOpen);
    QFETCH(int, oldEntriesCount);
    QFETCH(QString, newArc);
    QFETCH(bool, newIsOpen);
    QFETCH(int, newEntriesCount);

    QtRAR *rar;
    if (oldArc.isNull()) {
        rar = new QtRAR();
    } else {
        rar = new QtRAR(oldArc);
    }

    rar->open(QtRAR::OpenModeList);
    QCOMPARE(rar->isOpen(), oldIsOpen);
    QCOMPARE(rar->entriesCount(), oldEntriesCount);
    QCOMPARE(rar->error() == 0, oldIsOpen);

    rar->close();
    QCOMPARE(rar->isOpen(), false);

    rar->setArchiveName(newArc);
    rar->open(QtRAR::OpenModeList);
    QCOMPARE(rar->isOpen(), newIsOpen);
    QCOMPARE(rar->entriesCount(), newEntriesCount);
    QCOMPARE(rar->error() == 0, newIsOpen);

    delete rar;
}

void TestQtRAR::setArchiveName_data()
{
    QTest::addColumn<QString>("oldArc");
    QTest::addColumn<bool>("oldIsOpen");
    QTest::addColumn<int>("oldEntriesCount");
    QTest::addColumn<QString>("newArc");
    QTest::addColumn<bool>("newIsOpen");
    QTest::addColumn<int>("newEntriesCount");

    QTest::newRow("single -> multiple")
        << "single.rar" << true << 1
        << "multiple.rar" << true << 2;
    QTest::newRow("multiple -> single")
        << "multiple.rar" << true << 2
        << "single.rar" << true << 1;
    QTest::newRow("null -> mulitple")
        << QString() << false << 0
        << "multiple.rar" << true << 2;
    QTest::newRow("multiple -> null")
        << "multiple.rar" << true << 2
        << QString() << false << 0;
    QTest::newRow("not found -> multiple")
        << "notfound.rar" << false << 0
        << "multiple.rar" << true << 2;
    QTest::newRow("multiple -> not found")
        << "multiple.rar" << true << 2
        << "notfound.rar" << false << 0;
}

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
    QTest::newRow("multiple files archive")
        << "multiple.rar"
        << 2;
    QTest::newRow("UTF-8 archive name")
        << "中文.rar"
        << 1;
}

void TestQtRAR::comment()
{
    QFETCH(QString, arcName);
    QFETCH(QString, comment);

    QtRAR rar(arcName);
    rar.open(QtRAR::OpenModeList);
    QCOMPARE(rar.comment(), comment);
}

void TestQtRAR::comment_data()
{
    QTest::addColumn<QString>("arcName");
    QTest::addColumn<QString>("comment");

    QTest::newRow("regular comment")
        << "comment.rar"
        << "QtRAR";
    QTest::newRow("UTF-8 comment")
        << "comment-utf8.rar"
        << "中文";
}

void TestQtRAR::currentFileInfo()
{
    QFETCH(QString, arcName);
    QFETCH(QString, fileName);
    QFETCH(unsigned int, packSize);
    QFETCH(unsigned int, unpSize);
    QFETCH(unsigned int, checksum);

    QtRAR rar(arcName);
    rar.open(QtRAR::OpenModeList);
    QtRARFileInfo info;
    QVERIFY2(rar.currentFileInfo(&info), "cannot get current file info");
    QCOMPARE(info.fileName, fileName);
    QCOMPARE(info.packSize, packSize);
    QCOMPARE(info.unpSize, unpSize);
    QCOMPARE(info.fileCRC, checksum);
}

void TestQtRAR::currentFileInfo_data()
{
    QTest::addColumn<QString>("arcName");
    QTest::addColumn<QString>("fileName");
    QTest::addColumn<unsigned int>("packSize");
    QTest::addColumn<unsigned int>("unpSize");
    QTest::addColumn<unsigned int>("checksum");

    QTest::newRow("single file archive")
        << "single.rar"
        << "qt.txt"
        << 13u
        << 4u
        << QByteArray("54BBE476").toUInt(0, 16);
    QTest::newRow("multiple file archive")
        << "multiple.rar"
        << "qt2.txt"
        << 15u
        << 5u
        << QByteArray("9C7AD585").toUInt(0, 16);
}

void TestQtRAR::setCurrentFile()
{
    QFETCH(QString, arcName);
    QFETCH(QString, fileName);
    QFETCH(Qt::CaseSensitivity, caseSensitive);
    QFETCH(bool, setCurrentFileSuccess);
    QFETCH(unsigned int, packSize);
    QFETCH(unsigned int, unpSize);
    QFETCH(unsigned int, checksum);

    QtRAR rar(arcName);
    rar.open(QtRAR::OpenModeList);

    QCOMPARE(rar.setCurrentFile(fileName, caseSensitive),
             setCurrentFileSuccess);

    if (setCurrentFileSuccess) {
        QCOMPARE(rar.currentFileName().compare(fileName, caseSensitive), 0);

        QtRARFileInfo info;
        QVERIFY2(rar.currentFileInfo(&info), "cannot get current file info");

        QCOMPARE(info.fileName.compare(fileName, caseSensitive), 0);
        QCOMPARE(info.packSize, packSize);
        QCOMPARE(info.unpSize, unpSize);
        QCOMPARE(info.fileCRC, checksum);
    }
}

void TestQtRAR::setCurrentFile_data()
{
    QTest::addColumn<QString>("arcName");
    QTest::addColumn<QString>("fileName");
    QTest::addColumn<Qt::CaseSensitivity>("caseSensitive");
    QTest::addColumn<bool>("setCurrentFileSuccess");
    QTest::addColumn<unsigned int>("packSize");
    QTest::addColumn<unsigned int>("unpSize");
    QTest::addColumn<unsigned int>("checksum");

    QTest::newRow("multiple file archive")
        << "multiple.rar"
        << "qt.txt"
        << Qt::CaseSensitive
        << true
        << 13u
        << 4u
        << QByteArray("54BBE476").toUInt(0, 16);
    QTest::newRow("correct file name - case insensitive")
        << "multiple.rar"
        << "QT.TXT"
        << Qt::CaseInsensitive
        << true
        << 13u
        << 4u
        << QByteArray("54BBE476").toUInt(0, 16);
    QTest::newRow("incorrect file name - case insensitive")
        << "multiple.rar"
        << "QT.TXT"
        << Qt::CaseSensitive
        << false
        << 13u
        << 4u
        << QByteArray("54BBE476").toUInt(0, 16);
}

void TestQtRAR::fileNameList()
{
    QFETCH(QString, arcName);
    QFETCH(QStringList, fileNameList);

    QtRAR rar(arcName);
    rar.open(QtRAR::OpenModeList);

    QCOMPARE(rar.fileNameList(), fileNameList);
}

void TestQtRAR::fileNameList_data()
{
    QTest::addColumn<QString>("arcName");
    QTest::addColumn<QStringList>("fileNameList");

    QTest::newRow("single file archive")
        << "single.rar"
        << (QStringList() << "qt.txt");
    QTest::newRow("multiple files archive")
        << "multiple.rar"
        << (QStringList() << "qt2.txt" << "qt.txt");
    QTest::newRow("multiple files(with utf8 file names) archive")
        << "multiple-with-utf8.rar"
        << (QStringList() << "qt2.txt" << "qt.txt" << "中文.txt");
}
