#ifndef TESTQTRARFILE_H
#define TESTQTRARFILE_H

#include <QObject>

class QtRAR;

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

private:
    QtRAR *m_rar;
};

#endif // TESTQTRARFILE_H
