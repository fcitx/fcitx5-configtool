/*
* Copyright (C) 2011~2017 by CSSlayer
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

// KDE
#include <KPluginFactory>

// self
#include "module.h"

K_PLUGIN_FACTORY_WITH_JSON(KcmFcitx5Factory, "kcm_fcitx5.json",
                           registerPlugin<fcitx::kcm::Module>();)
K_EXPORT_PLUGIN(KcmFcitx5Factory("kcm_fcitx5"))

#include "main.moc"
