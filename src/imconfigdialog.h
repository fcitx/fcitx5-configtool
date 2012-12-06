#include <KDialog>
#include <fcitx/addon.h>
#include <QDBusConnection>
#include <fcitx-qt/fcitxqtkeyboardproxy.h>

class KeyboardLayoutWidget;
class KComboBox;
namespace Fcitx {

class ConfigWidget;
class IMConfigDialog: public KDialog
{
    Q_OBJECT
public:
    explicit IMConfigDialog(const QString& imName, const FcitxAddon* addon, QWidget* parent = 0);

private slots:
    void onButtonClicked(KDialog::ButtonCode code);
    void layoutComboBoxChanged();

private:
    QDBusConnection m_connection;
    QString m_imName;
    FcitxQtKeyboardProxy* m_keyboard;
    KComboBox* m_layoutCombobox;
    ConfigWidget* m_configPage;
    FcitxQtKeyboardLayoutList m_layoutList;
    KeyboardLayoutWidget* m_layoutWidget;
};
}