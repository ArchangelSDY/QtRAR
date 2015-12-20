#include <QImage>
#include <QImageReader>
#include <QTest>

#include "../src/qtrar.h"
#include "../src/qtrarfile.h"
#include "../src/qtrarfileinfo.h"

class TestQtRARFile : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void cleanupTestCase();

    void openAndRead();
    void openAndRead_data();
    void constructor();
    void constructor_data();
    void change();
    void change_data();
    void password();
    void password_data();
    void imageInArchive();
    void imageInArchive_data();

private:
    QtRAR *m_rar;
};

Q_DECLARE_METATYPE(Qt::CaseSensitivity)

void TestQtRARFile::initTestCase()
{
    m_rar = new QtRAR("assets/multiple.rar");
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
        QCOMPARE(f.isSequential(), true);

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
        << "assets/multiple.rar"
        << "qt2.txt"
        << Qt::CaseSensitive
        << true
        << QByteArray("rar2\n")
        << Q_INT64_C(5)
        << Q_INT64_C(15);
    QTest::newRow("second file in archive")
        << "assets/multiple.rar"
        << "qt.txt"
        << Qt::CaseSensitive
        << true
        << QByteArray("rar\n")
        << Q_INT64_C(4)
        << Q_INT64_C(13);
    QTest::newRow("file not found")
        << "assets/multiple.rar"
        << "QT.txt"
        << Qt::CaseSensitive
        << false
        << QByteArray()
        << Q_INT64_C(4)
        << Q_INT64_C(13);
    QTest::newRow("case insensitive")
        << "assets/multiple.rar"
        << "QT.txt"
        << Qt::CaseInsensitive
        << true
        << QByteArray("rar\n")
        << Q_INT64_C(4)
        << Q_INT64_C(13);
    QTest::newRow("UTF-8 content")
        << "assets/multiple-with-utf8.rar"
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
    fInitWithAllNull->setArchiveName("assets/multiple.rar");
    fInitWithAllNull->setFileName("qt.txt");
    QTest::newRow("init with all null")
        << fInitWithAllNull
        << true
        << true
        << QByteArray("rar\n");

    QtRARFile *fInitWithArcNameOnly = new QtRARFile("assets/multiple.rar", this);
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
        << "assets/multiple.rar"
        << "qt.txt"
        << true
        << QByteArray("rar\n")
        << "assets/multiple.rar"
        << "qt2.txt"
        << true
        << QByteArray("rar2\n");
    QTest::newRow("different archive")
        << "assets/multiple.rar"
        << "qt2.txt"
        << true
        << QByteArray("rar2\n")
        << "assets/single.rar"
        << "qt.txt"
        << true
        << QByteArray("rar\n");
    QTest::newRow("found -> not found")
        << "assets/multiple.rar"
        << "qt.txt"
        << true
        << QByteArray("rar\n")
        << "assets/single.rar"
        << "QT.txt"
        << false
        << QByteArray();
    QTest::newRow("not found -> found")
        << "assets/multiple.rar"
        << "QT.txt"
        << false
        << QByteArray("")
        << "assets/single.rar"
        << "qt.txt"
        << true
        << QByteArray("rar\n");
}

void TestQtRARFile::password()
{
    QFETCH(QString, arcName);
    QFETCH(QString, fileName);
    QFETCH(QByteArray, password);
    QFETCH(bool, canOpenWithoutPassword);
    QFETCH(bool, isOpen);
    QFETCH(QByteArray, content);

    QtRARFile f(arcName, fileName);

    // First open without password
    QCOMPARE(f.open(QIODevice::ReadOnly), canOpenWithoutPassword);
    if (canOpenWithoutPassword) {
        f.close();
    }

    // Second open with password
    QCOMPARE(f.open(QIODevice::ReadOnly, password.data()), isOpen);

    QtRARFileInfo info;
    f.fileInfo(&info);
    QCOMPARE(info.isEncrypted(), true);

    QCOMPARE(f.readAll(), content);
}

void TestQtRARFile::password_data()
{
    QTest::addColumn<QString>("arcName");
    QTest::addColumn<QString>("fileName");
    QTest::addColumn<QByteArray>("password");
    QTest::addColumn<bool>("canOpenWithoutPassword");
    QTest::addColumn<bool>("isOpen");
    QTest::addColumn<QByteArray>("content");

    QTest::newRow("valid password")
        << "assets/password.rar"
        << "qt.txt"
        << QByteArray("qt")
        << true
        << true
        << QByteArray("rar\n");
    QTest::newRow("invalid password")
        << "assets/password.rar"
        << "qt.txt"
        << QByteArray("tq")
        << true
        << false
        << QByteArray();
    QTest::newRow("archive with headers encrypted")
        << "assets/password-header.rar"
        << "qt.txt"
        << QByteArray("qt")
        << false
        << true
        << QByteArray("rar\n");
}

void TestQtRARFile::imageInArchive()
{
    QFETCH(QString, arcName);
    QFETCH(QString, fileName);
    QFETCH(QSize, size);

    QtRARFile f(arcName, fileName);
    QVERIFY2(f.open(QtRARFile::ReadOnly), "fail to open archive");

    QImageReader reader(&f);
    QImage image = reader.read();
    QVERIFY2(!image.isNull(), "image is null");
    QCOMPARE(image.size(), size);
}

void TestQtRARFile::imageInArchive_data()
{
    QTest::addColumn<QString>("arcName");
    QTest::addColumn<QString>("fileName");
    QTest::addColumn<QSize>("size");

    QTest::newRow("PNG")
        << "assets/image.rar"
        << "blank.png"
        << QSize(5, 5);
    QTest::newRow("JPG")
        << "assets/image.rar"
        << "blank.jpg"
        << QSize(5, 5);
}

QTEST_MAIN(TestQtRARFile)
#include "qtrarfile_test.moc"