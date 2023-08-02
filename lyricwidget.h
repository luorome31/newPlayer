#ifndef LYRICWIDGET_H
#define LYRICWIDGET_H

#include "lyrics.h"

#include <QWidget>

namespace Ui {
class LyricWidget;
}

class LyricWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LyricWidget(QWidget *parent = nullptr);
    ~LyricWidget();
    int lyricsIndex;//控制歌词播放进度
    Lyrics* lyricsFile;//当前歌曲歌词文件
    void updateLyrics();
    bool mMoveing;
    QPoint mMovePosition;
    void clearLyrics();
protected:
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);

private:
    Ui::LyricWidget *ui;
};

#endif // LYRICWIDGET_H
