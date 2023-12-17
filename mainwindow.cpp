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
}


MainWindow::~MainWindow()
{
    delete ui;
}


// эта функция переводит время текущей позиции мелодии из qint64 в текст,
// для того чтобы показывать на всяких label в формате 00:00:00.000
QString MainWindow::timeToString(qint64 duration){
    if (duration||Mduration)
    {
        QTime   durTime((duration / 3600 / 1000) % 60, (duration / 60 / 1000) % 60, (duration / 1000) % 60, duration % 1000);
        QString format = "mm:ss.zzz"; // zzz - значит миллисекунды

        // если длительность аудио больше часа, ставим "hh:mm:ss.zzz" вместо "mm:ss.zzz"
        if ((duration / 1000) > 3600) // делим на 1000 потому что это миллисекунды
        {
            format = "hh:mm:ss.zzz";
        }
        return durTime.toString(format);
    }

    return "error";
}


void MainWindow::updateduration(qint64 duration)
{
    //QString timestr;
    if (duration || Mduration)
    {
        // новый способ конвертации int->text
        ui->label_CurrTime->setText(MainWindow::timeToString(duration)); // + " / " + totalTime.toString(format));
        ui->label_Total_Time->setText(MainWindow::timeToString(Mduration));
    }
}


void MainWindow::durationChanged(qint64 duration)
{
    //Mduration = duration / 1000;
    Mduration = duration;
    ui->hSlider_AudioFileDuration->setMaximum(Mduration);
    ui->hSlider_TagTimeBegin->setMaximum(Mduration);
    ui->hSlider_TagTimeEnd->setMaximum(Mduration);

    duration_tag_begin=0;
    ui->label_TAGBeginTime->setText(MainWindow::timeToString(0));

    duration_tag_end=Mduration;
    ui->label_TAGEndTime->setText(MainWindow::timeToString(Mduration));

    // в первый раз нет смысла вычислять, разница равна максимальной длительности мелодии.
    ui->label_TAG_Duration->setText(ui->label_TAGEndTime->text());
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
    /*
    qDebug() << "M_Player->PausedState" << M_Player->PausedState;
    if (M_Player->PausedState) {
        M_Player->pause();
    } else {
        M_Player->play();
    }
    */
    M_Player->pause();
}


void MainWindow::on_hSlider_VolumeControl_valueChanged(int value)
{
    M_Player->setVolume(value);
}


void MainWindow::on_hSlider_AudioFileDuration_sliderMoved(int position)
{
    ui->label_CurrTime->setText(MainWindow::timeToString(position));
}


void MainWindow::on_hSlider_AudioFileDuration_sliderReleased()
{
    M_Player->setPosition(ui->hSlider_AudioFileDuration->value());
}


void MainWindow::on_hSlider_TagTimeBegin_sliderMoved(int position)
{
    duration_tag_begin=position;

    ui->label_TAGBeginTime->setText(MainWindow::timeToString(duration_tag_begin));

    ui->label_TAG_Duration->setText(MainWindow::timeToString(duration_tag_end - duration_tag_begin));
}


void MainWindow::on_hSlider_TagTimeBegin_valueChanged(int value)
{

}


void MainWindow::on_hSlider_TagTimeBegin_sliderReleased()
{
    /*
    qDebug() << "duration_tag_begin\t" << duration_tag_begin;
    qDebug() << "duration_tag_end\t" << duration_tag_end;
    qDebug() << "Mduration\t\t" << Mduration;
    */
}


void MainWindow::on_hSlider_TagTimeEnd_sliderMoved(int position)
{
    duration_tag_end = Mduration - position;

    ui->label_TAGEndTime->setText(MainWindow::timeToString(duration_tag_end));
    //ui->label_TAGEndTime->setText(MainWindow::timeToString(duration_tag_end));

    //ui->label_TAG_Duration->setText(MainWindow::timeToString(Mduration - position - duration_tag_begin));
    ui->label_TAG_Duration->setText(MainWindow::timeToString(duration_tag_end - duration_tag_begin));
}


void MainWindow::on_hSlider_TagTimeEnd_sliderReleased()
{
    /*
    qDebug() << "duration_tag_begin\t" << duration_tag_begin;
    qDebug() << "duration_tag_end\t" << duration_tag_end;
    qDebug() << "Mduration\t\t" << Mduration;
    */
}


// при нажатии на кнопку (TAG)BEGIN, копируем текущее время мелодии (M_Player->position)
// и положение ползунка с hSlider_AudioFileDuration.
void MainWindow::on_pushButton_TAGSetBeginTime_clicked()
{
    duration_tag_begin=M_Player->position();

    ui->label_TAGBeginTime->setText(MainWindow::timeToString(duration_tag_begin));
    ui->hSlider_TagTimeBegin->setValue(ui->hSlider_AudioFileDuration->value());

    ui->label_TAG_Duration->setText(MainWindow::timeToString(duration_tag_end - duration_tag_begin));
}


void MainWindow::on_pushButton_TAGSetEndTime_clicked()
{
    //duration_tag_end=Mduration - M_Player->position();
    duration_tag_end=M_Player->position();

    ui->label_TAGEndTime->setText(MainWindow::timeToString(duration_tag_end));
    ui->hSlider_TagTimeEnd->setValue(Mduration - ui->hSlider_AudioFileDuration->value());

    ui->label_TAG_Duration->setText(MainWindow::timeToString(duration_tag_end - duration_tag_begin));
}
