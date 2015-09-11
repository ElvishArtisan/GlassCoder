// messagewidget.cpp
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

#include <stdio.h>

#include "messagewidget.h"

MessageWidget::MessageWidget(QWidget *parent)
  : QLabel(parent)
{
  msg_hang_timer=new QTimer(this);
  msg_hang_timer->setSingleShot(true);
  connect(msg_hang_timer,SIGNAL(timeout()),this,SLOT(hangData()));
}


MessageWidget::~MessageWidget()
{
  delete msg_hang_timer;
}


void MessageWidget::addMessage(const QString &msg)
{
  msg_messages.push(msg);
  if(!msg_hang_timer->isActive()) {
    msg_hang_timer->start(0);
  }
}


void MessageWidget::hangData()
{
  if(msg_messages.size()>0) {
    setText(msg_messages.front());
    msg_messages.pop();
    msg_hang_timer->start(MESSAGEWIDGET_HANG_TIME);
  }
  else {
    setText("");
  }
}
