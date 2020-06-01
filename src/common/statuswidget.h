// statuswidget.h
//
// Connection status widget
//
//   (C) Copyright 2015-2020 Fred Gleason <fredg@paravelsystems.com>
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

#ifndef STATUSWIDGET_H
#define STATUSWIDGET_H

#include <QLabel>
#include <QPalette>

class StatusWidget : public QLabel
{
 Q_OBJECT;
 public:
  StatusWidget(QWidget *parent=0);
  int status() const;
  bool setStatus(int status);

 private:
  int stat_status;
  QString stat_idle_style;
  QString stat_connecting_style;
  QString stat_connected_style;
  QString stat_disconnecting_style;
  QString stat_failed_style;
};


#endif  // STATUSWIDGET_H
