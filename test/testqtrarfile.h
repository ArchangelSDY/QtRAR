#ifndef TESTQTRARFILE_H
#define TESTQTRARFILE_H

#include <QObject>

class TestQtRARFile : public QObject
{
    Q_OBJECT
private slots:
    void openAndRead();
    void openAndRead_data();
};

#endif // TESTQTRARFILE_H
