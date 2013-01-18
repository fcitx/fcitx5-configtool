#include <KDialog>
#include <fcitx/addon.h>
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
    QString m_imName;
    KComboBox* m_layoutCombobox;
    ConfigWidget* m_configPage;
    FcitxQtKeyboardLayoutList m_layoutList;
    KeyboardLayoutWidget* m_layoutWidget;
};
}
