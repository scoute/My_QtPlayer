#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>


/*
    ПОЛНОЕ ОПИСАНИЕ ПРОГРАММЫ.

    Программа задумана с целью реализовать возможность играть не весь mp3 файл,
    а какую-то его часть, называемую audio-TAG.

    Для этого за основу взят стандартный Qt класс QMediaPlayer и реализован функционал
    как полного прослушивания мелодии, так и возможность указать её часть и пометить
    через audio-TAG (временная метка).

    Границы этой метки можно изменить 2мя способами:
    1) кнопками "set_current_time_as BEGIN/END", когда время берётся из текущего
       положения ползунка проигрывателя.
    2) через самостоятельное передвижение ползунков начала и конца audio-TAG.

    Надо сказать, что 2 ползунка для audio-TAG сделаны от безысходности )) потому что
    реализация ползунка(слайдера) с двумя маркерами - слишком сложная задача для новичка,
    пришлось как-то выкручиваться.
*/



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


// эта подпрограмма вызывается из другой подпрограммы (MainWindow::positionChanged)
// Честно говоря не совсем понятно, почему её создали отдельно в уроке на ютубе,
// возможно в этом нет никакого смысла.
void MainWindow::updateduration(qint64 duration)
{
    // этот блок останавливает проигрывание аудио-метки, если достигнут её конец.
    // Срабатывает ТОЛЬКО при проигрывании метки, а не основного аудио-файла.
    if (is_playing_TAG==true){
        if (M_Player->position() >= duration_tag_end){
            is_playing_TAG=false;
            M_Player->pause();
        }
    }

    if (duration || Mduration) // не знаю точно зачем это, но если убрать, программа иногда падает)) подсмотрел на ютубе.
    {
        // новый способ конвертации int->text, через функцию timeToString.
        ui->label_CurrTime->setText(MainWindow::timeToString(duration)); // + " / " + totalTime.toString(format));
        ui->label_Total_Time->setText(MainWindow::timeToString(Mduration));
    }
}


// эта подпрограмма срабатывает при "изменении длительности мелодии", а по сути при смене мелодии.
void MainWindow::durationChanged(qint64 duration)
{
    Mduration = duration; // запоминаем её максимальную продолжительность

    // максимальное значение диапазона всех ползунков ставим на эту максимальную продолжительность.
    ui->hSlider_AudioFileDuration->setMaximum(Mduration);
    ui->hSlider_TagTimeBegin->setMaximum(Mduration);
    ui->hSlider_TagTimeEnd->setMaximum(Mduration);

    duration_tag_begin=0; // начало временной метки ставим "ноль"
    ui->label_TAGBeginTime->setText(MainWindow::timeToString(0));

    duration_tag_end=Mduration; // конец временной метки = конец мелодии (первая инициализация)
    ui->label_TAGEndTime->setText(MainWindow::timeToString(Mduration));

    // в первый раз нет смысла вычислять через функцию, длина TAG равна максимальной длительности мелодии.
    ui->label_TAG_Duration->setText(ui->label_TAGEndTime->text());
}


// эта подпрограмма срабатывает, когда обновляется (невидимая) позиция проигрывателя (примерно каждую секунду),
// а дальше она вызывает другие подпрограммы, чтобы перерисовать UI.
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


// эта подпрограмма открывает аудио-файл для проигрывания.
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


// при нажатии на кнопку Play
void MainWindow::on_pushButton_Play_clicked()
{
    is_playing_TAG=false; // выходим из режима проигрывания метки, чтобы двигаться за её пределы.
    M_Player->play();
}


