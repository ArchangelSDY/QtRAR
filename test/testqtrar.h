#ifndef TESTQTRAR_H
#define TESTQTRAR_H

#include <QObject>

class TestQtRAR : public QObject
{
    Q_OBJECT
private slots:
    void openClose();
    void openClose_data();
    void error();
    void error_data();
    void archiveName();
    void archiveName_data();
    void setArchiveName();
    void setArchiveName_data();
    void entriesCount();
    void entriesCount_data();
    void comment();
    void comment_data();
    void currentFileInfo();
    void currentFileInfo_data();
    void setCurrentFile();
    void setCurrentFile_data();
    void fileNameList();
    void fileNameList_data();
};

#endif // TESTQTRAR_H
