#include <QX11Info>
#include <QPaintEvent>
#include <QPainter>
#include <QDebug>
#include <QVector2D>
#include <QApplication>
#include <qmath.h>
#include <QDir>

#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>
#include <X11/extensions/XKBgeom.h>
#include <X11/extensions/XKBrules.h>
#include <X11/extensions/XKBstr.h>

#include <math.h>
#include <fcitx-config/hotkey.h>

#include "config.h"
#include "KeyboardLayoutWidget.h"
#include "deadmapdata.h"

#define INVALID_KEYCODE ((uint)(-1))
#define TEXT_SIZE (0.8)

#ifndef XKB_RULES_XML_FILE
#define XKB_RULES_XML_FILE "/usr/share/X11/xkb/rules/evdev.xml"
#endif

struct DrawingItemCompare {
    bool operator() (const DrawingItem* a, const DrawingItem* b)
    {
        return a->priority < b->priority;
    }
};
static KeyboardDrawingGroupLevel defaultGroupsLevels[] = {
  {0, 1},
  {0, 3},
  {0, 0},
  {0, 2}
};

static KeyboardDrawingGroupLevel *pGroupsLevels[] = {
    defaultGroupsLevels,
    defaultGroupsLevels + 1,
    defaultGroupsLevels + 2,
    defaultGroupsLevels + 3
};


static QString FcitxXkbGetRulesName()
{
    XkbRF_VarDefsRec vd;
    char *tmp = NULL;

    if (XkbRF_GetNamesProp(QX11Info::display(), &tmp, &vd) && tmp != NULL ) {
        return tmp;
    }

    return QString();
}
static QString FcitxXkbFindXkbRulesFile()
{
    QString rulesFile;
    QString rulesName = FcitxXkbGetRulesName();

    if ( rulesName.isNull() ) {
        QString xkbParentDir;

        const QString base = XLIBDIR;

        int count = base.count('/');

        if( count >= 3 ) {
            // .../usr/lib/X11 -> /usr/share/X11/xkb vs .../usr/X11/lib -> /usr/X11/share/X11/xkb
            const char* delta = base.endsWith("X11") ? "/../../share/X11" : "/../share/X11";
            QString dirPath = base + delta;

            QDir dir(dirPath);
            if( dir.exists() ) {
                xkbParentDir = dir.canonicalPath();
            }
            else {
                dirPath = dirPath + "/X11";
                dir = QDir(dirPath);
                if( dir.exists() ) {
                    xkbParentDir = dir.canonicalPath();
                }
            }
        }

        if( xkbParentDir.isEmpty() ) {
            xkbParentDir = "/usr/share/X11";
        }

        rulesFile = QString("%1/xkb/rules/%2.xml").arg(xkbParentDir).arg(rulesName);
    }

    if (rulesFile.isNull())
        rulesFile = XKB_RULES_XML_FILE;

    return rulesFile;
}


KeyboardLayoutWidget::KeyboardLayoutWidget(QWidget* parent): QWidget(parent),
    ratio(1.0),
    trackModifiers(false )
{
    uint i = 0;
    for (i = 0; i < sizeof(deadMapData) / sizeof(deadMapData[0]); i ++)
        deadMap[deadMapData[i].dead] = deadMapData[i].nondead;

    xkb = XkbGetKeyboard (QX11Info::display(),
                          XkbGBN_GeometryMask |
                          XkbGBN_KeyNamesMask |
                          XkbGBN_OtherNamesMask |
                          XkbGBN_SymbolsMask |
                          XkbGBN_IndicatorMapMask,
                          XkbUseCoreKbd);

    if (!xkb)
        return;

    groupLevels = pGroupsLevels;

    XkbGetNames (QX11Info::display(), XkbAllNamesMask, xkb);

    l3mod = XkbKeysymToModifiers (QX11Info::display(),
                                  XK_ISO_Level3_Shift);

    XkbSelectEventDetails (QX11Info::display(), XkbUseCoreKbd,
                   XkbIndicatorStateNotify,
                   xkb->indicators->phys_indicators,
                   xkb->indicators->phys_indicators);

    xkbOnDisplay = true;

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


    alloc ();
    init();
    initColors();

    setFocusPolicy(Qt::StrongFocus);
}

void KeyboardLayoutWidget::setGroup(int group)
{

}


