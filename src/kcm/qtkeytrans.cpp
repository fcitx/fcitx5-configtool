/*
 * SPDX-FileCopyrightText: 2012~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "qtkeytrans.h"
#include "qtkeytransdata.h"
#include <QTextCodec>
#include <ctype.h>
#include <fcitx-utils/key.h>

#define _ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#define _ARRAY_END(a) (a + _ARRAY_SIZE(a))

namespace fcitx {
namespace kcm {

bool qEventToSym(int key, Qt::KeyboardModifiers mod, const QString &text,
                 int &outsym, unsigned int &outstate) {
    int sym = 0;
    fcitx::KeyStates state;
    do {
        if (text.length() <= 0)
            break;
        int uni = text[0].unicode();
        int *result =
            std::lower_bound(unicodeHasKey, _ARRAY_END(unicodeHasKey), uni);
        if (result != _ARRAY_END(unicodeHasKey) && *result == uni) {
            sym = *result + 0x1000000;
            break;
        }

        Unicode2Key *keyMap =
            std::lower_bound(unicodeKeyMap, _ARRAY_END(unicodeKeyMap), uni);
        if (keyMap != _ARRAY_END(unicodeKeyMap) && keyMap->unicode == uni) {
            sym = keyMap->key;
            break;
        }
    } while (0);

    do {
        if (sym)
            break;

        QtCode2Key *result = nullptr;
        if (mod & Qt::KeypadModifier) {
            result = std::lower_bound(keyPadQtCodeToKey,
                                      _ARRAY_END(keyPadQtCodeToKey), key);
            if (result == _ARRAY_END(keyPadQtCodeToKey) ||
                result->qtcode != key)
                result = nullptr;
        } else {
            if (text.isNull()) {
                result = std::lower_bound(qtCodeToKeyBackup,
                                          _ARRAY_END(qtCodeToKeyBackup), key);
                if (result == _ARRAY_END(qtCodeToKeyBackup) ||
                    result->qtcode != key)
                    result = nullptr;
            }
            if (!result) {
                result =
                    std::lower_bound(qtCodeToKey, _ARRAY_END(qtCodeToKey), key);

                if (result == _ARRAY_END(qtCodeToKey) || result->qtcode != key)
                    result = nullptr;
            }

            if (!result) {
                result = std::lower_bound(keyPadQtCodeToKey,
                                          _ARRAY_END(keyPadQtCodeToKey), key);
                if (result == _ARRAY_END(keyPadQtCodeToKey) ||
                    result->qtcode != key)
                    result = nullptr;
            }
        }

        if (result)
            sym = result->key;

    } while (0);

    state = fcitx::KeyState::NoState;

    if (mod & Qt::CTRL)
        state |= fcitx::KeyState::Ctrl;

    if (mod & Qt::ALT)
        state |= fcitx::KeyState::Alt;

    if (mod & Qt::SHIFT)
        state |= fcitx::KeyState::Shift;

    if (mod & Qt::META)
        state |= fcitx::KeyState::Super;

    outsym = sym;
    outstate = state;

    return (outsym > 0);
}

} // namespace kcm
} // namespace fcitx