// при нажатии на кнопку Pause
void MainWindow::on_pushButton_Pause_clicked()
{
    // попытка второе нажатие Pause сделать как Play, но пока безуспешно.

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


// задаём громкость мышкой
void MainWindow::on_hSlider_VolumeControl_valueChanged(int value)
{
    M_Player->setVolume(value);
}


// при изменении ползунка длительности мелодии (ещё до отпускания мышки),
// отображаем время на лейбе label_CurrTime, так мы понимаем "где мы".
void MainWindow::on_hSlider_AudioFileDuration_sliderMoved(int position)
{
    ui->label_CurrTime->setText(MainWindow::timeToString(position));
}


// а вот когда мышку отпустили, тогда M_Player перескакивает на новую позицию.
void MainWindow::on_hSlider_AudioFileDuration_sliderReleased()
{
    M_Player->setPosition(ui->hSlider_AudioFileDuration->value());
}


// при изменении значения ползунка проверяем, не вышел ли он за границы своего "напарника".
// Если да, то срабатывает "предохранитель", то есть уравниваем duration_tag_begin и duration_tag_end,
// (это невидимая часть для пользователя), а визуально, чтобы ползунки не пересекали друг друга,
// это разруливают подпрограммы TagTimeBegin_sliderReleased и TagTimeEnd_sliderReleased
void MainWindow::on_hSlider_TagTimeBegin_sliderMoved(int position)
{
    if (position <= duration_tag_end) {
        duration_tag_begin=position;
        ui->label_TAGBeginTime->setText(MainWindow::timeToString(duration_tag_begin));
        ui->label_TAG_Duration->setText(MainWindow::timeToString(duration_tag_end - duration_tag_begin));
    }
}


// что происходит, когда пользователь "отпускает мышку" при изменении ползунка TagTimeBegin
void MainWindow::on_hSlider_TagTimeBegin_sliderReleased()
{
    // если ползунок выходит за границы своего "напарника"-ползунка,
    // то при "отпускании мышки" их положения уравниваются,
    // а визуально это выглядит так, будто ползунок отпрыгнул "назад".
    // Это нужно чтобы предотвратить отрицательное значение audio-TAG.
    if (ui->hSlider_TagTimeBegin->value() > duration_tag_end) {
        ui->hSlider_TagTimeBegin->setValue(duration_tag_end);
    }

    //qDebug() << "hSlider_TagTimeBegin" << ui->hSlider_TagTimeBegin->value();

    /*
    qDebug() << "duration_tag_begin\t" << duration_tag_begin;
    qDebug() << "duration_tag_end\t" << duration_tag_end;
    qDebug() << "Mduration\t\t" << Mduration;
    */
}


// при изменении значения ползунка проверяем, не вышел ли он за границы своего "напарника".
// Если да, то срабатывает "предохранитель", то есть уравниваем duration_tag_begin и duration_tag_end,
// (это невидимая часть для пользователя), а визуально, чтобы ползунки не пересекали друг друга,
// это разруливают подпрограммы TagTimeBegin_sliderReleased и TagTimeEnd_sliderReleased
void MainWindow::on_hSlider_TagTimeEnd_sliderMoved(int position)
{
    if (duration_tag_begin <= Mduration - position) {
        duration_tag_end = Mduration - position;

        ui->label_TAGEndTime->setText(MainWindow::timeToString(duration_tag_end));
        ui->label_TAG_Duration->setText(MainWindow::timeToString(duration_tag_end - duration_tag_begin));
    }
}


// что происходит, когда пользователь "отпускает мышку" при изменении ползунка TagTimeEnd
void MainWindow::on_hSlider_TagTimeEnd_sliderReleased()
{
    // если ползунок выходит за границы своего "напарника"-ползунка,
    // то при "отпускании мышки" их положения уравниваются,
    // а визуально это выглядит так, будто ползунок отпрыгнул "назад".
    // Это нужно чтобы предотвратить отрицательное значение audio-TAG.
    if ((Mduration - ui->hSlider_TagTimeEnd->value()) < duration_tag_begin) {
        ui->hSlider_TagTimeEnd->setValue(Mduration - duration_tag_end);
    }

    // qDebug() << "hSlider_TagTimeEnd" << Mduration - ui->hSlider_TagTimeEnd->value();

}


// при нажатии на кнопку (TAG)BEGIN, копируем текущее время мелодии (M_Player->position)
// и положение ползунка с hSlider_AudioFileDuration.
//
// Также не даём возможности создать TAG отрицательной длины,
// когда начало(duration_tag_begin) больше конца(duration_tag_end), или наоборот.
void MainWindow::on_pushButton_TAGSetBeginTime_clicked()
{
    duration_tag_begin=M_Player->position();

    ui->label_TAGBeginTime->setText(MainWindow::timeToString(duration_tag_begin));
    ui->hSlider_TagTimeBegin->setValue(ui->hSlider_AudioFileDuration->value());

    // если при нажатии кнопки (TAG)BEGIN один из ползунков вышел за границу другого,
    // то второй ползунок смещается туда же, время начала и конца TAG labels становится одинаковым,
    // а длительность TAG становится 00:00:00 (естественно)
    if (duration_tag_begin > duration_tag_end){
        duration_tag_end = duration_tag_begin;
        ui->hSlider_TagTimeEnd->setValue(Mduration - ui->hSlider_TagTimeBegin->value());
        ui->label_TAGEndTime->setText(ui->label_TAGBeginTime->text());
        ui->label_TAG_Duration->setText(MainWindow::timeToString(0)); // длительность TAG становится 00:00:00
    } else {
        ui->label_TAG_Duration->setText(MainWindow::timeToString(duration_tag_end - duration_tag_begin));
    }

}


// при нажатии на кнопку (TAG)END, копируем текущее время мелодии (M_Player->position)
// и положение ползунка с hSlider_AudioFileDuration.
//
// Также не даём возможности создать TAG отрицательной длины,
// когда начало(duration_tag_begin) больше конца(duration_tag_end), или наоборот.
void MainWindow::on_pushButton_TAGSetEndTime_clicked()
{
    duration_tag_end=M_Player->position();

    ui->label_TAGEndTime->setText(MainWindow::timeToString(duration_tag_end));
    ui->hSlider_TagTimeEnd->setValue(Mduration - ui->hSlider_AudioFileDuration->value());

    // если при нажатии кнопки (TAG)END один из ползунков вышел за границу другого,
    // то второй ползунок смещается туда же, время начала и конца TAG labels становится одинаковым,
    // а длительность TAG становится 00:00:00 (естественно)
    if (duration_tag_end < duration_tag_begin){
        duration_tag_begin = duration_tag_end;
        ui->hSlider_TagTimeBegin->setValue(Mduration - ui->hSlider_TagTimeEnd->value());
        ui->label_TAGBeginTime->setText(ui->label_TAGEndTime->text());
        ui->label_TAG_Duration->setText(MainWindow::timeToString(0)); // длительность TAG становится 00:00:00
    } else {
        ui->label_TAG_Duration->setText(MainWindow::timeToString(duration_tag_end - duration_tag_begin));
    }
}


// если нажимаем на кнопку TagPlay, то переходим в режим проигрывания метки (is_playing_TAG=true)
// и перемещаем позицию основного проигрывателя на начало метки.
//
// Это создаёт впечатление, что проигрывается метка, но визуально это происходит
// в рамках основного проигрывателя.
void MainWindow::on_pushButton_TagPlay_clicked()
{
    is_playing_TAG=true;
    M_Player->setPosition(duration_tag_begin);
    M_Player->play();
}
