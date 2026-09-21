/*
 * SPDX-FileCopyrightText: 2026 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#ifndef _FCITX5_CONFIGTOOL_SCREEN_SCALE_TRACKER_H_
#define _FCITX5_CONFIGTOOL_SCREEN_SCALE_TRACKER_H_

#include <QObject>

class QGuiApplication;
class QScreen;

class ScreenScaleTracker : public QObject {
    Q_OBJECT
public:
    explicit ScreenScaleTracker(QGuiApplication *application);

    static int calculateMaximumScale(const QGuiApplication *application);

    int maximumScale() const;

Q_SIGNALS:
    void maximumScaleChanged(int scale);

private:
    void updateMaximumScale();
    void watchScreen(QScreen *screen);

    QGuiApplication *application_;
    int maximumScale_;
};

#endif // _FCITX5_CONFIGTOOL_SCREEN_SCALE_TRACKER_H_
