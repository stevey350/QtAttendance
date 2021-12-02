#include "widget.h"
#include "ui_widget.h"
#include <QtGui>
#include <QDateTime>
#include <QDebug>
#include <QSqlQueryModel>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QFileDialog>
#include <QDesktopServices>
#include <QAxObject>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    ui->stackedWidget->setCurrentIndex(0);      // 使显示第0个界面，即“人员信息管理”界面

    // 人员信息管理
    model_stu_manage = new QSqlQueryModel(this);
    model_stu_manage->setQuery("select * from student");
    model_stu_manage->setHeaderData(0, Qt::Horizontal, tr("ID"));
    model_stu_manage->setHeaderData(1, Qt::Horizontal, tr("姓名"));
    model_stu_manage->setHeaderData(2, Qt::Horizontal, tr("学号"));
    model_stu_manage->setHeaderData(3, Qt::Horizontal, tr("性别"));

    ui->tableView->setModel(model_stu_manage);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); // 所有列都扩展自适应宽度，填充充满整个屏幕宽度

    // 考勤登记
    model_class_record = new QSqlQueryModel(this);
//    model_class_record->setQuery("select * from record");
    model_class_record->setQuery("select A.name, B.time_checkin, B.time_checkout, B.num_checkin, B.num_checkout, B.time_come, B.time_leave, B.address \
                                    from student A right join record B on A.id=B.id");
    model_class_record->setHeaderData(0, Qt::Horizontal, tr("姓名"));
    model_class_record->setHeaderData(1, Qt::Horizontal, tr("进教室时间"));
    model_class_record->setHeaderData(2, Qt::Horizontal, tr("出教室时间"));
    model_class_record->setHeaderData(3, Qt::Horizontal, tr("进教室次数"));
    model_class_record->setHeaderData(4, Qt::Horizontal, tr("出教室次数"));
    model_class_record->setHeaderData(5, Qt::Horizontal, tr("上课时间"));
    model_class_record->setHeaderData(6, Qt::Horizontal, tr("下课时间"));
    model_class_record->setHeaderData(7, Qt::Horizontal, tr("地点"));

    ui->tableView_record->setModel(model_class_record);
    ui->tableView_record->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // 考勤报表
    QStringList labels;
    labels << "姓名" << "姓别" << "学号" << "进教室时间" << "出教室时间" << "上课时间" << "下课时间" << "进教室次数" << "出教室次数" << "地点" << "考勤状态";
    ui->tableWidget_report->setColumnCount(11);
    ui->tableWidget_report->setHorizontalHeaderLabels(labels);
    ui->tableWidget_report->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    ui->progressBar->hide();  // 隐藏进度条
}

Widget::~Widget()
{
    delete ui;
}

// 获取当前的日期或者时间
QString Widget::getDateTime(Widget::DateTimeType type)
{
    QDateTime datetime = QDateTime::currentDateTime();
    QString date = datetime.toString("yyyy-MM-dd");
    QString time = datetime.toString("hh:mm:ss");
    QString dateAndTime = datetime.toString("yyyy-MM-dd dddd hh:mm");
    if(type == Date)
        return date;
    else if(type == Time)
        return time;
    else
        return dateAndTime;
}

// ------------------------------------界面切换----------------------------------
void Widget::on_btn_stu_manage_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void Widget::on_btn_record_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
}

void Widget::on_btn_report_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);
}

// ------------------------------------人员管理----------------------------------
void Widget::on_btn_add_clicked()
{
    QString id, name, num, sex;

    id = ui->ldt_id->text();
    name = ui->ldt_name->text();
    num = ui->ldt_num->text();
    if(ui->cmb_sex->currentIndex() == 0) {
        sex = "male";
    }
    else {
        sex = "female";
    }

    model_stu_manage->setQuery(QString("insert into student values(%1, '%2', '%3', '%4')").arg(id).arg(name).arg(num).arg(sex));
    model_stu_manage->setQuery("select * from student");
    if (model_class_record->lastError().isValid())      // 执行SQL语句有问题时，将错误输出 (调试用)
        qDebug() << model_class_record->lastError();
}

void Widget::on_btn_delete_clicked()
{
    QString id;

    id = ui->ldt_id->text();
    if(id.isNull()) {
        QMessageBox::critical(this, tr("错误"), tr("请输入待删除记录对应的ID"), QMessageBox::Ok);
    }

    model_stu_manage->setQuery(QString("delete from student where id = %1").arg(id));
    model_stu_manage->setQuery("select * from student");
}

