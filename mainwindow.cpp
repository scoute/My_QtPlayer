#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "qsystemdetection.h"



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



    TODO! (начинать с лёгкого, чтобы не выгореть :D )
    * адаптивная верстка.
    * древовидный плейлист из папки с mp3.
    * отдельное окно со списком меток.
    * сохранения аудио-меток в БД sqlite3.
    *
    *
    * сканирование всех mp3 в папке и присвоение им md5sum в БД для исключения дублей.
    * возможность делиться аудио-метками (экспорт/импорт)
    * при импорте чужих меток, применение их для локальных mp3 при совпадении md5sum.
    *
    *
    * генерация mp3-файла из временной метки с возможностью сохранения (если это не очень сложно).
*/



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    // *********** РАЗДЕЛ С БД **************

    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("./sqlite.db");
    if (db.open()){ qDebug("DB Open"); } else { qDebug("DB not Open"); }  // проверка БД

    query = new QSqlQuery(db);
    query->exec("CREATE TABLE AudioFiles(ID int PRIMARY KEY, FILE_PATH TEXT, FILE_MD5 TEXT, TAG_NAME TEXT, TAG_START TEXT, TAG_FINISH TEXT, TAG_DUR TEXT);");

    //qDebug() << "In Table: " << query->exec("SELECT * FROM AudioFiles;");


    //    Таблица AudioFiles:
    //    * ID (int) (с уникальным ключом)
    //    * FILE_PATH (text)
    //    * FILE_MD5 (text) хеш-сумма этого файла
    //    * TAG_NAME (text) имя тэга
    //    * TAG_START (text)
    //    * TAG_FINISH (text)
    //    * TAG_DUR (text)


    model = new QSqlTableModel(this, db);
    model->setTable("AudioFiles");
    model->select();
    ui->tableView_Sqlite->setModel(model);


    //    model = new QSqlQueryModel(this);
    //    model->setQuery(QString("SELECT * FROM AudioFiles"), db);

    //    QSortFilterProxyModel *proxyModel;
    //    proxyModel = new QSortFilterProxyModel(this);
    //    proxyModel->setSourceModel(model);
    //    ui->tableView_Sqlite->setModel(proxyModel);




    //    model = new QSqlQueryModel(this);
    //    model->setQuery(QString("SELECT * FROM tablexyz), db);
    //    proxyModel = new QSortFilterProxyModel(this);
    //    proxyModel->setSourceModel(model);
    //    ui->tableView->setModel(proxyModel);






    // раздел с Плеером
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

    ui->hSlider_AudioFileDuration->setRange(0,M_Player->duration());
    ui->hSlider_TagTimeBegin->setRange(0,M_Player->duration());

    //ui->hSlider_TagTimeBegin->setStyleSheet("QSlider::groove:horizontal {background-color:red;}");
    //ui->hSlider_TagTimeBegin->setStyleSheet("QSlider::groove:horizontal { border: 1px solid #999999; height: 8px; background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #B1B1B1, stop:1 #c4c4c4);  margin: 2px 0; }");


    // это окно с деревом папок (слева вверху)
    QFileSystemModel *dirModel = new QFileSystemModel(); //Create new model
    //dirModel->setRootPath("./"); //Set model path
    dirModel->setRootPath(QDir::currentPath());
    //dirModel->setFilter(QDir::Files); //Only show files
    //dirModel->setFilter("*.mp3"); //Only show files

    ui->treeView_DirTree->setModel(dirModel); //Add model to QTreeView

    //QModelIndex idx = dirModel->index("./uploaded"); //Set the root item
    QModelIndex idx = dirModel->index("../"); //Set the root item
    ui->treeView_DirTree->setRootIndex(idx);

    // скрываем все столбцы дерева, кроме название аудио-файла
    ui->treeView_DirTree->setColumnHidden(1,true);
    ui->treeView_DirTree->setColumnHidden(2,true);
    ui->treeView_DirTree->setColumnHidden(3,true);

    // связываем значения некоторых столбцов SQLite с формами ввода текста и слайдерами.
    // Конечно, данные можно править прямо в ячейках SQLite, но с помощью слайдеров удобнее.
    mapper->setModel(model);
    mapper->setItemDelegate(new QSqlRelationalDelegate(this));
    mapper->addMapping(ui->lineEdit_TagSaveName, model->fieldIndex("TAG_NAME"));
    mapper->addMapping(ui->label_TAGBeginTime, model->fieldIndex("TAG_START"));
    //mapper->addMapping(ui->hSlider_TagTimeBegin, model->fieldIndex("TAG_NAME"));

    connect(ui->tableView_Sqlite->selectionModel(), &QItemSelectionModel::currentRowChanged,
                                            mapper, &QDataWidgetMapper::setCurrentModelIndex );

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
    qDebug() << "Open FileName" << FileName;

    QString audio_URL_prefix={""};
    // если на момент компиляции qsystemdetection.h ОС=Linux, то прибавляем префикс "file://"
    #if defined(Q_OS_LINUX)
        audio_URL_prefix="file://";
        qDebug() << "TARGET_OS= UNIX";
    #endif


    M_Player->setMedia(QUrl(audio_URL_prefix + FileName));

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

