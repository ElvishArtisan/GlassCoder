// combobox.h
//
// ComboBox widget for GlassGui
//
//   (C) Copyright 2015 Fred Gleason <fredg@paravelsystems.com>
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

#ifndef COMBOBOX_H
#define COMBOBOX_H

#include <QComboBox>
#include <QKeyEvent>
#include <QVariant>

class ComboBox : public QComboBox
{
 Q_OBJECT;
 public:
  ComboBox(QWidget *parent=0);
  void setReadOnly(bool state);
  QVariant currentItemData(int role=Qt::UserRole);
  bool setCurrentItemData(unsigned val);

 protected:
  void keyPressEvent(QKeyEvent *e);
  void mousePressEvent(QMouseEvent *e);

 private:
  bool box_read_only;
};


#endif  // COMBOBOX_H
