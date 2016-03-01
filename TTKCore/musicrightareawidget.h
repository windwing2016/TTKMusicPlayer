#ifndef MUSICRIGHTAREAWIDGET_H
#define MUSICRIGHTAREAWIDGET_H

/* =================================================
 * This file is part of the TTK Music Player project
 * Copyright (c) 2014 - 2016 Greedysky Studio
 * All rights reserved!
 * Redistribution and use of the source code or any derivative
 * works are strictly forbiden.
   =================================================*/

#include <QWidget>
#include "musicglobaldefine.h"

class MusicVideoPlayWidget;
class MusicSettingWidget;
class MusicDownloadStatusLabel;
class MusicLrcContainerForDesktop;

namespace Ui {
    class MusicApplication;
}

class MUSIC_GUI_EXPORT MusicRightAreaWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MusicRightAreaWidget(QWidget *parent = 0);
    ~MusicRightAreaWidget();

    void setupUi(Ui::MusicApplication* ui);
    void stopLrcMask() const;
    void startTimerClock() const;
    void showPlayStatus(bool status) const;
    /*!
     * Set current play state button.
     */
    void setDestopLrcVisible(const QString &status) const;
    bool getDestopLrcVisible() const;
    void setInlineLrcVisible(const QString &status) const;
    void setSettingParameter() const;
    void getParameterSetting() const;
    bool checkSettingParameterValue() const;
    void updateCurrentLrc(qint64 current, qint64 total,
                          bool playStatus) const;
    void loadCurrentSongLrc(const QString &name,
                            const QString &path) const;
    void setSongSpeedAndSlow(qint64 time) const;
    void musicCheckHasLrcAlready() const;
    void showSettingWidget() const;

Q_SIGNALS:
    void updateBgThemeDownload();
    void updateBackgroundTheme();
    void desktopLrcClosed();
    void lockDesktopLrc(bool lock);

public Q_SLOTS:
    void deleteVideoWidget();
    void setDestopLrcVisible(bool v) const;
    void setWindowLockedChanged();
    /*!
     * Lock current desktop lrc state changed.
     */
    void musicSearchButtonSearched();
    void musicResearchButtonSearched(const QString &name);
    void musicIndexWidgetButtonSearched();
    void musicSearchWidgetButtonSearched();
    void musicLrcWidgetButtonSearched();
    void musicSearchRefreshButtonRefreshed();
    void musicVideoWidgetButtonSearched();
    void musicVideoButtonSearched(const QString &name);
    void musicVideoSetPopup(bool popup);
    void musicVideoFullscreen(bool full);
    void musicLrcDisplayAllButtonClicked();

protected:
    void createVideoWidget(bool create);
    void musicButtonStyleClear();

    bool m_lrcDisplayAll;
    QWidget *m_supperClass;
    Ui::MusicApplication *m_ui;
    MusicSettingWidget *m_setting;
    MusicLrcContainerForDesktop *m_musiclrcfordesktop;
    MusicDownloadStatusLabel *m_downloadStatusLabel;
    MusicVideoPlayWidget *m_videoPlayer;

};

#endif // MUSICRIGHTAREAWIDGET_H
