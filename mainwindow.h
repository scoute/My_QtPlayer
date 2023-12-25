#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>

#include <QtMultimedia>
#include <QFileDialog>
#include <QStyle>

// для БД
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlTableModel>

#include <QSqlRelationalDelegate>


// для отображения дерева папок
#include <QFile>
#include <QSaveFile>
#include <QFileDialog>
#include <QFileSystemModel>


#include <QDataWidgetMapper>


//#include <QTextStream>
//#include <QMessageBox>

//#if defined(QT_PRINTSUPPORT_LIB)
//#include <QtPrintSupport/qtprintsupportglobal.h>
//#if QT_CONFIG(printer)
//#if QT_CONFIG(printdialog)
//#include <QPrintDialog>
//#endif // QT_CONFIG(printdialog)
//#include <QPrinter>
//#endif // QT_CONFIG(printer)
//#endif // QT_PRINTSUPPORT_LIB

//#include <QFont>
//#include <QFontDialog>



QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

private slots:
  void durationChanged(qint64 duration);
  void positionChanged(qint64 progress);

  void on_actionOpen_File_triggered();

  void on_pushButton_Play_clicked();
  void on_pushButton_Pause_clicked();

  void on_hSlider_VolumeControl_valueChanged(int value);
  void on_hSlider_AudioFileDuration_sliderMoved(int position);
  void on_hSlider_AudioFileDuration_sliderReleased();

  void on_hSlider_TagTimeBegin_sliderMoved(int position);
  void on_hSlider_TagTimeBegin_sliderReleased();
  void on_hSlider_TagTimeEnd_sliderMoved(int position);
  void on_hSlider_TagTimeEnd_sliderReleased();

  void on_pushButton_TAGSetBeginTime_clicked();
  void on_pushButton_TAGSetEndTime_clicked();

  void on_pushButton_TagPlay_clicked();

  void on_actionExit_triggered();

  void on_treeView_DirTree_clicked(const QModelIndex &index);

  void on_treeView_DirTree_doubleClicked(const QModelIndex &index);

  void on_pushButton_TAGSave_clicked();

  void on_PushButton_TAGDel_clicked();

  void on_tableView_Sqlite_clicked(const QModelIndex &index);

private:
  void updateduration(qint64 duration);
  Ui::MainWindow *ui;
  QMediaPlayer *M_Player;
  qint64 Mduration;
  QString timeToString(qint64 duration);
  qint64 duration_tag_begin;
  qint64 duration_tag_end;
  bool is_playing_TAG;

  QSqlDatabase db;
  QSqlQuery *query;
  QSqlTableModel *model;
  //QSqlQueryModel *model;

  int row; // выделенная строка в SQL редакторе Тегов

  QDataWidgetMapper *mapper = new QDataWidgetMapper(this);

};
#endif // MAINWINDOW_H
