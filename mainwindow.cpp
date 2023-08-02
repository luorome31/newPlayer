#include"mainwindow.h"
#include "ui_mainwindow.h"
#include <QTableWidget>
#include <QFileDialog>
#include <QMediaMetaData>
#include <QMediaPlaylist>
#include <QMediaService>
#include <QTime>
#include <QTimer>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QDesktopWidget>
#include "musicbeans.h"
/*注意：本地音乐文件路径在开发时是固定的，如要使用，请记得修改*/
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow),musicFileDir(QApplication::applicationDirPath() + "/musics")
{
    ui->setupUi(this);
    //去掉主界面类自带的菜单
    this->setWindowFlag(Qt::FramelessWindowHint);
    this->initSearchResultTable();
    this->initOtherWidget();
    this->ui->tabWidget->tabBar()->hide();
    this->ui->tabWidget->setCurrentIndex(0);
    this->player=new QMediaPlayer;
    this->playlist=new QMediaPlaylist;
    this->lyricWidget = new LyricWidget(this);
    this->lyricWidget->hide();
    this->setWindowIcon(QIcon(":/bear.png"));
    //加入音乐文件
    this->musicFiles =getFileNames(MusicDir);
    QString curFile;
    foreach(curFile,musicFiles){
        addItem(curFile);
        playlist->addMedia(QUrl::fromLocalFile(MusicDir+"\\"+curFile));
    }
    songNumber = playlist->mediaCount();

    //设置播放数值
    playlist->setCurrentIndex(0);
    player->setPlaylist(playlist);
    player->setVolume(50);
    player->setPlaybackRate(QMediaPlaylist::Sequential);
    player->setPlaybackRate(1);
    this->isPlaying=false;
    this->isSilence=false;
    this->isLyrics = false;
    this->playMode=1;
    this->playlist->setPlaybackMode(QMediaPlaylist::Sequential);
    this->ui->audioSlider->setValue(50);
    //初始化第一首歌的歌词
    if(playlist->currentIndex()==0&&playlist->mediaCount()!=0){
    lyricWidget->lyricsIndex = 0;
    QString currentMusic=musicFiles.at(playlist->currentIndex());
    QString lyricsPath=QString(MusicDir+"\\"+currentMusic.left(currentMusic.lastIndexOf("."))+".lrc");
    lyricWidget->lyricsFile=new Lyrics(lyricsPath);
    bool flag=lyricWidget->lyricsFile->readLyrics(lyricsPath);
    qDebug()<<"歌词是否成功："<<flag;

    QString tepinfor = ui->songText->item(playlist->currentIndex())->text();
    QString infor = tepinfor.left(tepinfor.lastIndexOf("."));
    QStringList parts = infor.split("-");
    QString musicName = parts.at(0).trimmed();
    QString musicSinger = infor.right(infor.length() - infor.indexOf("-") - 1).trimmed();

    ui->label_musicName->setText(musicName);
    ui->label_musicSinger->setText(musicSinger);
    }

    //局部测试部分
    //
    connect(player, &QMediaPlayer::durationChanged, this, &MainWindow::updateDuration);
    connect(this->ui->audioSlider,&QSlider::valueChanged,this,&MainWindow::audioShouldAdjust);
    connect(player,&QMediaPlayer::positionChanged,this,&MainWindow::timeAdjust);
    connect(player,&QMediaPlayer::positionChanged,this,&MainWindow::updateLyricsPosition);
    connect(this->ui->timeSlider,&QSlider::sliderMoved,this,&MainWindow::timeShouldAdjust);
    connect(player,&QMediaPlayer::mediaChanged,this,&MainWindow::songInit);
    connect(player, &QMediaPlayer::positionChanged, this, &MainWindow::updateCurrentTimeLabel);
    //列表点击问题：记得同步MusicFile文件
    connect(this->ui->songText, &QListWidget::itemClicked, this, &MainWindow::onSongItemClicked);
    connect(this->ui->authorWidget, &QTreeWidget::itemClicked, this, &MainWindow::onAuthorItemClicked);
    connect(this->ui->albumWidget, &QTreeWidget::itemClicked, this, &MainWindow::onAlbumItemClicked);
    connect(this->ui->listone,&QListWidget::itemClicked,this,&MainWindow::widgetAdjust);
    populateAuthorWidget();
    populateAlbumWidget();
}

