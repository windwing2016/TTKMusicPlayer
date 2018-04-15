#include "musicdjradioprogramcategorythread.h"
#include "musicdjradiothreadabstract.h"
#include "musicsemaphoreloop.h"
#include "musictime.h"
#///QJson import
#include "qjson/parser.h"

MusicDJRadioProgramCategoryThread::MusicDJRadioProgramCategoryThread(QObject *parent)
    : MusicDownLoadQueryThreadAbstract(parent)
{
    m_pageSize = 30;
    m_queryServer = "WangYi";
}

QString MusicDJRadioProgramCategoryThread::getClassName()
{
    return staticMetaObject.className();
}

void MusicDJRadioProgramCategoryThread::startToSearch(QueryType type, const QString &category)
{
    if(type == MusicQuery)
    {
        startToSearch(category);
    }
    else
    {
        m_searchText = category;
        startToPage(0);
    }
}

void MusicDJRadioProgramCategoryThread::startToPage(int offset)
{
    if(!m_manager)
    {
        return;
    }

    M_LOGGER_INFO(QString("%1 startToSearch %2").arg(getClassName()).arg(offset));
    deleteAll();
    m_pageTotal = 0;
    m_interrupt = true;

    QNetworkRequest request;
    QUrl musicUrl(MusicUtils::Algorithm::mdII(DJ_RADIO_LIST_URL, false)
                  .arg(m_searchText).arg(m_pageSize).arg(offset*m_pageSize));
    request.setUrl(musicUrl);
    setSslConfiguration(&request);

    m_reply = m_manager->get(request);
    connect(m_reply, SIGNAL(finished()), SLOT(downLoadFinished()));
    connect(m_reply, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(replyError(QNetworkReply::NetworkError)));
}

void MusicDJRadioProgramCategoryThread::startToSearch(const QString &category)
{
    if(!m_manager)
    {
        return;
    }

    M_LOGGER_INFO(QString("%1 startToSearch %2").arg(getClassName()).arg(category));
    m_interrupt = true;

    QNetworkRequest request;
    if(!m_manager || m_stateCode != MusicNetworkAbstract::Init) return;
    QByteArray parameter = makeTokenQueryUrl(&request,
               MusicUtils::Algorithm::mdII(DJ_DETAIL_N_URL, false),
               MusicUtils::Algorithm::mdII(DJ_DETAIL_NDT_URL, false).arg(category));
    if(!m_manager || m_stateCode != MusicNetworkAbstract::Init) return;
    setSslConfiguration(&request);

    QNetworkReply *reply = m_manager->post(request, parameter);
    connect(reply, SIGNAL(finished()), SLOT(getDetailsFinished()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(replyError(QNetworkReply::NetworkError)));
}

void MusicDJRadioProgramCategoryThread::getProgramInfo(MusicResultsItem &item)
{
    if(!m_manager)
    {
        return;
    }

    M_LOGGER_INFO(QString("%1 getProgramInfo %2").arg(getClassName()).arg(item.m_id));

    QNetworkRequest request;
    if(!m_manager || m_stateCode != MusicNetworkAbstract::Init) return;
    QByteArray parameter = makeTokenQueryUrl(&request,
               MusicUtils::Algorithm::mdII(DJ_PROGRAM_INFO_N_URL, false),
               MusicUtils::Algorithm::mdII(DJ_PROGRAM_INFO_NDT_URL, false).arg(item.m_id));
    if(!m_manager || m_stateCode != MusicNetworkAbstract::Init) return;
    setSslConfiguration(&request);

    MusicSemaphoreLoop loop;
    QNetworkReply *reply = m_manager->post(request, parameter);
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    QObject::connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), &loop, SLOT(quit()));
    loop.exec();

    if(!reply || reply->error() != QNetworkReply::NoError)
    {
        return;
    }

    QJson::Parser parser;
    bool ok;
    QVariant data = parser.parse(reply->readAll(), &ok);
    if(ok)
    {
        QVariantMap value = data.toMap();
        if(value["code"].toInt() == 200 && value.contains("djRadio"))
        {
            value = value["djRadio"].toMap();
            item.m_coverUrl = value["picUrl"].toString();
            item.m_name = value["name"].toString();

            value = value["dj"].toMap();
            item.m_nickName = value["nickname"].toString();
        }
    }
}

