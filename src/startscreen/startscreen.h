#ifndef STARTSCREEN_H
#define STARTSCREEN_H

#include<QtWidgets>
#include<QtSql>

class StartScreen : public QWidget {
public:
    StartScreen(QString version, QString copyright, QString fileVersion, QWidget *parent = nullptr);
    void openFile(QString fileName, QString version, QString copyright, QString fileVersion);
private:
    const QString FILENAME_FILTER = "zfr Files (*.zfr)";
    static QList<QString> getCreateFileQueries(QString fileVersion);
    static QString getCreateTableQuery(QString tableName, QStringList fields, QStringList primaryKeys);
    static QString getCreateItemListTableQuery(QString tableName, QString itemTable, QString foreignItemTable);
    static QString getCreateItemSpecificNumberTableQuery(QString tableName, QString itemTable, QString foreignItemTable, QString valueType);
    static QString getCreateItemSpecificItemTableQuery(QString tableName, QString itemTable, QString foreignItemTable, QString valueItemTable);
    static QString getCreateItemSpecificItemListTableQuery(QString tableName, QString itemTable, QString foreignItemTable, QString valueItemTable);
    static QString getCreateIntegerSpecificItemTableQuery(QString tableName, QString itemTable, QString valueItemTable);
    static QString getCreateIntegerSpecificItemListTableQuery(QString tableName, QString itemTable, QString valueItemTable);
    static QString getCreateIntegerSpecificNumberTableQuery(QString tableName, QString itemTable, QString valueType);
    static QString getCreateItemAndIntegerSpecificNumberTableQuery(QString tableName, QString itemTable, QString foreignItemTable, QString valueType);
};

#endif // STARTSCREEN_H
