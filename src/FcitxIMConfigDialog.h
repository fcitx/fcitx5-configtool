#include <KDialog>
#include <fcitx/addon.h>
#include "org.fcitx.Fcitx.Keyboard.h"

class KComboBox;
namespace Fcitx {

class FcitxConfigPage;
class FcitxIMConfigDialog: public KDialog
{
    Q_OBJECT
public:
    explicit FcitxIMConfigDialog(const QString& imName, const FcitxAddon* addon, QWidget* parent = 0);

private slots:
    void onButtonClicked(KDialog::ButtonCode code);

private:
    QDBusConnection m_connection;
    QString m_imName;
    org::fcitx::Fcitx::Keyboard* m_keyboard;
    KComboBox* m_layoutCombobox;
    FcitxConfigPage* m_configPage;
    FcitxLayoutList m_layoutList;
};
}