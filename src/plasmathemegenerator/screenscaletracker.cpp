/*
 * SPDX-FileCopyrightText: 2026 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#include "screenscaletracker.h"
#include <QGuiApplication>
#include <QScreen>
#include <QtNumeric>
#include <algorithm>

ScreenScaleTracker::ScreenScaleTracker(QGuiApplication *application)
    : QObject(application), application_(application),
      maximumScale_(calculateMaximumScale(application)) {
    for (auto *screen : application_->screens()) {
        watchScreen(screen);
    }
    connect(application_, &QGuiApplication::screenAdded, this,
            [this](QScreen *screen) {
                watchScreen(screen);
                updateMaximumScale();
            });
    connect(application_, &QGuiApplication::screenRemoved, this,
            [this](QScreen *) { updateMaximumScale(); });
}

int ScreenScaleTracker::maximumScale() const { return maximumScale_; }

int ScreenScaleTracker::calculateMaximumScale(
    const QGuiApplication *application) {
    int scale = 1;
    for (const auto *screen : application->screens()) {
        scale = std::max(scale, qCeil(screen->devicePixelRatio()));
    }
    return scale;
}

void ScreenScaleTracker::updateMaximumScale() {
    const auto scale = calculateMaximumScale(application_);
    if (scale == maximumScale_) {
        return;
    }
    maximumScale_ = scale;
    Q_EMIT maximumScaleChanged(scale);
}

void ScreenScaleTracker::watchScreen(QScreen *screen) {
    const auto screenChanged = [this]() { updateMaximumScale(); };
    connect(screen, &QScreen::physicalDotsPerInchChanged, this, screenChanged);
}
