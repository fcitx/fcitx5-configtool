/*
 * Copyright (C) 2017~2017 by CSSlayer
 * wengxt@gmail.com
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of the
 * License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; see the file COPYING. If not,
 * see <http://www.gnu.org/licenses/>.
 */
#ifndef _FCITX_MODULE_H_
#define _FCITX_MODULE_H_

// KDE
#include "dbusprovider.h"
#include "iso639.h"
#include "ui_module.h"
#include <KCModule>

namespace fcitx {

class FcitxQtWatcher;
class FcitxQtControllerProxy;

namespace kcm {

class IMPage;
class ErrorOverlay;
class AddonSelector;
class ConfigWidget;

class Module : public KCModule, public Ui::Module {
    Q_OBJECT

public:
    Module(QWidget *parent, const QVariantList &args = QVariantList());
    ~Module();

    void load() override;
    void save() override;
    void defaults() override;

    const Iso639 &iso639() { return iso639_; }

private:
    DBusProvider *dbus_;
    ErrorOverlay *errorOverlay_;
    IMPage *impage_;
    AddonSelector *addonPage_;
    ConfigWidget *configPage_;
    Iso639 iso639_;
};

} // namespace kcm
} // namespace fcitx

#endif // _FCITX_MODULE_H_
