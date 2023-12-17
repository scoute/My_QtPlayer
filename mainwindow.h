#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QtMultimedia>
#include <QFileDialog>
#include <QStyle>


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

  void on_hSlider_TagTimeBegin_valueChanged(int value);

private:
  void updateduration(qint64 duration);
  Ui::MainWindow *ui;
  QMediaPlayer *M_Player;
  qint64 Mduration;
  QString timeToString(qint64 duration);
  qint64 duration_tag_begin;
  qint64 duration_tag_end;
};
#endif // MAINWINDOW_H
