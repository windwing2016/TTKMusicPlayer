#include "musickgquerytoplistrequest.h"

MusicKGQueryToplistRequest::MusicKGQueryToplistRequest(QObject *parent)
    : MusicQueryToplistRequest(parent)
{
    m_pageSize = TOPLIST_PAGE_SIZE;
    m_queryServer = QUERY_KG_INTERFACE;
}

void MusicKGQueryToplistRequest::startToPage(int offset)
{
    TTK_INFO_STREAM(className() << "startToPage" << offset);

    deleteAll();
    m_totalSize = 0;
    m_pageIndex = offset;

    QNetworkRequest request;
    request.setUrl(TTK::Algorithm::mdII(KG_TOPLIST_URL, false).arg(m_queryValue).arg(offset).arg(m_pageSize));
    ReqKGInterface::makeRequestRawHeader(&request);

    m_reply = m_manager.get(request);
    connect(m_reply, SIGNAL(finished()), SLOT(downLoadFinished()));
    QtNetworkErrorConnect(m_reply, this, replyError, TTK_SLOT);
}

void MusicKGQueryToplistRequest::startToSearch(const QString &value)
{
    MusicQueryToplistRequest::startToSearch(value.isEmpty() ? "6666" : value);
}

void MusicKGQueryToplistRequest::startToQueryResult(TTK::MusicSongInformation *info, int bitrate)
{
    TTK_INFO_STREAM(className() << "startToQueryResult" << info->m_songId << bitrate << "kbps");

    MusicPageQueryRequest::downLoadFinished();
    TTK_NETWORK_QUERY_CHECK();
    ReqKGInterface::parseFromSongProperty(info, bitrate);
    TTK_NETWORK_QUERY_CHECK();
    MusicQueryToplistRequest::startToQueryResult(info, bitrate);
}

void MusicKGQueryToplistRequest::downLoadFinished()
{
    TTK_INFO_STREAM(className() << "downLoadFinished");

    MusicPageQueryRequest::downLoadFinished();
    if(m_reply && m_reply->error() == QNetworkReply::NoError)
    {
        QJson::Parser json;
        bool ok = false;
        const QVariant &data = json.parse(m_reply->readAll(), &ok);
        if(ok)
        {
            QVariantMap value = data.toMap();
            if(value.contains("data"))
            {
                value = value["data"].toMap();
                m_totalSize = value["total"].toInt();

                queryToplistInfo(value);

                const QVariantList &datas = value["info"].toList();
                for(const QVariant &var : qAsConst(datas))
                {
                    if(var.isNull())
                    {
                        continue;
                    }

                    value = var.toMap();
                    TTK_NETWORK_QUERY_CHECK();

                    TTK::MusicSongInformation info;
                    info.m_songId = value["hash"].toString();

                    info.m_albumId = value["album_id"].toString();

                    info.m_duration = TTKTime::formatDuration(value["duration"].toInt() * TTK_DN_S2MS);
                    info.m_year.clear();
                    info.m_trackNumber = "0";

                    TTK_NETWORK_QUERY_CHECK();
                    ReqKGInterface::parseFromSongAlbumLrc(&info);
                    TTK_NETWORK_QUERY_CHECK();
                    ReqKGInterface::parseFromSongAlbumInfo(&info, value["album_audio_id"].toString());
                    TTK_NETWORK_QUERY_CHECK();
                    ReqKGInterface::parseFromSongProperty(&info, value);
                    TTK_NETWORK_QUERY_CHECK();

                    Q_EMIT createResultItem({info, serverToString()});
                    m_items << info;
                }
            }
        }
    }

    Q_EMIT downLoadDataChanged({});
    deleteAll();
}

void MusicKGQueryToplistRequest::queryToplistInfo(const QVariantMap &input)
{
    Q_UNUSED(input);

    QNetworkRequest request;
    request.setUrl(TTK::Algorithm::mdII(KG_TOPLIST_INFO_URL, false));
    ReqKGInterface::makeRequestRawHeader(&request);

    const QByteArray &bytes = TTK::syncNetworkQueryForGet(&request);
    if(bytes.isEmpty())
    {
        return;
    }

    QJson::Parser json;
    bool ok = false;
    const QVariant &data = json.parse(bytes, &ok);
    if(ok)
    {
        QVariantMap value = data.toMap();
        if(value.contains("data"))
        {
            value = value["data"].toMap();
            MusicResultDataItem item;
            item.m_updateTime = TTKDateTime::format(value["timestamp"].toLongLong() * TTK_DN_S2MS, TTK_YEAR_FORMAT);

            const QVariantList &datas = value["info"].toList();
            for(const QVariant &var : qAsConst(datas))
            {
                if(var.isNull())
                {
                    continue;
                }

                value = var.toMap();
                TTK_NETWORK_QUERY_CHECK();

                if(m_queryValue != value["rankid"])
                {
                    continue;
                }

                item.m_name = value["rankname"].toString();
                item.m_coverUrl = value["banner7url"].toString().replace("{size}", "400");
                item.m_count = value["play_times"].toString();
                item.m_description = value["intro"].toString();
                Q_EMIT createToplistItem(item);
                return;
            }
        }
    }
}
