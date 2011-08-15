#ifndef __FCITX_ADDON_SELECTOR_H__
#define __FCITX_ADDON_SELECTOR_H__

#include <QWidget>

class Module;
struct _FcitxAddon;
class FcitxAddonSelector : public QWidget
{
    Q_OBJECT
public:
    FcitxAddonSelector(Module* parent);
    virtual ~FcitxAddonSelector();
    void load();
    void save();
    void addAddon(struct _FcitxAddon* fcitxAddon);
 
Q_SIGNALS:
    void changed(bool hasChanged);
    void configCommitted(const QByteArray& componentName);
    
private:
    class Private;
    Private* d;
    Module* parent;
};

#endif