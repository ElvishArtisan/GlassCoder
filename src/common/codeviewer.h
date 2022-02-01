// codeviewer.h
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

#ifndef CODEVIEWER_H
#define CODEVIEWER_H

#include <QPushButton>
#include <QDialog>
#include <QString>
#include <QStringList>
#include <QTextEdit>

class CodeViewer : public QDialog
{
 Q_OBJECT;
 public:
 CodeViewer(const QString &caption,QWidget *parent);
  QSize sizeHint() const;

 public slots:
  int exec(const QString &str);
  int exec(const QStringList &strs);

 protected:
  void resizeEvent(QResizeEvent *e);

 private slots:
  void closeData();

 private:
  QTextEdit *view_text;
  QPushButton *view_close_button;
  QString view_caption;
};


#endif  // CODEVIEWER_H
