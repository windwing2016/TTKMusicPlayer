#ifndef MUSICDOWNLOADTHREADABSTRACT_H
#define MUSICDOWNLOADTHREADABSTRACT_H

/* =================================================
 * This file is part of the Music Player project
 * Copyright (c) 2014 - 2015 Greedysky Studio
 * All rights reserved!
 * Redistribution and use of the source code or any derivative
 * works are strictly forbiden.
   =================================================*/

#include <QNetworkReply>
#include <QFile>
#include "musiclibexportglobal.h"

const QString MUSIC_REQUERY_URL = "http://so.ard.iyyin.com/s/song_with_out?q=%1&page=1&size=10000000";
const QString MUSIC_LRC_URL = "http://lp.music.ttpod.com/lrc/down?lrcid=&artist=%1&title=%2&song_id=%3";
const QString SML_BG_ART_URL = "http://lp.music.ttpod.com/pic/down?artist=%1";
const QString BIG_BG_ART_URL = "http://www.kuwo.cn/mingxing/%1/pic.htm";

class QNetworkAccessManager;

class MUSIC_NETWORK_EXPORT MusicDownLoadThreadAbstract : public QObject
{
    Q_OBJECT
public:
    explicit MusicDownLoadThreadAbstract(const QString &url, const QString &save,
                                         QObject *parent = 0);
    virtual ~MusicDownLoadThreadAbstract();

    void deleteAll();
    virtual void startToDownload() = 0;

signals:
    void musicDownLoadFinished(const QString &name);
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);

public slots:
    virtual void downLoadFinished() = 0;
    void replyError(QNetworkReply::NetworkError error);

protected:
    QNetworkAccessManager *m_manager;
    QNetworkReply* m_reply;
    QFile *m_file;
    QString m_url;
    QString m_savePathName;

};

#endif // MUSICDOWNLOADTHREADABSTRACT_H