QString MainWindow::msecondToString(qint64 msecond)
{
    return QString("%1:%2").arg(msecond / 1000 / 60, 2, 10, QLatin1Char('0'))
            .arg(msecond / 1000 % 60, 2, 10, QLatin1Char('0'));
}
void MainWindow::initSearchResultTable()
{

    ui->searchResultTable->setShowGrid(false);
    ui->searchResultTable->verticalHeader()->hide();
    ui->searchResultTable->setFrameShape(QFrame::NoFrame);
    ui->searchResultTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->searchResultTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->searchResultTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->searchResultTable->setFocusPolicy(Qt::NoFocus);
    ui->searchResultTable->setSelectionBehavior(QTableWidget::SelectRows);
    ui->searchResultTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->searchResultTable->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    ui->searchResultTable->verticalHeader()->setDefaultAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->searchResultTable->horizontalHeader()->setFixedHeight(40);
    ui->searchResultTable->verticalHeader()->setDefaultSectionSize(40);
    ui->searchResultTable->setAlternatingRowColors(true);
    ui->searchResultTable->horizontalHeader()->setHighlightSections(false);
    ui->searchResultTable->verticalHeader()->setDefaultSectionSize(35);
}

//初始化其他控件
void MainWindow::initOtherWidget()
{
    this->ui->songText->setStyleSheet(
                "QListWidget::item {"
                "height: 40px;"
                "font-size: 20px;"
                "}"
                "QListWidget::item:hover {"
                "background-color: rgba(242, 154, 118, 100);"
                "}"
                "QListWidget::item:selected {"
                "background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #EDF1BB, stop: 1 #EDF1BB);"
                "}"
                "QListWidget {"
                "border: 1px solid #000000;"
                "border-radius: 10px;"
                "}"
            );
    this->ui->authorWidget->setStyleSheet(
                "QTreeWidget::item {"
                "height: 40px;"
                "font-size: 20px;"
                "}"
                "QTreeWidget::item:hover {"
                "background-color: rgba(242, 154, 118, 100)"
                "}"
                "QTreeWidget::item:selected {"
                "background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #EDF1BB, stop: 1 #EDF1BB);"
                "}"
                "QTreeWidget {"
                "border-radius: 10px;"
                "background-color: rgb(248, 247, 240);"
                "border: none;"

                "}"
            );
    this->ui->albumWidget->setStyleSheet(
                "QTreeWidget::item {"
                "height: 40px;"
                "font-size: 20px;"
                "}"
                "QTreeWidget::item:hover {"
                "background-color: rgba(242, 154, 118, 100);"
                "}"
                "QTreeWidget::item:selected {"
                "background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #EDF1BB, stop: 1 #EDF1BB);"
                "}"
                "QTreeWidget {"
                "border-radius: 10px;"
                "background-color: rgb(248, 247, 240);"
                "border: none;"
                "}"
            );
}
//本地文件添加到专辑分类容器中
void MainWindow::addToAlbumMap(const QString& album, const QString& musicItem)
{
    if (albumMap.contains(album)) {
        albumMap[album].append(musicItem);
    } else {
        QStringList songs;
        songs.append(musicItem);
        albumMap.insert(album, songs);
    }
}
//本地文件添加到歌手分类容器中
void MainWindow::addToArtistMap(const QString& artist, const QString& musicItem)
{
    if (artistMap.contains(artist)) {
        artistMap[artist].append(musicItem);
    } else {
        QStringList songs;
        songs.append(musicItem);
        artistMap.insert(artist, songs);
    }
}
//下载歌曲的存储路径(歌曲下载有概率会失败，现已注释）
QString MainWindow::songFilePath(const Music &music) const
{
    return musicFileDir.absoluteFilePath(snum(music.id) + ".mp3");
}
//在线歌曲搜索
void MainWindow::downloadSongFile(Music music)
{
    QString url = API_DOMAIN +"/song/url?id=" + snum(music.id);

    qDebug()<< "获取歌曲信息：" << music.simpleString();
    QNetworkAccessManager* manager = new QNetworkAccessManager;
    QNetworkRequest* request = new QNetworkRequest(url);
    request->setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded; charset=UTF-8");
    request->setHeader(QNetworkRequest::UserAgentHeader, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/86.0.4240.111 Safari/537.36");
    connect(manager, &QNetworkAccessManager::finished, this, [=](QNetworkReply* reply)
    {
        QByteArray baData = reply->readAll();
        QJsonParseError error;
        QJsonDocument document = QJsonDocument::fromJson(baData, &error);
        if (error.error != QJsonParseError::NoError)
        {
            qDebug() << error.errorString();
            return ;
        }
        QJsonObject json = document.object();
        if (json.value("code").toInt() != 200)
        {
            qDebug() << ("返回结果不为200：") << json.value("message").toString();
            return ;
        }
        QJsonArray array = json.value("data").toArray();


        json = array.first().toObject();
        QString url = JVAL_STR(url);
        int br = JVAL_INT(br); // 比率320000
        int size = JVAL_INT(size);
        QString type = JVAL_STR(type); // mp3
        QString encodeType = JVAL_STR(encodeType); // mp3
        qDebug() << "  信息：" << br << size << type << encodeType << url;

        playlist->addMedia(QUrl(url));
        this->addNewItem(music);
//原在线歌曲下载功能
        // 下载歌曲本身
//        QNetworkAccessManager manager;
//        QEventLoop loop;
//        QNetworkReply *reply1 = manager.get(QNetworkRequest(QUrl(url)));
//        // 请求结束并完成下载后退出子事件循环
//        connect(reply1, &QNetworkReply::finished, &loop, &QEventLoop::quit);

//        // 开启子事件循环
//        loop.exec();
//        QByteArray baData1 = reply1->readAll();

//        QFile file(songFilePath(music));
//        file.open(QIODevice::WriteOnly);
//        file.write(baData1);
//        file.flush();
//        file.close();
//        qDebug()<<"歌曲下载完成";
//        playlist->addMedia(QUrl(songFilePath(music)));
//        this->addNewItem(music);

    });
    manager->get(*request);
    downloadSongLyric(music);

}
//歌词下载
void MainWindow::downloadSongLyric(Music music)
{
    if (QFileInfo(lyricPath(music)).exists())
        return ;
    QString url = API_DOMAIN + "/lyric?id=" + snum(music.id);
    QNetworkAccessManager* manager = new QNetworkAccessManager;
    QNetworkRequest* request = new QNetworkRequest(url);
    request->setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded; charset=UTF-8");
    request->setHeader(QNetworkRequest::UserAgentHeader, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/86.0.4240.111 Safari/537.36");
    connect(manager, &QNetworkAccessManager::finished, this, [=](QNetworkReply* reply){
        QByteArray baData = reply->readAll();
        QJsonParseError error;
        QJsonDocument document = QJsonDocument::fromJson(baData, &error);
        if (error.error != QJsonParseError::NoError)
        {
            qDebug() << error.errorString();
            return ;
        }
        QJsonObject json = document.object();
        if (json.value("code").toInt() != 200)
        {
            qDebug() << ("返回结果不为200：") << json.value("message").toString();
            return ;
        }

        QString lrc = json.value("lrc").toObject().value("lyric").toString();
        if (!lrc.isEmpty())
        {
            QFile file(lyricPath(music));
            file.open(QIODevice::WriteOnly);
            QTextStream stream(&file);
            stream << lrc;
            file.flush();
            file.close();
        }
        else
        {
            qDebug() << "warning: 下载的歌词是空的" << music.simpleString();
        }
    });
    manager->get(*request);
}
//下载歌词的存储路径
QString MainWindow::lyricPath(const Music &music) const
{
    return musicFileDir.absoluteFilePath(snum(music.id) + ".lrc");
}
void MainWindow::updateLyricsPosition(int position)
{
    // 查找当前播放位置对应的歌词索引
        int index = 0;
        while (index < lyricWidget->lyricsFile->getLyricsTime().size() && lyricWidget->lyricsFile->getLyricsTime().at(index) <= position) {
            index++;
        }
        lyricWidget->lyricsIndex = index - 1;
        lyricWidget->updateLyrics();
        // 更新歌词显示
//        if (lyricsIndex >= 0 && lyricsIndex < lyricsFile->getLyricsWords().size()) {
//            this->ui->lyricsLabel1->setText(lyricsFile->getLyricsWords().at(lyricsIndex));
//            this->ui->lyricsLabel2->setText(lyricsFile->getLyricsWords().at(lyricsIndex + 1));
//        }

}
QTreeWidgetItem* MainWindow::createTreeItem(const QString& text)
{
    QTreeWidgetItem* item = new QTreeWidgetItem();
    item->setText(0, text);
    return item;
}
//更新歌手分类
void MainWindow::populateAuthorWidget()
{
    this->ui->authorWidget->clear();
    for (const QString& artist : artistMap.keys()) {
        QTreeWidgetItem* artistItem = createTreeItem(artist);
        QStringList songs = artistMap.value(artist);
        for (const QString& song : songs) {
            QTreeWidgetItem* songItem = createTreeItem(song);
            artistItem->setText(1, song);
            artistItem->addChild(songItem);
        }
        this->ui->authorWidget->addTopLevelItem(artistItem);
    }
}

//更新专辑分类
void MainWindow::populateAlbumWidget()
{
     this->ui->albumWidget->clear();

    for (const QString& album : albumMap.keys()) {
        QTreeWidgetItem* albumItem = createTreeItem(album);
        QStringList songs = albumMap.value(album);
        for (const QString& song : songs) {
            QTreeWidgetItem* songItem = createTreeItem(song);
            songItem->setText(1, song);  // 将歌曲名称设置到索引为 1 的列中
            albumItem->addChild(songItem);
        }
        this->ui->albumWidget->addTopLevelItem(albumItem);
    }
}

//显示搜素结果
void MainWindow::setSearchResultTable(SongList songs)
{
    // 建立表
    QTableWidget* table = this->ui->searchResultTable;
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    enum{
        titleCol,
        artistCol,
        albumCol,
        durationCol
    };
    table->setColumnCount(4);
    QStringList headers{"标题", "作者", "专辑", "时长"};
    table->setHorizontalHeaderLabels(headers);

    // 设置列长度
    QFontMetrics fm(font());
    int fw = fm.horizontalAdvance("一二三四五六七八九十十一十二十三十四十五");
    auto createItem = [=](QString s){
        QTableWidgetItem *item = new QTableWidgetItem();
        if (s.length() > 16 && fm.horizontalAdvance(s) > fw)
        {
            item->setToolTip(s);
            s = s.left(15) + "...";
        }
        item->setText(s);
        return item;
    };

    table->setRowCount(songs.size());
    for (int row = 0; row < songs.size(); row++)
    {
       Music song = songs.at(row);
       table->setItem(row, titleCol, createItem(song.name));
       table->setItem(row, artistCol, createItem(song.artistNames));
       table->setItem(row, albumCol, createItem(song.album.name));
       table->setItem(row, durationCol, createItem(msecondToString(song.duration)));
    }

    QTimer::singleShot(0, [=]{
        table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    });
}

QStringList MainWindow::getFileNames(const QString &path)
{
    QDir dir(path);
    QStringList files = dir.entryList(QStringList()<<"*.mp3", QDir::Files|QDir::Readable, QDir::Name);
    return files;
}

//初始化时从固定文件夹获取本地音乐
void MainWindow::addItem(const QString &musicItem)
{

        this->ui->songText->addItem(new QListWidgetItem(QIcon(":/image/musicIcon.png"), musicItem));

        QMediaPlayer temPlayer;
        temPlayer.setMedia(QUrl::fromLocalFile(MusicDir + "\\" + musicItem));
        while(!temPlayer.isMetaDataAvailable())
        {
            QApplication::processEvents();
        }
        QString album = temPlayer.metaData(QMediaMetaData::AlbumTitle).toString();
        QString artist = temPlayer.metaData(QMediaMetaData::Author).toString();
        qDebug()<<"元数据: "<<artist<<album;
        addToAlbumMap(album, musicItem);
        addToArtistMap(artist, musicItem);


}
// 添加新的本地音乐
void MainWindow::addNewItem(const QString &musicItem)
{
        QString fileName = QFileInfo(musicItem).fileName();
        this->ui->songText->addItem(new QListWidgetItem(QIcon(":/image/musicIcon.png"),fileName));
        QMediaPlayer temPlayer;
        temPlayer.setMedia(QUrl::fromLocalFile(musicItem));
        while(!temPlayer.isMetaDataAvailable())
        {
            QApplication::processEvents();
        }
        QString album = temPlayer.metaData(QMediaMetaData::AlbumTitle).toString();
        QString artist = temPlayer.metaData(QMediaMetaData::Author).toString();
        qDebug()<<"元数据: "<<artist<<album;
        musicFiles.push_back(fileName);

        addToAlbumMap(album, fileName);
        addToArtistMap(artist, fileName);

}
//添加在线音乐
void MainWindow::addNewItem(const Music &musci)
{
    QString filename = musci.simpleString();
    this->ui->songText->addItem(new QListWidgetItem(QIcon(":/image/musicIcon.png"),filename+".mp3"));
    qDebug()<<"add list success"<<endl;
    this->musicFiles.insert(this->playlist->mediaCount(),QString::number(musci.id)+".mp3");

}

MainWindow::~MainWindow()
{
    delete this->ui;
}


void MainWindow::on_controlButton_clicked()
{
    if(!this->isPlaying){
        this->player->play();
        this->isPlaying=true;
        this->ui->controlButton->setStyleSheet(
        "QPushButton{"
        "border-style:hidden;"
        "border-image: url(:/statusImage/image/statusImage/pause.png);"
        "}"
         );
    }else{
        this->player->pause();
        this->isPlaying=false;
        this->ui->controlButton->setStyleSheet(
        "QPushButton{"
        "border-style:hidden;"
        "border-image: url(:/statusImage/image/statusImage/play.png);"
        "}"
         );
    }
//    QString currentMusic=musicFiles.at(playlist->currentIndex());
//    qDebug()<<currentMusic.left(currentMusic.lastIndexOf("."))<<player->state()
//           <<player->mediaStatus()
//          <<player->playbackRate()
//         <<playlist->playbackMode()
//        <<this->ui->timeSlider->value()
//         <<endl;
    qDebug()<<player->state();
}

void MainWindow::timeAdjust(int position)
{
    if(this->ui->timeSlider->isSliderDown())
        return;//如果手动调整进度条，则不处理
    this->ui->timeSlider->setSliderPosition(position);
}

void MainWindow::audioShouldAdjust(int value)
{

    int volume = value%101;
    if(volume ==0){
        this->isSilence=true;
    }
    this->player->setVolume(volume);
}

void MainWindow::timeShouldAdjust(int position)
{
    if (player->isAudioAvailable()) {
        player->setPosition(position);
        updateLyricsPosition(position);
    }
}

void MainWindow::songInit()
{
    qDebug()<<"mediaChange success";
    lyricWidget->lyricsIndex = 0;
    QString currentMusic=musicFiles.at(playlist->currentIndex());

    QString lyricsPath=QString(MusicDir+"\\"+currentMusic.left(currentMusic.lastIndexOf("."))+".lrc");
    if(!QFile(lyricsPath).exists()){
        lyricsPath=QString(QApplication::applicationDirPath()+"/musics/"+currentMusic.left(currentMusic.lastIndexOf("."))+".lrc");
    }
    qDebug()<<"歌词路径："<<lyricsPath;
    lyricWidget->lyricsFile=new Lyrics(lyricsPath);
    bool flag=lyricWidget->lyricsFile->readLyrics(lyricsPath);
    qDebug()<<"歌词是否成功："<<flag;
    if(!flag){
        connect(this,&MainWindow::clear_Lyrics,this->lyricWidget,&LyricWidget::clearLyrics);
        emit clear_Lyrics();
    }


//    QString tepinfor = ui->songText->item(playlist->currentIndex())->text();
//    QString infor = tepinfor.left(tepinfor.lastIndexOf("."));
//    QString musicName = infor.left(infor.indexOf("-"));
//    QString musicSinger =infor.right(infor.indexOf("-"));
    QString tepinfor = ui->songText->item(playlist->currentIndex())->text();
    QString infor = tepinfor.left(tepinfor.lastIndexOf("."));
    QStringList parts = infor.split("-");
    QString musicName = parts.at(0).trimmed();
    QString musicSinger = infor.right(infor.length() - infor.indexOf("-") - 1).trimmed();
    ui->label_musicName->setText(musicName);
    ui->label_musicSinger->setText(musicSinger);
}

//第一版同步显示歌词：失败在于无法跟随进度条做到实时切换
//void MainWindow::songInfoShow()
//{
//    //显示播放时间
//    int totalTime =this->player->duration();
//    int currentTime = ((player->position()/totalTime)*totalTime)/1000;
//    int second = (currentTime)/60;
//    int minus = currentTime%60;
//    this->ui->timeLabel->setText(QString("%1:%2").arg(second).arg(minus));

//    //显示歌词
//    if(!this->lyricsFile->getLyricsTime().isEmpty()&&currentTime*1000<lyricsFile->getLyricsTime().at(lyricsIndex)){
//        this->ui->lyricsLabel1->setText(lyricsFile->lyricsWords.at(lyricsIndex));
//        this->ui->lyricsLabel2->setText(lyricsFile->lyricsWords.at(lyricsIndex+1));
//        lyricsIndex++;
//    }
//    int currentTime =this->player->position();
//    int second = currentTime/1000;
//    int minu = second/60;
//    int sec = second%60;
    //显示歌词

//    if(!this->lyricsFile->getLyricsTime().empty()&&this->player->position()>=lyricsFile->lyricsTime.at(lyricsIndex)){
//        this->ui->lyricsLabel1->setText(lyricsFile->lyricsWords.at(lyricsIndex));
//        this->ui->lyricsLabel2->setText(lyricsFile->lyricsWords.at(lyricsIndex+1));
//        lyricsIndex++;
//    }
//    qDebug()<<lyricsIndex<<currentTime;
//    qDebug()<<this->player->duration();
//    qDebug()<<lyricsFile->getLyricsTime().empty()<<" "<<lyricsFile->getLyricsTime().at(lyricsIndex++);
//    qDebug()<<"time compare:"<<(currentTime*1000<lyricsFile->getLyricsTime().at(lyricsIndex));

//}

void MainWindow::updateDuration(int duration)
{
    this->ui->timeSlider->setRange(0, duration);

    QTime totalTime = QTime::fromMSecsSinceStartOfDay(duration);

    QString totalTimeStr = totalTime.toString("mm:ss");

    this->ui->timeLabel->setText(totalTimeStr);
}

void MainWindow::updateCurrentTimeLabel()
{
        qint64 duration = player->duration();
        qint64 position = player->position();

        QTime currentTime = QTime::fromMSecsSinceStartOfDay(position);
        QTime totalTime = QTime::fromMSecsSinceStartOfDay(duration);

        QString currentTimeStr = currentTime.toString("mm:ss");
        QString totalTimeStr = totalTime.toString("mm:ss");

        this->ui->currentTimeLabel->setText(currentTimeStr + " / " + totalTimeStr);
}

void MainWindow::onSongItemClicked(QListWidgetItem *item)
{
    int index = this->ui->songText->row(item);
    playlist->setCurrentIndex(index);
    player->play();
    this->isPlaying = true;
    emit player->mediaChanged(playlist->currentMedia());
    this->ui->controlButton->setStyleSheet(
    "QPushButton{"
    "border-style:hidden;"
    "border-image: url(:/statusImage/image/statusImage/pause.png);"
    "}");
}

void MainWindow::onAuthorItemClicked(QTreeWidgetItem* item)
{
    QString song = item->text(0);
    qDebug()<<"author information:"<<song;
    int index = musicFiles.indexOf(song);
    qDebug()<<"author"<<index;
    for(int i =0;i<musicFiles.size();i++){
        qDebug()<<musicFiles.at(i);
    }
    if (index != -1) {
        playlist->setCurrentIndex(index);
        player->play();
        this->isPlaying = true;

    }
    emit player->mediaChanged(playlist->currentMedia());
    this->ui->controlButton->setStyleSheet(
    "QPushButton{"
    "border-style:hidden;"
    "border-image: url(:/statusImage/image/statusImage/pause.png);"
    "}");

}

void MainWindow::onAlbumItemClicked(QTreeWidgetItem* item)
{
    QString album = item->text(0);
    QString song = item->text(1);
    int index = musicFiles.indexOf(song);
    if (index != -1) {
        playlist->setCurrentIndex(index);
        player->play();
        this->isPlaying = true;
    }
    emit player->mediaChanged(playlist->currentMedia());
    this->ui->controlButton->setStyleSheet(
    "QPushButton{"
    "border-style:hidden;"
    "border-image: url(:/statusImage/image/statusImage/pause.png);"
    "}");
}

void MainWindow::widgetAdjust(QListWidgetItem *item)
{
    int index=ui->listone->row(item);
    ui->tabWidget->setCurrentIndex(index);
}
void MainWindow::searchMusic(const QString& key)
{

    if (key.trimmed().isEmpty())
        return;
    QString url = "http://iwxyi.com:3000/search?keywords=" + key.toUtf8().toPercentEncoding();
    QNetworkAccessManager* manager = new QNetworkAccessManager;
    QNetworkRequest* request = new QNetworkRequest(url);
    request->setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded; charset=UTF-8");
    request->setHeader(QNetworkRequest::UserAgentHeader, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/86.0.4240.111 Safari/537.36");
    connect(manager, &QNetworkAccessManager::finished, this, [=](QNetworkReply* reply){
        QByteArray data = reply->readAll();
        QJsonParseError error;
        QJsonDocument document = QJsonDocument::fromJson(data, &error);
        if (error.error != QJsonParseError::NoError)
        {
            qDebug() << error.errorString();
            return;
        }

        QJsonObject json = document.object();
        if (json.value("code").toInt() != 200)
        {
            qDebug() << ("返回结果不为200：") << json.value("message").toString();
            return;
        }

        QJsonArray musics = json.value("result").toObject().value("songs").toArray();
        // 清除上次搜索结果
        searchResultSongs.clear();
        foreach (QJsonValue val, musics)
        {
            searchResultSongs << Music::fromJson(val.toObject());
        }

        setSearchResultTable(searchResultSongs);


    });
    manager->get(*request);

}



void MainWindow::on_previousButton_clicked()
{
    if(this->playMode==2){
        this->player->setPosition(0);
        return;
    }
    int currentIndex = playlist->currentIndex();
    if (currentIndex > 0) {
        playlist->setCurrentIndex(currentIndex - 1);
    }
//    connect(this,&MainWindow::clear_Lyrics,this->lyricWidget,&LyricWidget::clearLyrics);
    emit player->mediaChanged(playlist->currentMedia());
    this->ui->controlButton->setStyleSheet(
    "QPushButton{"
    "border-style:hidden;"
    "border-image: url(:/statusImage/image/statusImage/pause.png);"
    "}");
}

void MainWindow::on_nextButton_clicked()
{
    if(this->playMode==2){
        this->player->setPosition(0);
        return;
    }
    int currentIndex = playlist->currentIndex();
    if (currentIndex < playlist->mediaCount() - 1) {
        playlist->setCurrentIndex(currentIndex + 1);
    }
//    connect(this,&MainWindow::clear_Lyrics,this->lyricWidget,&LyricWidget::clearLyrics);
    emit player->mediaChanged(playlist->currentMedia());
    this->ui->controlButton->setStyleSheet(
    "QPushButton{"
    "border-style:hidden;"
    "border-image: url(:/statusImage/image/statusImage/pause.png);"
    "}");
}


void MainWindow::on_audioButton_clicked()
{
    if(this->isSilence){
        isSilence=false;
        this->player->setVolume(50);
        this->ui->audioSlider->setValue(50);
        this->ui->audioButton->setStyleSheet(
        "QPushButton{"
        "border-style:hidden;"
        "border-image: url(:/statusImage/image/statusImage/volume.png);"
        "}"
        "QPushButton:hover"
        "{"
        "border-style:hidden;"
        "border-image: url(:/statusImage/image/statusImage/volume-hover.png);"
        "};");

    }else{
        isSilence=true;
        this->player->setVolume(0);
        this->ui->audioSlider->setValue(0);
        this->ui->audioButton->setStyleSheet(
        "QPushButton{"
        "border-style:hidden;"
        "border-image: url(:/statusImage/image/statusImage/novolume.png);"
        "}"
        "QPushButton:hover"
        "{"
        "border-style:hidden;"
        "border-image: url(:/statusImage/image/statusImage/novolume-hover.png);"
        "};");
    }
}

void MainWindow::on_playMode_clicked()
{
    switch (this->playMode) {
    case 1:{
        playMode=2;
        this->playlist->setPlaybackMode(QMediaPlaylist::Loop);
        this->ui->playMode->setStyleSheet(
        "QPushButton{"
        "border-style:hidden;"
        "border-image: url(:/statusImage/image/statusImage/singleCycle.png);"
        "}"
        "QPushButton:hover"
        "{"
        "border-style:hidden;"
        "border-image: url(:/statusImage/image/statusImage/singleCycle-hover.png);"
        "};"
         );
        break;
        }
    case 2:{
        playMode=3;
        this->playlist->setPlaybackMode(QMediaPlaylist::Random);
        this->ui->playMode->setStyleSheet(
        "QPushButton{"
        "border-style:hidden;"
        "border-image: url(:/statusImage/image/statusImage/shufflePlay.png);"
        "}"
        "QPushButton:hover"
        "{"
        "border-style:hidden;"
        "border-image: url(:/statusImage/image/statusImage/shufflePlay-hover.png);"
        "};"
        );
        break;
        }
    case 3:{
        playMode=1;
        this->playlist->setPlaybackMode(QMediaPlaylist::Sequential);
        this->ui->playMode->setStyleSheet(
        "QPushButton{"
        "border-style:hidden;"
        "border-image: url(:/statusImage/image/statusImage/listRepeat.png);"
        "}"
        "QPushButton:hover"
        "{"
        "border-style:hidden;"
        "border-image: url(:/statusImage/image/statusImage/listRepeat-hover.png);"
        "};"
        );
        break;
        }
}}

void MainWindow::on_addFilesButton_clicked()
{
    QString file = QFileDialog::getOpenFileName(this, "选择音乐文件", QString(), "音乐文件 (*.mp3)");
    if (!file.isEmpty()) {
        QString fileName = QFileInfo(file).fileName();
        addNewItem(file);
//        this->musicFiles.insert(this->playlist->mediaCount(),file);
        playlist->addMedia(QUrl::fromLocalFile(file));
        populateAuthorWidget();
        populateAlbumWidget();
    }
}


void MainWindow::on_searchLineEdit_returnPressed()
{
    this->ui->searchResultTable->clear();
    QString text = this->ui->searchLineEdit->text();
    searchMusic(text);
    this->ui->tabWidget->setCurrentIndex(3);

}

void MainWindow::on_searchButton_clicked()
{
    on_searchLineEdit_returnPressed();
    this->ui->tabWidget->setCurrentIndex(3);
 }

void MainWindow::on_searchResultTable_itemDoubleClicked(QTableWidgetItem *item)
{
    this->downloadSongFile(this->searchResultSongs.at(item->row()));
}

void MainWindow::on_btn_showlYRIC_clicked()
{
    if(isLyrics){
        this->lyricWidget->hide();
        isLyrics = false;
    }else{
        this->lyricWidget->show();
        isLyrics = true;
    }
}