void MainWindow::on_actionExit_triggered()
{
    QCoreApplication::quit();
}


// если нажать на дереве папок (один клик)
void MainWindow::on_treeView_DirTree_clicked(const QModelIndex &index)
{
    qDebug() << "вы кликнули по файлу в дереве с именем: " << " =[+ " << index.data(Qt::DisplayRole).toString() << "+]=";
}


// обработка двойного клика на каком-то файле в дереве папок
void MainWindow::on_treeView_DirTree_doubleClicked(const QModelIndex &index)
{
    // создаём временную модель и вытягиваем из неё полный путь к кликнутому файлу.
    QFileSystemModel *dirModel_tmp = new QFileSystemModel();
    QString full_path_mp3 = dirModel_tmp->filePath(index);

    qDebug() << "path=" << full_path_mp3;

    // загружаем новый аудио-файл в проигрыватель.
    //    if (QOperatingSystemVersion::Windows == 1){
    //        M_Player->setMedia(QUrl(full_path_mp3));
    //    }

    //    if (Q_OS_UNIX){
    //        M_Player->setMedia(QUrl("file://" + full_path_mp3));
    //    }


    QString audio_URL_prefix={""};
    // если в файле .pro задана переменная TARGET_OS=UNIX_OS, то прибавляем префикс "file://"
    #if defined(Q_OS_LINUX)
        audio_URL_prefix="file://";
        qDebug() << "TARGET_OS= UNIX";
    #endif

    M_Player->setMedia(QUrl(audio_URL_prefix + full_path_mp3));


    // загружаем имя кликнутого файла в лейблу "File Name"
    QFileInfo File(index.data(Qt::DisplayRole).toString());
    ui->lbl_Value_File_Name->setText(File.fileName());

    // запускаем проигрыватель через функцию.
    MainWindow::on_pushButton_Play_clicked();
}


// кнопка сохранения TAG в БД SQlite
void MainWindow::on_pushButton_TAGSave_clicked()
{
    model->insertRow(model->rowCount());

    query->prepare("INSERT INTO AudioFiles(ID, FILE_PATH, FILE_MD5, TAG_NAME, TAG_START, TAG_FINISH, TAG_DUR)"
                   "VALUES (:ID_Value, :FPATH_Value, :FSUM_Value, :TAGNAME_Value, :TAGSTART_Value, :TAGFINISH_Value, :TAGDUR_Value)");
    query->bindValue(":ID_Value",        model->rowCount());
    query->bindValue(":FPATH_Value",     ui->lbl_Value_File_Name->text());
    query->bindValue(":FSUM_Value",      "123eef");
    query->bindValue(":TAGNAME_Value",   ui->lineEdit_TagSaveName->text());
    query->bindValue(":TAGSTART_Value",  ui->label_TAGBeginTime->text());
    query->bindValue(":TAGFINISH_Value", ui->label_TAGEndTime->text());
    query->bindValue(":TAGDUR_Value",    ui->label_TAG_Duration->text());





    query->exec();

    model->select();




    //    QDataWidgetMapper *mapper = new QDataWidgetMapper(this);
    //    mapper->setModel(model);
    //    mapper->setItemDelegate(new BookDelegate(this));
    //    mapper->addMapping(ui.titleEdit, model->fieldIndex("title"));
    //    mapper->addMapping(ui.yearEdit, model->fieldIndex("year"));
    //    mapper->addMapping(ui.authorEdit, authorIdx);
    //    mapper->addMapping(ui.genreEdit, genreIdx);
    //    mapper->addMapping(ui.ratingEdit, model->fieldIndex("rating"));

    //    connect(ui.bookTable->selectionModel(),
    //            &QItemSelectionModel::currentRowChanged,
    //            mapper,
    //            &QDataWidgetMapper::setCurrentModelIndex
    //            );

    //    ui.bookTable->setCurrentIndex(model->index(0, 0));
    //    createMenuBar();



//    QString query_ = QString("ALTER TABLE %1 ADD COLUMN %2 %3").arg(m_table).arg(m_name).arg(m_type);
//    QSqlQuery query("", *m_db);
//    bool status = query.exec(query_);

//    if (!status) {
//        QString err = query.lastError().text();
//        //emit errorOccured(err);
//    }



    //    Таблица AudioFiles:
    //    * ID (int) (с уникальным ключом)
    //    * FILE_PATH (text)
    //    * FILE_MD5 (text) хеш-сумма этого файла
    //    * TAG_NAME (text) имя тэга
    //    * TAG_START (text)
    //    * TAG_FINISH (text)
    //    * TAG_DUR (text)

}


void MainWindow::on_PushButton_TAGDel_clicked()
{
    model->removeRow(row);
    model->select();
}


void MainWindow::on_tableView_Sqlite_clicked(const QModelIndex &index)
{
    row = index.row();

    //qDebug() << "в таблице SQLite вы нажали на: " << index.data(Qt::DisplayRole).toString();
    qDebug() << "в таблице SQLite вы нажали на: " << index.data().toString();
    qDebug() << query->exec("SELECT * FROM AudioFiles WHERE ID=1;");

    // TODO!! LOAD TAG from DB
}

