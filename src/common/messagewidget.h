// messagewidget.h
//
// Connection message widget
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

#ifndef MESSAGEWIDGET_H
#define MESSAGEWIDGET_H

#include <queue>

#include <QLabel>
#include <QTimer>

#define MESSAGEWIDGET_HANG_TIME 5000

class MessageWidget : public QLabel
{
 Q_OBJECT;
 public:
  MessageWidget(QWidget *parent=0);
  ~MessageWidget();
  void addMessage(const QString &msg);

 private slots:
  void hangData();

 private:
  std::queue<QString> msg_messages;
  QTimer *msg_hang_timer;
};


#endif  // MESSAGEWIDGET_H
