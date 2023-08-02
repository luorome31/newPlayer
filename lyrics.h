#ifndef LYRICS_H
#define LYRICS_H

#include <QObject>
class Lyrics : public QObject
{
    Q_OBJECT
public:
    explicit Lyrics(const QString& path,QObject *parent = nullptr);
    explicit Lyrics(QObject *parent = nullptr);
    ~Lyrics();
    QString lyricsPath;
    QList<QString> lyricsWords;
    QList<int> lyricsTime;
    QList<QString> getLyricsWords();
    QList<int> getLyricsTime();
    bool readLyrics(QString path);
    bool analyseLyrics(QString lyricLine);

signals:

};

#endif // LYRICS_H
