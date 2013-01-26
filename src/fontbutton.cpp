/***************************************************************************
 *   Copyright (C) 2012~2012 by CSSlayer                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.              *
 ***************************************************************************/

#include <QDebug>

#include <KFontDialog>

#include "fontbutton.h"
#include "ui_fontbutton.h"

FontButton::FontButton(QWidget* parent): QWidget(parent)
    ,m_ui(new Ui::FontButton)
{
    m_ui->setupUi(this);
    connect(m_ui->fontSelectButton, SIGNAL(clicked(bool)), this, SLOT(selectFont()));
}

FontButton::~FontButton()
{
    delete m_ui;
}

const QFont& FontButton::font()
{
    return m_font;
}

QString FontButton::fontName()
{
    return m_ui->fontPreviewLabel->text();
}

QFont FontButton::parseFont(const QString& string)
{
    QStringList list = string.split(" ", QString::SkipEmptyParts);
    bool bold = false;
    bool italic = false;
    while(!list.empty()) {
        if (list.last() == "Bold") {
            bold = true;
            list.pop_back();
        }
        else if (list.last() == "Italic") {
            italic = true;
            list.pop_back();
        }
        else
            break;
    }
    QString family = list.join(" ");
    QFont font;
    font.setFamily(family);
    font.setBold(bold);
    font.setItalic(italic);
    return font;
}

void FontButton::setFont(const QFont& font)
{
    m_font = font;
    QString style;
#if QT_VERSION >= QT_VERSION_CHECK(4, 8, 0)
    if (!font.styleName().isEmpty()) {
        style = font.styleName();
    }
    else
#endif
    {
        QStringList styles;
        if (font.bold())
            styles << "Bold";
        if (font.italic())
            styles << "Italic";
        style = styles.join(" ");
    }
    m_ui->fontPreviewLabel->setText(QString("%1 %2").arg(m_font.family(), style));
    m_ui->fontPreviewLabel->setFont(m_font);
    if (font.family() != m_font.family()) {
        emit fontChanged(m_font);
    }
}

void FontButton::selectFont()
{
    KDialog dialog(NULL);
    KFontChooser* chooser = new KFontChooser(&dialog);
    chooser->enableColumn(KFontChooser::SizeList, false);
    chooser->setFont(m_font);
    dialog.setMainWidget(chooser);
    dialog.setCaption(i18n("Select Font"));
    dialog.setButtons(KDialog::Ok | KDialog::Cancel);
    dialog.setDefaultButton(KDialog::Ok);
    if (dialog.exec() == KDialog::Accepted) {
        setFont(chooser->font());
    }
}
