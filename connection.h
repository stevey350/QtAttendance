#ifndef CONNECTION_H
#define CONNECTION_H

#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlQuery>

static bool createConnection()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL"); //, "connection1"
    db.setHostName("localhost");
    db.setDatabaseName("attendance");
    db.setUserName("root");
    db.setPassword("root");
    if (!db.open())
    {
        QMessageBox::critical(0, "Cannot open database1", "Unable to establish a database connection.", QMessageBox::Cancel);
        return false;
    }
//    QSqlQuery query;

//    // 创建student表
//    query.exec("create table student (id int primary key, name varchar, course int)");
//    query.exec("insert into student values(1, 'liqiang', 11)");
//    query.exec("insert into student values(2, 'madeng', 11)");
//    query.exec("insert into student values(3, 'sunhong', 12)");

//    // 创建course表
//    query.exec("create table course (id int primary key, name varchar, teacher varchar)");
//    query.exec("insert into course values(10, 'math', 'wang')");
//    query.exec("insert into course values(11, 'english', 'zhang')");
//    query.exec("insert into course values(12, 'computer', 'bai')");

    return true;
}

#endif // CONNECTION_H