// ------------------------------------考勤登记----------------------------------
// ID输入框有输入回车时，会自动跳转到该函数执行
void Widget::on_ldt_id_for_record_returnPressed()
{
    QString id, cur_time, time_come, time_leave, address;  // ID 刷卡时间（当前） 上课时间   下课时间      地点

    // 1-获取界面输入框内的内容
    id = ui->ldt_id_for_record->text();
    cur_time = getDateTime(Time);
    time_come = ui->ldt_time_come->text();
    time_leave = ui->ldt_time_leave->text();
    address = ui->ldt_address->text();

    // 2-查询该学生是否有记录（有记录说明已经进教室)
    QSqlQuery query;
    query.exec(QString("select * from record where id=%1").arg(id));
    if(0 == query.size()) {
        // 记录个数为0，此种情况为进教室
        // 执行插入新记录的SQL语句 (第二列进教室时间为当前值，第三列出教室时间为默认值空）
        model_class_record->setQuery(QString("insert into record values(%1, '%2', default, 1, 0, '%3', '%4', '%5')")\
                                   .arg(id).arg(cur_time).arg(time_come).arg(time_leave).arg(address));
    }
    else {
        // 执行更新记录的SQL语句 (将第三列出教室时间修改为当前时间）
        query.next();       // QSqlQuery返回的数据集，record是停在第一条记录之前的。所以，你获得数据集后，必须执行next()或first()到第一条记录，这时候record才是有效的。
        int num_checkin, num_checkout;
        num_checkin = query.value(3).toInt();
        num_checkout = query.value(4).toInt();
        if(num_checkin > num_checkout ) {
            model_class_record->setQuery(QString("update record set time_checkout='%1', num_checkout=%2 where id=%3").arg(cur_time).arg(num_checkout+1).arg(id));
        }
        else {
            model_class_record->setQuery(QString("update record set time_checkin='%1', num_checkin=%2 where id=%3").arg(cur_time).arg(num_checkin+1).arg(id));
        }


    }

    // 3-重新查询record表的记录，便于将结果显示出来
    // model_class_record->setQuery("select * from record");
    model_class_record->setQuery("select A.name, B.time_checkin, B.time_checkout, B.num_checkin, B.num_checkout, B.time_come, B.time_leave, B.address \
                                    from student A right join record B on A.id=B.id");

    // 4-清空ID输入框内的内容
    ui->ldt_id_for_record->clear();
}


void Widget::on_btn_clear_record_clicked()
{
    model_class_record->setQuery(QString("delete from record")); // 删除record表中所有数据
    model_class_record->setQuery("select * from record");        // 重新查询显示
}


// ------------------------------------考勤报告----------------------------------
void Widget::on_btn_report_generate_clicked()
{
    // 姓名  性别 学号 进教室时间 出教室时间 上课时间 下课时间  地点 状态
    QSqlQuery query;
    query.exec(QString("select A.name, A.sex, A.num, B.time_checkin, B.time_checkout, B.time_come, B.time_leave, B.num_checkin, B.num_checkout, B.address \
                       from student A left join record B on A.id=B.id"));
    if (query.lastError().isValid())      // 执行SQL语句有问题时，将错误输出 (调试用)
            qDebug() << query.lastError();

    int row = 0;
    ui->tableWidget_report->clearContents();        // 清空表格内容
    ui->tableWidget_report->setRowCount(0);         // 使表格行数清为零
    while(query.next()){
        ui->tableWidget_report->insertRow(row);

        QTableWidgetItem *newItem0 = new QTableWidgetItem(query.value(0).toString());
        ui->tableWidget_report->setItem(row, 0, newItem0);

        QTableWidgetItem *newItem1 = new QTableWidgetItem(query.value(1).toString());
        ui->tableWidget_report->setItem(row, 1, newItem1);

        QTableWidgetItem *newItem2 = new QTableWidgetItem(query.value(2).toString());
        ui->tableWidget_report->setItem(row, 2, newItem2);

        QTableWidgetItem *newItem3 = new QTableWidgetItem(query.value(3).toString());
        ui->tableWidget_report->setItem(row, 3, newItem3);

        QTableWidgetItem *newItem4 = new QTableWidgetItem(query.value(4).toString());
        ui->tableWidget_report->setItem(row, 4, newItem4);

        QTableWidgetItem *newItem5 = new QTableWidgetItem(query.value(5).toString());
        ui->tableWidget_report->setItem(row, 5, newItem5);

        QTableWidgetItem *newItem6 = new QTableWidgetItem(query.value(6).toString());
        ui->tableWidget_report->setItem(row, 6, newItem6);

        QTableWidgetItem *newItem7 = new QTableWidgetItem(query.value(7).toString());
        ui->tableWidget_report->setItem(row, 7, newItem7);

        QTableWidgetItem *newItem8 = new QTableWidgetItem(query.value(8).toString());
        ui->tableWidget_report->setItem(row, 8, newItem8);

        QTableWidgetItem *newItem9 = new QTableWidgetItem(query.value(9).toString());
        ui->tableWidget_report->setItem(row, 9, newItem9);

        // 生成考勤状态
        if(query.value(3).isNull()) {
            QTableWidgetItem *newItem10 = new QTableWidgetItem("旷课");
            ui->tableWidget_report->setItem(row, 10, newItem10);
        }
        else if(query.value(3)>query.value(5)) {
            if(query.value(4)<query.value(6)) {
                QTableWidgetItem *newItem10 = new QTableWidgetItem("迟到&早退");
                ui->tableWidget_report->setItem(row, 10, newItem10);
            }
            else {
                QTableWidgetItem *newItem10 = new QTableWidgetItem("迟到");
                ui->tableWidget_report->setItem(row, 10, newItem10);
            }
        }
        else if(query.value(4)<query.value(6)) {
            QTableWidgetItem *newItem10 = new QTableWidgetItem("早退");
            ui->tableWidget_report->setItem(row, 10, newItem10);
        }
        else {
            QTableWidgetItem *newItem10 = new QTableWidgetItem("正常");
            ui->tableWidget_report->setItem(row, 10, newItem10);
        }

        row++;
    }
}

