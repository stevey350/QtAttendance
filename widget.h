#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QSqlQueryModel>

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();

    enum DateTimeType{Time, Date, DateTime};
    QString getDateTime(DateTimeType type);

private slots:
    void on_btn_add_clicked();

    void on_btn_delete_clicked();

    void on_btn_stu_manage_clicked();

    void on_btn_record_clicked();

    void on_btn_report_clicked();

    void on_ldt_id_for_record_returnPressed();

    void on_btn_clear_record_clicked();

    void on_btn_report_generate_clicked();

    void on_btn_report_save_clicked();

private:
    Ui::Widget *ui;

    QSqlQueryModel *model_stu_manage;           // 用于显示人员管理的数据模型
    QSqlQueryModel *model_class_record;         // 用于显示考勤登记结果的数据模型
};

#endif // WIDGET_H