static bool
FcitxXkbInitDefaultLayout (QString& model, QString& option)
{
    Display* dpy = QX11Info::display();
    XkbRF_VarDefsRec vd;
    char *tmp = NULL;

    if (!XkbRF_GetNamesProp(dpy, &tmp, &vd) || !tmp)
        return false;
    if (!vd.model || !vd.layout)
        return false;
    if (vd.model)
        model = vd.model;
    else
        model = QString();
    if (vd.options)
        option = vd.options;
    else
        option = QString();
    return true;
}

void KeyboardLayoutWidget::setLayout(const QString& layout, const QString& variant)
{
    XkbRF_VarDefsRec rdefs;
    XkbComponentNamesRec rnames;
    QString rulesPath = "./rules/evdev";
    char c[] = "C";
    XkbRF_RulesPtr rules = XkbRF_Load (rulesPath.toLocal8Bit().data(), c, True, True);
    if (rules == NULL) {
        rulesPath = FcitxXkbFindXkbRulesFile();
        if (rulesPath.endsWith(".xml")) {
            rulesPath.chop(4);
        }
        rules = XkbRF_Load (rulesPath.toLocal8Bit().data(), c, True, True);
    }
    if (rules == NULL) {
        return;
    }
    memset (&rdefs, 0, sizeof (XkbRF_VarDefsRec));
    memset (&rnames, 0, sizeof (XkbComponentNamesRec));
    QString model, option;
    if (!FcitxXkbInitDefaultLayout(model, option))
        return;

    rdefs.model = !model.isNull() ? strdup(model.toUtf8().data()) : NULL;
    rdefs.layout =  !layout.isNull() ? strdup(layout.toUtf8().data()) : NULL;
    rdefs.variant =  !variant.isNull() ? strdup(variant.toUtf8().data()) : NULL;
    rdefs.options =  !option.isNull() ? strdup(option.toUtf8().data()) : NULL;
    XkbRF_GetComponents (rules, &rdefs, &rnames);
    free (rdefs.model);
    free (rdefs.layout);
    free (rdefs.variant);
    free (rdefs.options);

    setKeyboard(&rnames);
}

void KeyboardLayoutWidget::setKeyboard(XkbComponentNamesPtr names)
{
    release();
    if (xkb)
        XkbFreeKeyboard(xkb, 0, TRUE);
    if (names) {
        xkb = XkbGetKeyboardByName (QX11Info::display(), XkbUseCoreKbd,
                      names, 0,
                      XkbGBN_GeometryMask |
                      XkbGBN_KeyNamesMask |
                      XkbGBN_OtherNamesMask |
                      XkbGBN_ClientSymbolsMask |
                      XkbGBN_IndicatorMapMask, FALSE);
        xkbOnDisplay = FALSE;
    } else {
        xkb = XkbGetKeyboard (QX11Info::display(),
                           XkbGBN_GeometryMask |
                           XkbGBN_KeyNamesMask |
                           XkbGBN_OtherNamesMask |
                           XkbGBN_SymbolsMask |
                           XkbGBN_IndicatorMapMask,
                           XkbUseCoreKbd);
        XkbGetNames (QX11Info::display(), XkbAllNamesMask, xkb);
        xkbOnDisplay = TRUE;
    }

    if (xkb == NULL)
        return;

    alloc ();
    init();
    initColors();
    generatePixmap(true);
    repaint();
}

void KeyboardLayoutWidget::alloc()
{
    physicalIndicators.clear();
    physicalIndicatorsSize = xkb->indicators->phys_indicators + 1;
    physicalIndicators.reserve(physicalIndicatorsSize);
    for (int i = 0; i < physicalIndicatorsSize; i++)
        physicalIndicators << NULL;

    keys = new DrawingKey[xkb->max_key_code + 1];
}

