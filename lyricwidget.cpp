#include "lyricwidget.h"
#include "ui_lyricwidget.h"
#include <QMouseEvent>
#include<QDebug>
LyricWidget::LyricWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LyricWidget)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint |Qt::WindowStaysOnTopHint);
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    this->setMouseTracking(true);

}

LyricWidget::~LyricWidget()
{
    delete ui;
}

void LyricWidget::updateLyrics()
{
      if (lyricsIndex >= 0 && lyricsIndex < lyricsFile->getLyricsWords().size()) {
          if(lyricsIndex+1>=lyricsFile->getLyricsWords().size()){
              return;
          }
           this->ui->lyricsLabel1->setText(lyricsFile->getLyricsWords().at(lyricsIndex));
           this->ui->lyricsLabel2->setText(lyricsFile->getLyricsWords().at(lyricsIndex + 1));
      }
}

void LyricWidget::clearLyrics()
{
    this->ui->lyricsLabel1->clear();
    this->ui->lyricsLabel2->clear();
}

//重写鼠标按下事件
void LyricWidget::mousePressEvent(QMouseEvent *event)
{
    mMoveing = true;
    mMovePosition = event->globalPos() - pos();
    return QWidget:: mousePressEvent(event);
}

//重写鼠标移动事件
void LyricWidget::mouseMoveEvent(QMouseEvent *event)
{

    if (mMoveing && (event->buttons() && Qt::LeftButton)
        && (event->globalPos()-mMovePosition).manhattanLength() > QApplication::startDragDistance())
    {
        move(event->globalPos()-mMovePosition);
        mMovePosition = event->globalPos() - pos();
    }
    return QWidget::mouseMoveEvent(event);
}
void LyricWidget::mouseReleaseEvent(QMouseEvent *event)
{
    mMoveing = false;
}
