#include <QDialog>
#include <QDialogButtonBox>
#include <fcitx/addon.h>
#include <fcitxqtkeyboardproxy.h>

class KeyboardLayoutWidget;
class KComboBox;
namespace Fcitx {

class ConfigWidget;
class IMConfigDialog: public QDialog
{
    Q_OBJECT
public:
    explicit IMConfigDialog(const QString& imName, const FcitxAddon* addon, QWidget* parent = 0);

private slots:
    void onButtonClicked(QDialogButtonBox::StandardButton code);
    void layoutComboBoxChanged();

private:
    QString m_imName;
    KComboBox* m_layoutCombobox;
    ConfigWidget* m_configPage;
    FcitxQtKeyboardLayoutList m_layoutList;
#if 0
    KeyboardLayoutWidget* m_layoutWidget;
#endif
};
}
