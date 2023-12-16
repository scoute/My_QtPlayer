#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    M_Player = new QMediaPlayer();
    ui->pushButton_Play->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    ui->pushButton_Pause->setIcon(style()->standardIcon(QStyle::SP_MediaPause));

    ui->hSlider_VolumeControl->setMinimum(1);
    ui->hSlider_VolumeControl->setMaximum(100);
    ui->hSlider_VolumeControl->setValue(10);

    // изначальная громкость 10%
    M_Player->setVolume(10);

    connect(M_Player,&QMediaPlayer::durationChanged,this,&MainWindow::durationChanged);
    connect(M_Player,&QMediaPlayer::positionChanged,this,&MainWindow::positionChanged);

    /*
    ui->hSlider_AudioFileDuration->setRange(0,M_Player->duration() / 1000);
    ui->hSlider_TagTimeBegin->setRange(0,M_Player->duration() / 1000);
    */
    ui->hSlider_AudioFileDuration->setRange(0,M_Player->duration());
    ui->hSlider_TagTimeBegin->setRange(0,M_Player->duration());

    //ui->hSlider_TagTimeBegin->setStyleSheet("QSlider::groove:horizontal {background-color:red;}");
    //ui->hSlider_TagTimeBegin->setStyleSheet("QSlider::groove:horizontal { border: 1px solid #999999; height: 8px; background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #B1B1B1, stop:1 #c4c4c4);  margin: 2px 0; }");

    //ui->hSlider_TagTimeBegin->setMaximumWidth(100);
    //ui->hSlider_TagTimeBegin->setMinimumWidth(200);
}


MainWindow::~MainWindow()
{
    delete ui;
}


QString MainWindow::timeToString(qint64 duration){
    //QString timestr;
    if (duration)
    {
        /*
        QTime CurrentTime((duration / 3600) % 60, (duration / 60) % 60, duration % 60, duration * 1000 % 1000);
        QTime totalTime((Mduration / 3600) % 60, (Mduration / 60) % 60, Mduration % 60, Mduration * 1000 % 1000);
        */

        QTime CurrentTime((duration / 3600 / 1000) % 60, (duration / 60 / 1000) % 60, (duration / 1000) % 60, duration % 1000);
        //QTime totalTime((Mduration / 3600 / 1000) % 60, (Mduration / 60 / 1000) % 60, (Mduration / 1000) % 60, Mduration % 1000);

        //QString format = "mm:ss";
        QString format = "mm:ss.zzz";

        // если длительность аудио больше часа, ставим "hh:mm:ss" вместо "mm:ss"
        if (Mduration > 3600)
        {
            //format = "hh:mm:ss";
            format = "hh:mm:ss.zzz";
        }

        return CurrentTime.toString(format);
    }
}


void MainWindow::updateduration(qint64 duration)
{
    //QString timestr;
    if (duration || Mduration)
    {
        /*
        QTime CurrentTime((duration / 3600) % 60, (duration / 60) % 60, duration % 60, duration * 1000 % 1000);
        QTime totalTime((Mduration / 3600) % 60, (Mduration / 60) % 60, Mduration % 60, Mduration * 1000 % 1000);
        */


        QTime CurrentTime((duration / 3600 / 1000) % 60, (duration / 60 / 1000) % 60, (duration / 1000) % 60, duration % 1000);
        QTime totalTime((Mduration / 3600 / 1000) % 60, (Mduration / 60 / 1000) % 60, (Mduration / 1000) % 60, Mduration % 1000);


        //QString format = "mm:ss";
        QString format = "mm:ss.zzz";

        // если длительность аудио больше часа, ставим "hh:mm:ss" вместо "mm:ss"
        if (Mduration > 3600)
        {
            //format = "hh:mm:ss";
            format = "hh:mm:ss.zzz";
        }

        ui->label_CurrTime->setText(CurrentTime.toString(format)); // + " / " + totalTime.toString(format));
        ui->label_Total_Time->setText(totalTime.toString(format));
    }
}


void MainWindow::durationChanged(qint64 duration)
{
    //Mduration = duration / 1000;
    Mduration = duration;
    ui->hSlider_AudioFileDuration->setMaximum(Mduration);
    ui->hSlider_TagTimeBegin->setMaximum(Mduration);
    ui->hSlider_TagTimeEnd->setMaximum(Mduration);

    qDebug() << "ui->hSlider_AudioFileDuration->maximumWidth" <<
                ui->hSlider_AudioFileDuration->maximumWidth();
}