void KeyboardLayoutWidget::release()
{
    physicalIndicators.clear();
    physicalIndicatorsSize = 0;
    delete[] keys;
    delete[] colors;
    foreach(DrawingItem* item, keyboardItems) {
        switch (item->type) {
            case KEYBOARD_DRAWING_ITEM_TYPE_INVALID:
            case KEYBOARD_DRAWING_ITEM_TYPE_KEY:
                break;
            case KEYBOARD_DRAWING_ITEM_TYPE_KEY_EXTRA:
            case KEYBOARD_DRAWING_ITEM_TYPE_DOODAD:
                delete item;
                break;
        }
    }
    keyboardItems.clear();
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

        doodad->type = KEYBOARD_DRAWING_ITEM_TYPE_DOODAD;
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

        // qDebug() << "initing section " << i << " containing " << section->num_rows << " rows\n";

        x = section->left;
        y = section->top;
        priority = section->priority * 256 * 256;

        for (j = 0; j < section->num_rows; j++) {
            XkbRowRec *row = section->rows + j;

            // qDebug() << "  initing row " << j;

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

                // qDebug() << "    initing key " << k << ", shape: "
                //         << shape << "(" << xkb->geom->shapes <<" + " << xkbkey->shape_ndx << "), code: " << keycode;

                if (row->vertical)
                    y += xkbkey->gap;
                else
                    x += xkbkey->gap;

                if (keycode >= xkb->min_key_code
                    && keycode <=
                    xkb->max_key_code) {
                    key = &keys[keycode];
                    if (key->type ==
                        KEYBOARD_DRAWING_ITEM_TYPE_INVALID)
                    {
                        key->type =
                            KEYBOARD_DRAWING_ITEM_TYPE_KEY;
                    } else {
                        /* duplicate key for the same keycode,
                           already defined as KEYBOARD_DRAWING_ITEM_TYPE_KEY */
                        key = new DrawingKey;

                        key->type =
                            KEYBOARD_DRAWING_ITEM_TYPE_KEY_EXTRA;
                    }
                } else {
                    key = new DrawingKey;
                    key->type =
                        KEYBOARD_DRAWING_ITEM_TYPE_KEY_EXTRA;
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
                KEYBOARD_DRAWING_ITEM_TYPE_DOODAD;
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

void KeyboardLayoutWidget::focusOutEvent(QFocusEvent* event)
{
    bool update = false;
    for (int i = xkb->min_key_code; i <= xkb->max_key_code; i ++) {
        if (keys[i].pressed) {
            update = true;
            keys[i].pressed = false;
        }
    }

    if (update) {
        generatePixmap(true);
        repaint();
    }
    QWidget::focusOutEvent(event);
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

void KeyboardLayoutWidget::generatePixmap(bool force)
{
    double ratioX = (double) width() / xkb->geom->width_mm;
    double ratioY = (double) height() / xkb->geom->height_mm;

    ratio = qMin(ratioX, ratioY);

    int w = xkb->geom->width_mm * ratio;
    int h = xkb->geom->height_mm * ratio;
    if (w == image.width() && h == image.height() && !force)
        return;

    image = QImage(w, h, QImage::Format_ARGB32);
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(image.rect(), Qt::transparent);

    foreach(DrawingItem* item , keyboardItems) {
        if (!xkb)
            return;

        switch (item->type) {
        case KEYBOARD_DRAWING_ITEM_TYPE_INVALID:
            break;

        case KEYBOARD_DRAWING_ITEM_TYPE_KEY:
        case KEYBOARD_DRAWING_ITEM_TYPE_KEY_EXTRA:
            drawKey (&painter, (DrawingKey *) item);
            break;

        case KEYBOARD_DRAWING_ITEM_TYPE_DOODAD:
            drawDoodad (&painter, (Doodad *) item);
            break;
        }
    }

    /*
    QVector<QPointF> vec;
    vec << QPointF(20, 20);
    vec << QPointF(200, 20);
    vec << QPointF(200, 200);
    vec << QPointF(20, 200);

    QBrush brush(Qt::gray);
    painter.setBrush(brush);
    roundedPolygon(&painter, false, 20, vec);*/
    /*image.save("/tmp/test.png", "png");*/
}

void KeyboardLayoutWidget::drawKey(QPainter* painter, DrawingKey* key)
{
    XkbShapeRec *shape;
    QColor color;
    XkbOutlineRec *outline;
    int origin_offset_x;
    /* gint i; */

    if (!xkb)
        return;

    shape = xkb->geom->shapes + key->xkbkey->shape_ndx;

    if (key->pressed)
        color = qApp->palette().color(QPalette::Active, QPalette::Highlight);
    else
        color = colors[key->xkbkey->color_ndx];

    /* draw the primary outline */
    outline = shape->primary ? shape->primary : shape->outlines;
    drawOutline (painter, outline, color, key->angle, key->originX, key->originY);
#if 0
    /* don't draw other outlines for now, since
     * the text placement does not take them into account
     */
    for (i = 0; i < shape->num_outlines; i++) {
        if (shape->outlines + i == shape->approx ||
            shape->outlines + i == shape->primary)
            continue;
        draw_outline (context, shape->outlines + i, NULL,
                  key->angle, key->origin_x, key->origin_y);
    }
#endif
    origin_offset_x = calcShapeOriginOffsetX (outline);
    drawKeyLabel (painter, key->keycode, key->angle,
            key->originX + origin_offset_x, key->originY,
            shape->bounds.x2, shape->bounds.y2, key->pressed);
}

int KeyboardLayoutWidget::calcShapeOriginOffsetX(XkbOutlineRec* outline)
{
    int rv = 0;
    int i;
    XkbPointPtr point = outline->points;
    if (outline->num_points < 3)
        return 0;
    for (i = outline->num_points; --i > 0;) {
        int x1 = point->x;
        int y1 = point++->y;
        int x2 = point->x;
        int y2 = point->y;

        /*vertical, bottom to top (clock-wise), on the left */
        if ((x1 == x2) && (y1 > y2) && (x1 > rv)) {
            rv = x1;
        }
    }
    return rv;
}

void KeyboardLayoutWidget::drawOutline(QPainter* painter, XkbOutlinePtr outline, QColor color, int angle, int originX, int originY)
{
    if (outline->num_points == 1) {
        if (color.isValid())
            drawRectangle (painter, color, angle, originX,
                    originY, outline->points[0].x,
                    outline->points[0].y,
                    outline->corner_radius);

        drawRectangle (painter, QColor(), angle, originX,
                originY, outline->points[0].x,
                outline->points[0].y,
                outline->corner_radius);
    } else if (outline->num_points == 2) {
        int rotated_x0, rotated_y0;

        //qDebug() << "angle" << angle ;
        rotateCoordinate (originX, originY,
                   originX + outline->points[0].x,
                   originY + outline->points[0].y,
                   angle, &rotated_x0, &rotated_y0);
        if (color.isValid())
            drawRectangle (painter, color, angle, rotated_x0,
                    rotated_y0, outline->points[1].x,
                    outline->points[1].y,
                    outline->corner_radius);
        drawRectangle (painter, QColor(), angle, rotated_x0,
                rotated_y0, outline->points[1].x,
                outline->points[1].y,
                outline->corner_radius);
    } else {
        if (color.isValid())
            drawPolygon (painter, color, originX, originY,
                      outline->points, outline->num_points,
                      outline->corner_radius);
        drawPolygon (painter, QColor(), originX, originY,
                  outline->points, outline->num_points,
                  outline->corner_radius);
    }
}

void KeyboardLayoutWidget::rotateCoordinate(int originX, int originY, int x, int y, int angle, int* rotated_x, int* rotated_y)
{
    QTransform translate;
    QTransform rotate;
    QTransform translate2;
    QTransform trans;
    translate.translate(-originX, -originY);
    translate2.translate(originX, originY);
    rotate.rotate(angle / 10);
    trans = translate * rotate * translate2;
    trans.map(x, y, rotated_x, rotated_y);
}

int
KeyboardLayoutWidget::xkbToPixmapCoord (int n)
{
    return n * ratio;
}

double
KeyboardLayoutWidget::xkbToPixmapDouble (double d)
{
    return d * ratio;
}

void KeyboardLayoutWidget::drawPolygon(QPainter* painter, QColor fill_color, int xkb_x, int xkb_y, XkbPointPtr xkb_points, unsigned int num_points, unsigned int radius)
{
    QVector<QPointF> points;
    bool filled;
    unsigned int i;

    if (fill_color.isValid()) {
        filled = TRUE;
    } else {
        fill_color = Qt::gray;
        filled = FALSE;
    }

    QBrush brush(fill_color);
    painter->save();
    painter->setBrush(brush);


    for (i = 0; i < num_points; i++) {
        QPointF point;
        point.setX(xkbToPixmapCoord (xkb_x + xkb_points[i].x));
        point.setY(xkbToPixmapCoord (xkb_y + xkb_points[i].y));
        points << point;
    }

    roundedPolygon (painter, filled,
             xkbToPixmapDouble (radius),
             points);
    painter->restore();
}

double
distance(double x, double y)
{
    return qSqrt ((x * x) + (y * y));
}

double
distance(const QPointF& a, const QPointF& b)
{
    QPointF d = a - b;
    return distance(d.x(), d.y());
}

double
angle(const QVector2D& norm) {
     qreal result = qAcos(norm.x());
     if (norm.y() > 0)
         result = 2 * M_PI - result;

     return result / M_PI * 180.0;
}

/* draw an angle from the current point to b and then to c,
 * with a rounded corner of the given radius.
 */
void
KeyboardLayoutWidget::roundedCorner (QPainterPath& path,
        QPointF b, QPointF c, double radius)
{
    /* we may have 5 point here
     * a is the current point
     * c is the end point
     * and b is the corner
     * we will have a rounded corner with radious (maybe adjust by a,b,c position)
     *
     * a1 is on a-b, and c1 is on b-c
     */

    QPointF a = path.currentPosition();

    //qDebug() << "current" << a << b << c;

    /* make sure radius is not too large */
    double dist1 = distance (a, b);
    double dist2 = distance (b, c);

    //qDebug() << "dist" << dist1 << dist2 << radius;

    radius = qMin (radius, qMin (dist1, dist2));

    QPointF ba = a - b;
    QPointF bc = c - b;
    QVector2D na(ba);
    QVector2D nc(bc);
    na.normalize();
    nc.normalize();

    qreal cosine = QVector2D::dotProduct(na, nc);
    qreal halfcosine = qSqrt((1 + cosine) / 2);
    qreal halfsine = qSqrt( 1- halfcosine * halfcosine);
    qreal halftan = halfsine / halfcosine;
    QPointF a1 = b + na.toPointF() * (radius / halftan);
    QPointF c1 = b + nc.toPointF() * (radius / halftan);

    QVector2D n = na + nc;
    n.normalize();
    QPointF ctr = b + n.toPointF() * radius / halfsine;
    QRectF arcRect(ctr.x() - radius, ctr.y() - radius, 2 * radius, 2 * radius);

    qreal phiA, phiC;
    //qDebug() << c1 << ctr << a1;
    QVector2D ctra = QVector2D(a1 - ctr);
    QVector2D ctrc = QVector2D(c1 - ctr);
    ctra.normalize();
    ctrc.normalize();
    phiA = angle(ctra);
    phiC = angle(ctrc);

    qreal delta = phiC - phiA;
    while (delta > 0)
        delta -= 360;

    while (delta < -360)
        delta += 360;

    if (delta <- 180)
        delta += 360;

    //qDebug() << arcRect << ctra << ctrc << ctr << "degree" << phiA << phiC;

    path.lineTo(a1);
    path.arcTo(arcRect, phiA, delta);
    path.lineTo(c1);
    path.lineTo(c);
}

void
KeyboardLayoutWidget::roundedPolygon(QPainter* painter, bool filled, double radius, const QVector< QPointF >& points)
{
    int i, j;

    QPainterPath path;

    path.moveTo((points[points.size() - 1] +
                  points[0]) / 2);

    for (i = 0; i < points.size(); i++) {
        j = (i + 1) % points.size();
        roundedCorner (path, points[i],
                (points[i] + points[j]) / 2,
                radius);
        // qDebug() << "corner " << points[i] << points[j];
    };
    path.closeSubpath();

    if (filled) {
        painter->fillPath(path, painter->brush());
    }
    else {
        painter->drawPath(path);
    }
}

void KeyboardLayoutWidget::drawRectangle(QPainter* painter, QColor color, int angle, int xkb_x, int xkb_y, int xkb_width, int xkb_height, unsigned int radius)
{
    if (angle == 0) {
        int x, y, width, height;
        bool filled;

        if (color.isValid()) {
            filled = TRUE;
        } else {
            color = Qt::gray;
            filled = FALSE;
        }

        x = xkbToPixmapCoord (xkb_x);
        y = xkbToPixmapCoord (xkb_y);
        width =
            xkbToPixmapCoord (xkb_x + xkb_width) - x;
        height =
            xkbToPixmapCoord (xkb_y + xkb_height) - y;

        drawCurveRectangle (painter, filled, color,
                      x, y, width, height,
                      xkbToPixmapDouble (radius));
    } else {
        XkbPointRec points[4];
        int x, y;

        points[0].x = xkb_x;
        points[0].y = xkb_y;
        rotateCoordinate (xkb_x, xkb_y, xkb_x + xkb_width, xkb_y,
                   angle, &x, &y);
        points[1].x = x;
        points[1].y = y;
        rotateCoordinate (xkb_x, xkb_y, xkb_x + xkb_width,
                   xkb_y + xkb_height, angle, &x, &y);
        points[2].x = x;
        points[2].y = y;
        rotateCoordinate (xkb_x, xkb_y, xkb_x, xkb_y + xkb_height,
                   angle, &x, &y);
        points[3].x = x;
        points[3].y = y;

        /* the points we've calculated are relative to 0,0 */
        drawPolygon (painter, color, 0, 0, points, 4, radius);
    }
}

void KeyboardLayoutWidget::drawCurveRectangle(QPainter* painter, bool filled, QColor color, int x, int y, int width, int height, double radius)
{
    double x1, y1;

    if (!width || !height)
        return;

    x1 = x + width;
    y1 = y + height;

    radius = qMin (radius, (double) qMin (width / 2, height / 2));

    QPainterPath path;

    path.moveTo(x, y + radius);
    path.arcTo(x, y, 2 * radius, 2 * radius, 180, -90);
    path.lineTo (x1 - radius, y);
    path.arcTo (x1 - 2 * radius, y, 2 * radius, 2 * radius, 90, - 90);
    path.lineTo (x1, y1 - radius);
    path.arcTo (x1 - 2 * radius, y1 - 2 * radius, 2 * radius, 2 * radius, 0, -90);
    path.lineTo (x + radius, y1);
    path.arcTo (x , y1 - 2 * radius, 2 * radius, 2 * radius, -90, -90);
    path.closeSubpath();

    painter->save();
    if (filled) {
        QBrush brush(color);
        painter->fillPath (path, brush);
    }
    else {
        painter->setPen(color);
        painter->drawPath(path);
    }
    painter->restore();
}

KeyboardLayoutWidget::~KeyboardLayoutWidget()
{
    release();
}


void KeyboardLayoutWidget::drawKeyLabel(QPainter* painter, uint keycode, int angle, int xkb_origin_x, int xkb_origin_y, int xkb_width, int xkb_height, bool is_pressed)
{
    int x, y, width, height;
    int padding;
    int g, l, glp;

    if (!xkb)
        return;

    padding = 23 * ratio;   /* 2.3mm */

    x = xkbToPixmapCoord (xkb_origin_x);
    y = xkbToPixmapCoord (xkb_origin_y);
    width = xkbToPixmapCoord (xkb_origin_x + xkb_width) - x;
    height = xkbToPixmapCoord (xkb_origin_y + xkb_height) - y;

    for (glp = KEYBOARD_DRAWING_POS_TOPLEFT;
         glp < KEYBOARD_DRAWING_POS_TOTAL; glp++) {
        if (groupLevels[glp] == NULL)
            continue;
        g = groupLevels[glp]->group;
        l = groupLevels[glp]->level;

        if (g < 0 || g >= XkbKeyNumGroups (xkb, keycode))
            continue;
        if (l < 0
            || l >= XkbKeyGroupWidth (xkb, keycode, g))
            continue;

        /* Skip "exotic" levels like the "Ctrl" level in PC_SYSREQ */
        if (l > 0) {
            uint mods = XkbKeyKeyType (xkb, keycode,
                            g)->mods.mask;
            if ((mods & (ShiftMask | l3mod)) == 0)
                continue;
        }

        if (trackModifiers) {
            uint mods_rtrn;
            KeySym keysym;

            if (XkbTranslateKeyCode (xkb, keycode,
                         XkbBuildCoreState(mods, g),
                         &mods_rtrn, &keysym)) {
                drawKeyLabelHelper (painter, keysym, angle, glp,
                               x, y, width, height,
                               padding,
                               is_pressed);
                /* reverse y order */
            }
        } else {
            KeySym keysym;

            keysym =
                XkbKeySymEntry (xkb, keycode, l, g);

            drawKeyLabelHelper (painter, keysym,
                           angle, glp, x, y, width,
                           height, padding,
                           is_pressed);
            /* reverse y order */
        }
    }
}

void KeyboardLayoutWidget::drawKeyLabelHelper(QPainter* painter, int keysym, int angle, int glp, int x, int y, int width, int height, int padding, bool is_pressed)
{
    if (keysym == 0)
        return;

    if (keysym == XK_VoidSymbol)
        return;

    if (padding >= height / 2)
        padding = 0;
    if (padding >= width / 2)
        padding = 0;

    Qt::Alignment align;

    QRectF rect(padding, padding, (width - 2 * padding), (height - 2 * padding));
    switch (glp) {
    case KEYBOARD_DRAWING_POS_TOPLEFT:
        align = Qt::AlignTop | Qt::AlignLeft;
        break;
    case KEYBOARD_DRAWING_POS_BOTTOMLEFT:
        align = Qt::AlignBottom | Qt::AlignLeft;
        break;
    case KEYBOARD_DRAWING_POS_TOPRIGHT:
        align = Qt::AlignTop | Qt::AlignRight;
        break;
    case KEYBOARD_DRAWING_POS_BOTTOMRIGHT:
        align = Qt::AlignBottom | Qt::AlignRight;
        break;
    default:
        return;
    }

    if (keysym == XK_ISO_Left_Tab)
        keysym = XK_Tab;

    keysym = (int) FcitxHotkeyPadToMain((FcitxKeySym) keysym);
    uint32_t unicode = FcitxKeySymToUnicode((FcitxKeySym) keysym);

    if (deadMap.contains(keysym)) {
        unicode = deadMap[keysym];
    }
    QString text;
    if (unicode
        && QChar(unicode).category() != QChar::Other_Control
        && !QChar(unicode).isSpace())
        text.append(QChar(unicode));
    else {
        if (keysym == XK_Prior) {
            text = "PgUp";
        }
        else if (keysym == XK_Next) {
            text = "PgDn";
        }
        else {
            text = QString(XKeysymToString(keysym));
        }
    }
    if (text != "_") {
        if (text.endsWith("_L") || text.endsWith("_R"))
            text = text.replace('_', ' ');
        else
            text = text.replace('_', '\n');
    }
    painter->save();
    QTransform trans;
    trans.translate(x + padding / 2, y + padding / 2);
    trans.rotate(angle / 10);
    painter->setTransform(trans);
    //painter->fillRect(QRectF(0, 0, width - padding, height - padding), QBrush(Qt::blue));
    // painter->setClipRect(QRect(x + padding / 2, y + padding / 2, width - padding, height - padding));

    trans.reset();
    trans.translate(x, y);
    trans.rotate(angle / 10);
    QFont font = painter->font();
    font.setPixelSize(TEXT_SIZE * (height / 2 - padding));
    QFontMetricsF fm(font);
    QStringList lines = text.split('\n');
    foreach(const QString& line, lines) {
        qreal w = fm.width(line);
        if (w > (width - padding * 2)  * TEXT_SIZE) {
            double sz = font.pixelSize() / w * (width - padding * 2) * TEXT_SIZE;
            if (sz < 1)
                sz = 1;
            font.setPixelSize(font.pixelSize() / w * (width - padding * 2) * TEXT_SIZE);
        }
    }

    painter->setFont(font);
    painter->setTransform(trans);
#if 0
    painter->save();
    painter->setPen(QPen(Qt::red));
    painter->drawRect(rect);
    painter->restore();
#endif
    painter->drawText(rect, align, text);
    painter->restore();
}


void KeyboardLayoutWidget::drawDoodad(QPainter* painter, Doodad* doodad)
{
    switch (doodad->doodad->any.type) {
    case XkbOutlineDoodad:
    case XkbSolidDoodad:
        drawShapeDoodad (painter, doodad,
                   &doodad->doodad->shape);
        break;

    case XkbTextDoodad:
        drawTextDoodad (painter, doodad,
                  &doodad->doodad->text);
        break;

    case XkbIndicatorDoodad:
        drawIndicatorDoodad (painter, doodad,
                       &doodad->doodad->indicator);
        break;

    case XkbLogoDoodad:
        /* g_print ("draw_doodad: logo: %s\n", doodad->doodad->logo.logo_name); */
        /* XkbLogoDoodadRec is essentially a subclass of XkbShapeDoodadRec */
        drawShapeDoodad (painter, doodad,
                   &doodad->doodad->shape);
        break;
    }
}

void KeyboardLayoutWidget::drawShapeDoodad(QPainter* painter, Doodad* doodad, XkbShapeDoodadPtr shapeDoodad)
{
    XkbShapeRec *shape;
    QColor color;
    int i;

    if (!xkb)
        return;

    shape = xkb->geom->shapes + shapeDoodad->shape_ndx;
    color = colors[shapeDoodad->color_ndx];

    /* draw the primary outline filled */
    drawOutline (painter,
              shape->primary ? shape->primary : shape->outlines,
              color, doodad->angle,
              doodad->originX + shapeDoodad->left,
              doodad->originY + shapeDoodad->top);

    /* stroke the other outlines */
    for (i = 0; i < shape->num_outlines; i++) {
        if (shape->outlines + i == shape->approx ||
            shape->outlines + i == shape->primary)
            continue;
        drawOutline (painter, shape->outlines + i, QColor(),
                  doodad->angle,
                  doodad->originX + shapeDoodad->left,
                  doodad->originY + shapeDoodad->top);
    }
}

void KeyboardLayoutWidget::drawTextDoodad(QPainter* painter, Doodad* doodad, XkbTextDoodadPtr textDoodad)
{
    int x, y;
    if (!xkb)
        return;

    x = xkbToPixmapCoord (doodad->originX + textDoodad->left);
    y = xkbToPixmapCoord (doodad->originY + textDoodad->top);

    QRect rect(0, 0, xkbToPixmapDouble(textDoodad->width), xkbToPixmapDouble(textDoodad->height));
    QTransform trans;
    trans.translate(x, y);
    trans.rotate(textDoodad->angle / 10);
    QString text(textDoodad->text);
    int line = text.count('\n') + 1;
    QFont font = painter->font();
    font.setPixelSize((rect.height() / 2));
    QFontMetricsF fm(font);
    qreal h = fm.height() * line;
    if (h > rect.height()) {
        double sz = font.pixelSize() / h * rect.height();
        if (sz < 1)
            sz = 1;
        font.setPixelSize(sz);
    }
    qreal w = fm.width(textDoodad->text);
    if (w > rect.width()) {
        double sz = font.pixelSize() / w * rect.width();
        if (sz < 1)
            sz = 1;
        font.setPixelSize(sz);
    }

    painter->save();
    painter->setFont(font);
    painter->setTransform(trans);
    painter->drawText(rect, Qt::AlignLeft, textDoodad->text);
    painter->restore();
}


void KeyboardLayoutWidget::drawIndicatorDoodad(QPainter* painter, Doodad* doodad, XkbIndicatorDoodadPtr indicatorDoodad)
{
    QColor color;
    XkbShapeRec *shape;
    int i;

    if (!xkb)
        return;

    initInicatorDoodad(doodad->doodad, *doodad);

    shape = xkb->geom->shapes + indicatorDoodad->shape_ndx;

    color = colors[(doodad->on ?
                   indicatorDoodad->on_color_ndx :
                   indicatorDoodad->off_color_ndx)];

    for (i = 0; i < 1; i++)
        drawOutline (painter, shape->outlines + i, color,
                  doodad->angle,
                  doodad->originX + indicatorDoodad->left,
                  doodad->originY + indicatorDoodad->top);
}


void KeyboardLayoutWidget::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);
    QPainter p(this);
    p.setClipRect(event->rect());
    p.setCompositionMode(QPainter::CompositionMode_SourceOver);
    int dx = (rect().width() - image.rect().width()) / 2;
    int dy = (rect().height() - image.rect().height()) / 2;
    QRect r = image.rect();
    r.moveTo(dx, dy);
    p.drawImage(r, image);
}

void KeyboardLayoutWidget::resizeEvent(QResizeEvent* event)
{
    generatePixmap();
    update();
    QWidget::resizeEvent(event);
}

bool KeyboardLayoutWidget::x11Event(XEvent* event)
{
    do {
        if (event->type != KeyPress && event->type != KeyRelease)
            break;
        DrawingKey* key = &keys[event->xkey.keycode];
        if (event->xkey.keycode > xkb->max_key_code ||
            event->xkey.keycode < xkb->min_key_code ||
            key->xkbkey == NULL) {
            break;
        }
        if (event->type == KeyPress && key->pressed)
            break;
        if (event->type == KeyRelease && !key->pressed)
            break;
        key->pressed = (event->type == KeyPress);
        generatePixmap(true);
        repaint();
    } while(0);
    return QWidget::x11Event(event);
}


