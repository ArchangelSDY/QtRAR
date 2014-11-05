#ifndef TESTQTRAR_H
#define TESTQTRAR_H

#include <QObject>

class TestQtRAR : public QObject
{
    Q_OBJECT
private slots:
    void entriesCount();
    void entriesCount_data();
};

#endif // TESTQTRAR_H
