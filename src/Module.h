/*
  Copyright © 2010 Harald Sitter <apachelogger@ubuntu.com>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License as
  published by the Free Software Foundation; either version 2 of
  the License or (at your option) version 3 or any later version
  accepted by the membership of KDE e.V. (or its successor approved
  by the membership of KDE e.V.), which shall act as a proxy
  defined in Section 14 of version 3 of the license.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef MODULE_H
#define MODULE_H

#include <KCModule>
#include <fcitx-utils/utarray.h>

class ConfigDescManager;
class FcitxConfigPage;
class FcitxAddonSelector;

namespace Ui {
    class Module;
}

class QFile;
class QRadioButton;

class Module : public KCModule
{
    Q_OBJECT
public:
    /**
     * Constructor.
     *
     * @param parent Parent widget of the module
     * @param args Arguments for the module
     */
    Module(QWidget *parent, const QVariantList &args = QVariantList());

    /**
     * Destructor.
     */
    ~Module();

    /**
     * Overloading the KCModule load() function.
     */
    void load();

    /**
     * Overloading the KCModule save() function.
     *
     * Saving a script file exporting QT_GRAPHICSSYSTEM to KDE's env directory
     * which gets included by startkde at startup.
     *
     * The file will only be created if:
     *   * A radio button is checked
     *   * The checked button is NOT the same as the one probed @see probedButton
     *
     * If there is no button selected the script file will be removed.
     */
    void save();

    /**
     * Overloading the KCModule defaults() function.
     *
     * Setting all radio buttons to unchecked, which indicates for save() to
     * restore to system default (i.e. remove the script file).
     */
    void defaults();
    
    ConfigDescManager* configDescManager();

private:
    /**
     * UI
     */
    Ui::Module *ui;

    /**
     * The scriptFile to manipulate.
     */
    QFile *scriptFile;
    
    /**
     * Addon Selector
     */
    FcitxAddonSelector* addonSelector;
    
    /**
     * addon array
     */
    UT_array* m_addons;
    FcitxConfigPage* m_configPage;
    ConfigDescManager* m_configDescManager;
};

#endif // MODULE_H
