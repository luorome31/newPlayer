#ifndef MUSICPLAYER_H
#define MUSICPLAYER_H

#include <QDir>
#include <QListWidget>
#include <QMainWindow>
#include <QMediaPlayer>
#include <QTableWidget>
#include <QTreeWidget>
#include <QWidget>
#include"lyrics.h"
#include "lyricwidget.h"
#include"musicbeans.h"
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    QMediaPlayer* player;
    QMediaPlaylist* playlist;
    QString MusicDir="E:\\Users\\luorome\\Desktop\\question";
    bool isPlaying;
    bool isSilence;
    bool isLyrics;
    int playMode;
    int currentSongIndex;
    int songNumber;
    int totalDuration;
    QStringList musicFiles;

    QDir musicFileDir;//用于保存线上的下载歌曲
    SongList searchResultSongs;//用来表示下载歌曲本身

    QString msecondToString(qint64 msecond);
    QString songFilePath(const Music &music)const;
    QString lyricPath(const Music &music) const;
    const QString API_DOMAIN = "http://iwxyi.com:3000/";
    void downloadSongFile(Music music);
    void downloadSongLyric(Music music);
    void initSearchResultTable();
    void initOtherWidget();


//    int lyricsIndex;//控制歌词播放进度
//    Lyrics* lyricsFile;//当前歌曲歌词文件
    LyricWidget* lyricWidget;
    void updateLyricsPosition(int position);//当前歌曲同步显示歌词

    QMap<QString, QStringList> albumMap;       // 专辑名称分类映射
    QMap<QString, QStringList> artistMap;      // 演唱者分类映射

    void addToAlbumMap(const QString& album, const QString& musicItem);
    void addToArtistMap(const QString& artist, const QString& musicItem);
    QTreeWidgetItem* createTreeItem(const QString& text);
    void populateAuthorWidget();
    void populateAlbumWidget();
    void setSearchResultTable(SongList songs);

    QString author;//当前歌曲的作者
    QString album;//当前歌曲的专辑
    QStringList getFileNames(const QString& path);
    void addItem(const QString& musicItem);
    void addNewItem(const QString& musicItem);
    void addNewItem(const Music& musci);
    ~MainWindow();
signals:
    void clear_Lyrics();
public slots:
    void timeAdjust(int position);
    void audioShouldAdjust(int value);
    void timeShouldAdjust(int value);
    void songInit();
//    void songInfoShow();//第一版7.15号前的歌词显示功能
    void updateDuration(int duration);
    void updateCurrentTimeLabel();

    void onSongItemClicked(QListWidgetItem* item);
    void onAuthorItemClicked(QTreeWidgetItem* item);
    void onAlbumItemClicked(QTreeWidgetItem* item);
    void widgetAdjust(QListWidgetItem* item);

    void searchMusic(const QString& key);


private slots:
    void on_previousButton_clicked();
    void on_controlButton_clicked();

    void on_nextButton_clicked();

    void on_audioButton_clicked();

    void on_playMode_clicked();

    void on_addFilesButton_clicked();

    void on_searchLineEdit_returnPressed();

    void on_searchButton_clicked();

    void on_searchResultTable_itemDoubleClicked(QTableWidgetItem *item);

    void on_btn_showlYRIC_clicked();


private:
    Ui::MainWindow *ui;
};
#endif // MUSICPLAYER_H
