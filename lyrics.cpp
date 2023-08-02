#include "lyrics.h"
#include<QRegularExpression>
#include <QFile>
#include<QDebug>
#include <QTextCodec>
Lyrics::Lyrics(const QString &path, QObject *parent) : QObject(parent)
{
    this->lyricsPath=path;
}

 Lyrics::~Lyrics()
{

}

QList<QString> Lyrics::getLyricsWords()
{
    return this->lyricsWords;
}

QList<int> Lyrics::getLyricsTime()
{
    return this->lyricsTime;
}

bool Lyrics::readLyrics(QString path)
{
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){
       this->lyricsWords.clear();
        this->lyricsTime.clear();
        return false;
    }
    QString line="";
    QTextCodec* codec = QTextCodec::codecForName("ANSI");

    // 创建QTextStream对象，并设置编码
    QTextStream stream(&file);
    stream.setCodec(codec);
    while((line=stream.readLine())>0){
        qDebug()<<line;
        analyseLyrics(line);
    }
    return true;
}

bool Lyrics::analyseLyrics(QString lyricLine)
{
    if(lyricLine==nullptr||lyricLine.isEmpty()){
        qDebug()<<"the line is not existed";
        return false;
    }
     QRegularExpression regularExpression("\\[(\\d+)?:(\\d+)?(\\.\\d+)?\\](.*)?");
     int index=0;
     QRegularExpressionMatch match = regularExpression.match(lyricLine,index);
     if(match.hasMatch()){
         QString text = match.captured(4);
         int time = match.captured(1).toInt()*60000+match.captured(2).toInt()*1000;
         qDebug()<<text<<" "<<time;
         qDebug()<<"具体时间："<<match.captured(1).toInt()<<match.captured(2).toInt();
         this->lyricsTime.push_back(time);
         this->lyricsWords.push_back(text);
     }
     return true;
}
