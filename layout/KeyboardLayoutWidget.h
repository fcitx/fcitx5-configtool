#include <QWidget>

struct Doodad;
struct _XkbDesc;
union _XkbDoodad;

typedef enum {
    GKBD_KEYBOARD_DRAWING_ITEM_TYPE_INVALID = 0,
    GKBD_KEYBOARD_DRAWING_ITEM_TYPE_KEY,
    GKBD_KEYBOARD_DRAWING_ITEM_TYPE_KEY_EXTRA,
    GKBD_KEYBOARD_DRAWING_ITEM_TYPE_DOODAD
} GkbdKeyboardDrawingItemType;

struct DrawingItem {
    GkbdKeyboardDrawingItemType type;
    int originX;
    int originY;
    int angle;
    uint priority;
};

struct Doodad : public DrawingItem{
    union _XkbDoodad* doodad;
    int on;
};

struct DrawingKey : public DrawingItem{
    struct _XkbKey *xkbkey;
    bool pressed;
    uint keycode;
};

class KeyboardLayoutWidget : public QWidget {
    Q_OBJECT
public:
    explicit KeyboardLayoutWidget(QWidget* parent = 0);

protected:
    virtual void paintEvent(QPaintEvent* event);
    void init();
    void alloc();
    void initInicatorDoodad(union _XkbDoodad* xkbdoodad, Doodad& doodad);
    uint findKeycode(const char* keyName);
    void rotateRectangle(int origin_x,
                         int origin_y,
                         int x,
                         int y,
                         int angle,
                         int& rotated_x, int& rotated_y);
    bool parseXkbColorSpec(char* colorspec, QColor& color);
    void initColors();

private:
    QList<DrawingItem*> keyboardItems;
    DrawingKey* keys;
    QList<Doodad*> physicalIndicators;
    struct _XkbDesc* xkb;
    unsigned int l3mod;
    int physicalIndicatorsSize;
    bool xkbOnDisplay;
    QColor* colors;
};