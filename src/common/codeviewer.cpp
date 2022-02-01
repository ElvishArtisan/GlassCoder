// codeviewer.cpp
//
// Text viewer dialog
//
//   (C) Copyright 2015-2022 Fred Gleason <fredg@paravelsystems.com>
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License version 2 as
//   published by the Free Software Foundation.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public
//   License along with this program; if not, write to the Free Software
//   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//

#include "codeviewer.h"

CodeViewer::CodeViewer(const QString &caption,QWidget *parent)
  : QDialog(parent)
{
  view_caption=caption;

  setWindowTitle(view_caption+" - "+tr("View Code"));

  QFont button_font(font().family(),font().pointSize(),QFont::Bold);

  view_text=new QTextEdit(this);
  view_text->setReadOnly(true);

  view_close_button=new QPushButton(tr("Close"),this);
  view_close_button->setFont(button_font);
  connect(view_close_button,SIGNAL(clicked()),this,SLOT(closeData()));
}


QSize CodeViewer::sizeHint() const
{
  return QSize(600,400);
}


int CodeViewer::exec(const QString &str)
{
  view_text->setText(str);
  return QDialog::exec();
}


int CodeViewer::exec(const QStringList &strs)
{
  view_text->setText(strs.join(" \\\n"));
  return QDialog::exec();
}


void CodeViewer::resizeEvent(QResizeEvent *e)
{
  view_text->setGeometry(10,10,size().width()-20,size().height()-55);
  view_close_button->setGeometry(size().width()-70,size().height()-35,60,25);
}


void CodeViewer::closeData()
{
  done(0);
}
