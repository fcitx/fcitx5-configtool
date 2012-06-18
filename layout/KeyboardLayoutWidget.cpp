#include "KeyboardLayoutWidget.h"

#include <QX11Info>
#include <QDebug>

#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>
#include <X11/extensions/XKBgeom.h>

#include <math.h>

#define INVALID_KEYCODE ((uint)(-1))

struct DrawingItemCompare {
    bool operator() (const DrawingItem* a, const DrawingItem* b)
    {
        return a->priority < b->priority;
    }
};

KeyboardLayoutWidget::KeyboardLayoutWidget(QWidget* parent): QWidget(parent)
{
    xkb = XkbGetKeyboard (QX11Info::display(),
                          XkbGBN_GeometryMask |
                          XkbGBN_KeyNamesMask |
                          XkbGBN_OtherNamesMask |
                          XkbGBN_SymbolsMask |
                          XkbGBN_IndicatorMapMask,
                          XkbUseCoreKbd);

    if (!xkb)
        return;


    XkbGetNames (QX11Info::display(), XkbAllNamesMask, xkb);

    l3mod = XkbKeysymToModifiers (QX11Info::display(),
                                  XK_ISO_Level3_Shift);

    XkbSelectEventDetails (QX11Info::display(), XkbUseCoreKbd,
                   XkbIndicatorStateNotify,
                   xkb->indicators->phys_indicators,
                   xkb->indicators->phys_indicators);

    xkbOnDisplay = true;

    alloc ();

    int mask = (XkbStateNotifyMask | XkbNamesNotifyMask |
    XkbControlsNotifyMask | XkbIndicatorMapNotifyMask |
    XkbNewKeyboardNotifyMask);
    XkbSelectEvents (QX11Info::display(), XkbUseCoreKbd, mask, mask);


    mask = XkbGroupStateMask | XkbModifierStateMask;
    XkbSelectEventDetails (QX11Info::display(), XkbUseCoreKbd,
                   XkbStateNotify, mask, mask);

    mask = (XkbGroupNamesMask | XkbIndicatorNamesMask);
    XkbSelectEventDetails (QX11Info::display(), XkbUseCoreKbd,
                   XkbNamesNotify, mask, mask);

    init();
    initColors();
}

void KeyboardLayoutWidget::alloc()
{
    physicalIndicatorsSize = xkb->indicators->phys_indicators + 1;
    physicalIndicators.reserve(physicalIndicatorsSize);
    for (int i = 0; i < physicalIndicatorsSize; i++)
        physicalIndicators << NULL;

    keys = new DrawingKey[xkb->max_key_code + 1];
}

