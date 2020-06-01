// statuswidget.cpp
//
// Connection status widget
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

#include "logging.h"
#include "statuswidget.h"

StatusWidget::StatusWidget(QWidget *parent)
  : QLabel(parent)
{
  setAlignment(Qt::AlignCenter);

  //
  // Create Stylesheets
  //
  stat_idle_style="";
  stat_connected_style="background-color: green;color: lightGray";
  stat_connecting_style="background-color: #BBBB00;color: black";
  stat_disconnecting_style="background-color: #BBBB00;color: black";
  stat_failed_style="background-color: red;color: lightGray";

  setStatus(CONNECTION_IDLE);
}


int StatusWidget::status() const
{
  return stat_status;
}


bool StatusWidget::setStatus(int status)
{
  bool ret=false;

  switch(status) {
  case CONNECTION_IDLE:
    setText(tr("IDLE"));
    setStyleSheet(stat_idle_style);
    ret=true;
    break;

  case CONNECTION_PENDING:
    setText(tr("CONNECTING..."));
    setStyleSheet(stat_connecting_style);
    ret=true;
    break;

  case CONNECTION_STOPPING:
    setText(tr("STOPPING..."));
    setStyleSheet(stat_connecting_style);
    ret=true;
    break;

  case CONNECTION_OK:
    setText(tr("CONNECTED"));
    setStyleSheet(stat_connected_style);
    ret=true;
    break;

  case CONNECTION_FAILED:
    setText(tr("RECONNECTING..."));
    setStyleSheet(stat_failed_style);
    ret=true;
    break;
  }
  stat_status=status;

  return ret;
}
