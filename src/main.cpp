#include <KGenericFactory>
#include <KPluginFactory>

#include "Module.h"

K_PLUGIN_FACTORY(KcmFcitxFactory,
                 registerPlugin<Module>();)
K_EXPORT_PLUGIN(KcmFcitxFactory("kcm_fcitx"))