void KeyboardLayoutWidget::init()
{
    int i, j, k;
    int x, y;

    if (!xkb)
        return;

    for (i = 0; i < xkb->geom->num_doodads; i++) {
        XkbDoodadRec *xkbdoodad = xkb->geom->doodads + i;
        Doodad* doodad = new Doodad;

        doodad->type = GKBD_KEYBOARD_DRAWING_ITEM_TYPE_DOODAD;
        doodad->originX = 0;
        doodad->originY = 0;
        doodad->angle = 0;
        doodad->priority = xkbdoodad->any.priority * 256 * 256;
        doodad->doodad = xkbdoodad;

        initInicatorDoodad (xkbdoodad, *doodad);

        keyboardItems << doodad;
    }

    for (i = 0; i < xkb->geom->num_sections; i++) {
        XkbSectionRec *section = xkb->geom->sections + i;
        uint priority;

        qDebug() << "initing section " << i << " containing " << section->num_rows << " rows\n";

        x = section->left;
        y = section->top;
        priority = section->priority * 256 * 256;

        for (j = 0; j < section->num_rows; j++) {
            XkbRowRec *row = section->rows + j;

            qDebug() << "  initing row " << j;

            x = section->left + row->left;
            y = section->top + row->top;

            for (k = 0; k < row->num_keys; k++) {
                XkbKeyRec *xkbkey = row->keys + k;
                DrawingKey* key;
                XkbShapeRec *shape =
                    xkb->geom->shapes +
                    xkbkey->shape_ndx;
                uint keycode = findKeycode (xkbkey->name.name);

                if (keycode == INVALID_KEYCODE)
                    continue;

                qDebug() << "    initing key " << k << ", shape: "
                         << shape << "(" << xkb->geom->shapes <<" + " << xkbkey->shape_ndx << "), code: " << keycode;

                if (row->vertical)
                    y += xkbkey->gap;
                else
                    x += xkbkey->gap;

                if (keycode >= xkb->min_key_code
                    && keycode <=
                    xkb->max_key_code) {
                    key = &keys[keycode];
                    if (key->type ==
                        GKBD_KEYBOARD_DRAWING_ITEM_TYPE_INVALID)
                    {
                        key->type =
                            GKBD_KEYBOARD_DRAWING_ITEM_TYPE_KEY;
                    } else {
                        /* duplicate key for the same keycode,
                           already defined as GKBD_KEYBOARD_DRAWING_ITEM_TYPE_KEY */
                        key = new DrawingKey;

                        key->type =
                            GKBD_KEYBOARD_DRAWING_ITEM_TYPE_KEY_EXTRA;
                    }
                } else {
                    key = new DrawingKey;
                    key->type =
                        GKBD_KEYBOARD_DRAWING_ITEM_TYPE_KEY_EXTRA;
                }

                key->xkbkey = xkbkey;
                key->angle = section->angle;
                rotateRectangle (section->left,
                           section->top, x, y,
                           section->angle,
                           key->originX,
                           key->originY);
                key->priority = priority;
                key->keycode = keycode;

                keyboardItems << key;

                if (row->vertical)
                    y += shape->bounds.y2;
                else
                    x += shape->bounds.x2;

                priority++;
            }
        }

        for (j = 0; j < section->num_doodads; j++) {
            XkbDoodadRec *xkbdoodad = section->doodads + j;
            Doodad *doodad = new Doodad;

            doodad->type =
                GKBD_KEYBOARD_DRAWING_ITEM_TYPE_DOODAD;
            doodad->originX = x;
            doodad->originY = y;
            doodad->angle = section->angle;
            doodad->priority =
                priority + xkbdoodad->any.priority;
            doodad->doodad = xkbdoodad;

            initInicatorDoodad (xkbdoodad, *doodad);

            keyboardItems << doodad;
        }
    }

    qSort(keyboardItems.begin(), keyboardItems.end(), DrawingItemCompare());
}

void KeyboardLayoutWidget::initColors()
{
    bool result;
    int i;

    if (!xkb)
        return;

    colors = new QColor[xkb->geom->num_colors];

    for (i = 0; i < xkb->geom->num_colors; i++) {
        result =
            parseXkbColorSpec (xkb->geom->colors[i].
                      spec, colors[i]);

        if (!result)
            qWarning() << "init_colors: unable to parse color " << xkb->geom->colors[i].spec;
    }
}


/* see PSColorDef in xkbprint */
bool
KeyboardLayoutWidget::parseXkbColorSpec (char* colorspec, QColor& color)
{
    long level;

    color.setAlphaF(1);
    if (strcasecmp (colorspec, "black") == 0) {
        color = Qt::black;
    } else if (strcasecmp (colorspec, "white") == 0) {
        color = Qt::white;
    } else if (strncasecmp (colorspec, "grey", 4) == 0 ||
           strncasecmp (colorspec, "gray", 4) == 0) {
        level = strtol (colorspec + 4, NULL, 10);

        color.setRedF(1.0 - level / 100.0);
        color.setGreenF(1.0 - level / 100.0);
        color.setBlueF(1.0 - level / 100.0);
    } else if (strcasecmp (colorspec, "red") == 0) {
        color = Qt::red;
    } else if (strcasecmp (colorspec, "green") == 0) {
        color = Qt::green;
    } else if (strcasecmp (colorspec, "blue") == 0) {
        color = Qt::blue;
    } else if (strncasecmp (colorspec, "red", 3) == 0) {
        level = strtol (colorspec + 3, NULL, 10);

        color.setRedF(level / 100.0);
        color.setGreenF(0);
        color.setBlueF(0);
    } else if (strncasecmp (colorspec, "green", 5) == 0) {
        level = strtol (colorspec + 5, NULL, 10);

        color.setRedF(0);
        color.setGreenF(level / 100.0);
        color.setBlueF(0);
    } else if (strncasecmp (colorspec, "blue", 4) == 0) {
        level = strtol (colorspec + 4, NULL, 10);

        color.setRedF(0);
        color.setGreenF(0);
        color.setBlueF(level / 100.0);
    } else
        return false;

    return true;
}