void MusicDJRadioProgramCategoryThread::downLoadFinished()
{
    if(!m_reply)
    {
        deleteAll();
        return;
    }

    M_LOGGER_INFO(QString("%1 downLoadFinished").arg(getClassName()));
    emit clearAllItems();
    m_musicSongInfos.clear();
    m_interrupt = false;

    if(m_reply->error() == QNetworkReply::NoError)
    {
        QString html(m_reply->readAll());

        QRegExp regx("<a.*class=\"zpgi\".*>(\\d+)</a>");
        regx.setMinimal(true);
        int pos = html.indexOf(regx);
        while(pos != -1)
        {
            if(m_interrupt) return;

            m_pageTotal = regx.cap(1).toInt()*m_pageSize;

            pos += regx.matchedLength();
            pos = regx.indexIn(html, pos);
        }

        regx.setPattern("<a href=\".?/djradio\\?id=(\\d+).*title.*</a>");
        pos = html.indexOf(regx);

        while(pos != -1)
        {
            if(m_interrupt) return;

            MusicResultsItem info;
            info.m_id = regx.cap(1);

            if(m_interrupt || !m_manager || m_stateCode != MusicNetworkAbstract::Init) return;
            getProgramInfo(info);
            if(m_interrupt || !m_manager || m_stateCode != MusicNetworkAbstract::Init) return;

            emit createProgramItem(info);

            pos += regx.matchedLength();
            pos = regx.indexIn(html, pos);
        }
    }

//    emit downLoadDataChanged(QString());
    deleteAll();
    M_LOGGER_INFO(QString("%1 downLoadFinished deleteAll").arg(getClassName()));
}

void MusicDJRadioProgramCategoryThread::getDetailsFinished()
{
    QNetworkReply *reply = MObject_cast(QNetworkReply*, QObject::sender());

    M_LOGGER_INFO(QString("%1 getDetailsFinished").arg(getClassName()));
    emit clearAllItems();
    m_musicSongInfos.clear();
    m_interrupt = false;

    if(reply && reply->error() == QNetworkReply::NoError)
    {
        QByteArray bytes = reply->readAll();

        QJson::Parser parser;
        bool ok;
        QVariant data = parser.parse(bytes, &ok);
        if(ok)
        {
            QVariantMap value = data.toMap();
            if(value["code"].toInt() == 200 && value.contains("programs"))
            {
                bool categoryFlag = false;
                ////////////////////////////////////////////////////////////
                QVariantList datas = value["programs"].toList();
                foreach(const QVariant &var, datas)
                {
                    if(var.isNull())
                    {
                        continue;
                    }

                    value = var.toMap();
                    MusicObject::MusicSongInformation musicInfo;
                    musicInfo.m_songName = MusicUtils::String::illegalCharactersReplaced(value["name"].toString());
                    musicInfo.m_timeLength = MusicTime::msecTime2LabelJustified(value["duration"].toInt());

                    QVariantMap radioObject = value["radio"].toMap();
                    musicInfo.m_smallPicUrl = radioObject["picUrl"].toString();
                    musicInfo.m_artistId = QString::number(radioObject["id"].toInt());
                    musicInfo.m_singerName = MusicUtils::String::illegalCharactersReplaced(radioObject["name"].toString());

                    QVariantMap mainSongObject = value["mainSong"].toMap();
                    musicInfo.m_songId = QString::number(mainSongObject["id"].toInt());

                    if(m_interrupt || !m_manager || m_stateCode != MusicNetworkAbstract::Init) return;
                    readFromMusicSongAttribute(&musicInfo, mainSongObject, m_searchQuality, true);
                    if(m_interrupt || !m_manager || m_stateCode != MusicNetworkAbstract::Init) return;
                    ////////////////////////////////////////////////////////////
                    if(!categoryFlag)
                    {
                        categoryFlag = true;
                        MusicResultsItem info;
                        info.m_name = musicInfo.m_songName;
                        info.m_nickName = musicInfo.m_singerName;
                        info.m_coverUrl = musicInfo.m_smallPicUrl;
                        info.m_playCount = QString::number(radioObject["subCount"].toInt());
                        info.m_updateTime = QDateTime::fromMSecsSinceEpoch(value["createTime"].toULongLong()).toString("yyyy-MM-dd");
                        emit createCategoryInfoItem(info);
                    }
                    ////////////////////////////////////////////////////////////
                    if(musicInfo.m_songAttrs.isEmpty())
                    {
                        continue;
                    }

                    MusicSearchedItem item;
                    item.m_songName = musicInfo.m_songName;
                    item.m_singerName = musicInfo.m_singerName;
                    item.m_albumName.clear();
                    item.m_time = musicInfo.m_timeLength;
                    item.m_type = mapQueryServerString();
                    emit createSearchedItem(item);
                    m_musicSongInfos << musicInfo;
                }
            }
        }
    }

    emit downLoadDataChanged(QString());
    M_LOGGER_INFO(QString("%1 getDetailsFinished deleteAll").arg(getClassName()));
}