void Widget::on_btn_report_save_clicked()
{
    ui->progressBar->show();        //进度条需要在ui文件中加个progressBar控件
    ui->progressBar->setValue(0);   //设置进度条的值为0

    QString fileName = QFileDialog::getSaveFileName(this,tr("Excle file"),QString("./考勤表.xlsx"),tr("Excel Files(*.xlsx)"));    //设置保存的文件名
    if(fileName != "")
    {
         QAxObject *excel = new QAxObject;
         if(excel->setControl("Excel.Application"))
         {
             excel->dynamicCall("SetVisible (bool Visible)",false);
             excel->setProperty("DisplayAlerts",false);
             QAxObject *workbooks = excel->querySubObject("WorkBooks");            //获取工作簿集合
             workbooks->dynamicCall("Add");                                        //新建一个工作簿
             QAxObject *workbook = excel->querySubObject("ActiveWorkBook");        //获取当前工作簿
             QAxObject *worksheet = workbook->querySubObject("Worksheets(int)", 1);
             QAxObject *cell;


             /*添加Excel表头数据*/
             for(int i = 1; i <= ui->tableWidget_report->columnCount(); i++)
             {
                 cell=worksheet->querySubObject("Cells(int,int)", 1, i);
                 cell->setProperty("RowHeight", 40);
                 cell->dynamicCall("SetValue(const QString&)", ui->tableWidget_report->horizontalHeaderItem(i-1)->data(0).toString());
                 if(ui->progressBar->value()<=50) {
                     ui->progressBar->setValue(10+i*5);
                 }
             }


             /*将form列表中的数据依此保存到Excel文件中*/
             for(int j = 2; j<=ui->tableWidget_report->rowCount()+1;j++)
             {
                 for(int k = 1;k<=ui->tableWidget_report->columnCount();k++)
                 {
                     cell=worksheet->querySubObject("Cells(int,int)", j, k);
                     cell->dynamicCall("SetValue(const QString&)",ui->tableWidget_report->item(j-2,k-1)->text()+ "\t");
                 }
                 if(ui->progressBar->value() < 80) {
                     ui->progressBar->setValue(50+j*5);
                 }
             }


             /*将生成的Excel文件保存到指定目录下*/
             workbook->dynamicCall("SaveAs(const QString&)",QDir::toNativeSeparators(fileName)); //保存至fileName
             workbook->dynamicCall("Close()");                                                   //关闭工作簿
             excel->dynamicCall("Quit()");                                                       //关闭excel
             delete excel;
             excel=NULL;

             ui->progressBar->setValue(100);
             if (QMessageBox::question(NULL,QString::fromUtf8("完成"),QString::fromUtf8("文件已经导出，是否现在打开？"),QMessageBox::Yes|QMessageBox::No)==QMessageBox::Yes)
             {
                 QDesktopServices::openUrl(QUrl("file:///" + QDir::toNativeSeparators(fileName)));
             }
             ui->progressBar->setValue(0);
             ui->progressBar->hide();
         }
    }
}