void MainWindow::positionChanged(qint64 progress)
{
    if (!ui->hSlider_AudioFileDuration->isSliderDown())
    {
      //ui->hSlider_AudioFileDuration->setValue(progress / 1000);
        ui->hSlider_AudioFileDuration->setValue(progress);
    }
    //updateduration(progress / 1000);
    updateduration(progress);
}


void MainWindow::on_actionOpen_File_triggered()
{
    QString FileName = QFileDialog::getOpenFileName(this,tr("Select Audio File"),
                                                         tr("MP3 Files (*.MP3)"));
    M_Player->setMedia(QUrl("file://" + FileName));

    QFileInfo File(FileName);
    ui->lbl_Value_File_Name->setText(File.fileName());

    //M_Player->play();
    MainWindow::on_pushButton_Play_clicked();
}


void MainWindow::on_pushButton_Play_clicked()
{
    M_Player->play();
}


void MainWindow::on_pushButton_Pause_clicked()
{
    M_Player->pause();
}


void MainWindow::on_hSlider_VolumeControl_valueChanged(int value)
{
    M_Player->setVolume(value);
}


void MainWindow::on_hSlider_AudioFileDuration_sliderMoved(int position)
{ 
    QTime CurrentTime((position / 3600 / 1000) % 60, (position / 60 / 1000) % 60, (position / 1000) % 60, position % 1000);

    //QString format = "mm:ss";
    QString format = "mm:ss.zzz";

    // если длительность аудио больше часа, ставим "hh:mm:ss" вместо "mm:ss"
    if (Mduration > 3600)
    {
        //format = "hh:mm:ss";
        format = "hh:mm:ss.zzz";
    }

    //ui->label_CurrTime->setText(CurrentTime.toString(format));
    ui->label_CurrTime->setText(MainWindow::timeToString(position));
}


void MainWindow::on_hSlider_AudioFileDuration_sliderReleased()
{
    //M_Player->setPosition(ui->hSlider_AudioFileDuration->value()*1000);
    M_Player->setPosition(ui->hSlider_AudioFileDuration->value());

    //ui->hSlider_TagTimeBegin->setMaximumWidth(movedposition * 10);
    //ui->hSlider_TagTimeBegin->setMinimumWidth(movedposition * 10);
}


void MainWindow::on_hSlider_TagTimeBegin_sliderMoved(int position)
{

}


void MainWindow::on_hSlider_TagTimeBegin_valueChanged(int value)
{
    //QTime CurrentTime((duration / 3600) % 60, (duration / 60) % 60, duration % 60, duration * 1000 % 1000);
    //QTime totalTime((Mduration / 3600) % 60, (Mduration / 60) % 60, Mduration % 60, Mduration * 1000 % 1000);
    //QString format = "mm:ss";
    //ui->label_TAGBeginTime->setText(CurrentTime.toString(format));
    //ui->label_TAGBeginTime->setText(QString::number( value ).toString("mm:ss"));
}


void MainWindow::on_hSlider_TagTimeBegin_sliderReleased()
{

}


void MainWindow::on_hSlider_TagTimeEnd_sliderMoved(int position)
{

}


void MainWindow::on_hSlider_TagTimeEnd_sliderReleased()
{

}


// при нажатии на кнопку (TAG)BEGIN, копируем время (а по сути текст) с label_CurrTime
// и положение ползунка с hSlider_AudioFileDuration.
void MainWindow::on_pushButton_TAGSetBeginTime_clicked()
{
    ui->label_TAGBeginTime->setText(ui->label_CurrTime->text());
    //ui->label_TAGBeginTime->setText(M_Player->position());

    ui->hSlider_TagTimeBegin->setValue(ui->hSlider_AudioFileDuration->value());
}


void MainWindow::on_pushButton_TAGSetEndTime_clicked()
{
    ui->label_TAGEndTime->setText(ui->label_CurrTime->text());
    ui->hSlider_TagTimeEnd->setValue(Mduration - ui->hSlider_AudioFileDuration->value());
}