uint KeyboardLayoutWidget::findKeycode(const char* keyName)
{
#define KEYSYM_NAME_MAX_LENGTH 4
    uint keycode;
    int i, j;
    XkbKeyNamePtr pkey;
    XkbKeyAliasPtr palias;
    uint is_name_matched;
    const char *src, *dst;

    if (!xkb)
        return INVALID_KEYCODE;

    pkey = xkb->names->keys + xkb->min_key_code;
    for (keycode = xkb->min_key_code;
         keycode <= xkb->max_key_code; keycode++) {
        is_name_matched = 1;
        src = keyName;
        dst = pkey->name;
        for (i = KEYSYM_NAME_MAX_LENGTH; --i >= 0;) {
            if ('\0' == *src)
                break;
            if (*src++ != *dst++) {
                is_name_matched = 0;
                break;
            }
        }
        if (is_name_matched) {
            return keycode;
        }
        pkey++;
    }

    palias = xkb->names->key_aliases;
    for (j = xkb->names->num_key_aliases; --j >= 0;) {
        is_name_matched = 1;
        src = keyName;
        dst = palias->alias;
        for (i = KEYSYM_NAME_MAX_LENGTH; --i >= 0;) {
            if ('\0' == *src)
                break;
            if (*src++ != *dst++) {
                is_name_matched = 0;
                break;
            }
        }

        if (is_name_matched) {
            keycode = findKeycode (palias->real);
            return keycode;
        }
        palias++;
    }

    return INVALID_KEYCODE;
}

void KeyboardLayoutWidget::rotateRectangle(int origin_x, int origin_y, int x, int y, int angle, int& rotated_x, int& rotated_y)
{
    rotated_x =
        origin_x + (x - origin_x) * cos (M_PI * angle / 1800.0) - (y -
                                       origin_y)
        * sin (M_PI * angle / 1800.0);
    rotated_y =
        origin_y + (x - origin_x) * sin (M_PI * angle / 1800.0) + (y -
                                       origin_y)
        * cos (M_PI * angle / 1800.0);
}


void KeyboardLayoutWidget::initInicatorDoodad(XkbDoodadRec * xkbdoodad, Doodad& doodad)
{
    if (!xkb)
        return;

    if (xkbdoodad->any.type == XkbIndicatorDoodad) {
        int index;
        Atom iname = 0;
        Atom sname = xkbdoodad->indicator.name;
        unsigned long phys_indicators =
            xkb->indicators->phys_indicators;
        Atom *pind = xkb->names->indicators;

        for (index = 0; index < XkbNumIndicators; index++) {
            iname = *pind++;
            /* name matches and it is real */
            if (iname == sname
                && (phys_indicators & (1 << index)))
                break;
            if (iname == 0)
                break;
        }
        if (iname == 0)
            return;
        else {
            physicalIndicators[index] = &doodad;
            /* Trying to obtain the real state, but if fail - just assume OFF */
            if (!XkbGetNamedIndicator
                (QX11Info::display(), sname, NULL, &doodad.on,
                 NULL, NULL))
                doodad.on = 0;
        }
    }
}


void KeyboardLayoutWidget::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);
}
