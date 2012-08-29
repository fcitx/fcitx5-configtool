#ifndef FONTDIALOG_H
#define FONTDIALOG_H

#include <KDialog>
class FontDialog : public KDialog
{
    Q_OBJECT
public:
    FontDialog(QWidget* parent = 0);
};

#endif // FONTDIALOG_H