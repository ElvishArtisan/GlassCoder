// streamdialog.h
//
// Configuration dialog for stream metadata settings
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

#ifndef STREAMDIALOG_H
#define STREAMDIALOG_H

#include <stdio.h>

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QString>
#include <QStringList>

#include "connector.h"
#include "profile.h"
#include "spinbox.h"

class StreamDialog : public QDialog
{
 Q_OBJECT;
 public:
  StreamDialog(const QString &caption,QWidget *parent);
  QSize sizeHint() const;
  void makeArgs(QStringList *args,bool escape_args);
  void setServerType(Connector::ServerType type);
  void setControlsLocked(bool state);
  void load(Profile *p);
  void save(FILE *f);

 protected:
  void resizeEvent(QResizeEvent *e);

 private:
  QLabel *gui_stream_label;
  QLabel *gui_stream_name_label;
  QLineEdit *gui_stream_name_edit;
  QLabel *gui_stream_description_label;
  QLineEdit *gui_stream_description_edit;
  QLabel *gui_stream_url_label;
  QLineEdit *gui_stream_url_edit;
  QLabel *gui_stream_genre_label;
  QLineEdit *gui_stream_genre_edit;
  QLabel *gui_stream_icq_label;
  QLineEdit *gui_stream_icq_edit;
  QLabel *gui_stream_aim_label;
  QLineEdit *gui_stream_aim_edit;
  QLabel *gui_stream_irc_label;
  QLineEdit *gui_stream_irc_edit;
  QPushButton *gui_close_button;
  QLabel *gui_stream_timestamp_offset_label;
  SpinBox *gui_stream_timestamp_offset_spin;
  QLabel *gui_stream_timestamp_offset_unit;
  QString gui_caption;
};


#endif  // STREAMDIALOG_H
