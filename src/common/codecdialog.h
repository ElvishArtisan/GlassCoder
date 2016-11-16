// codecdialog.h
//
// Configuration dialog for codec settings
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

#ifndef CODECDIALOG_H
#define CODECDIALOG_H

#include <stdio.h>

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QString>
#include <QStringList>
#include <QTextEdit>

#include "codec.h"
#include "combobox.h"
#include "profile.h"

#define CODECDIALOG_MAX_SUBSTREAMS 3

class CodecDialog : public QDialog
{
 Q_OBJECT;
 public:
  CodecDialog(QWidget *parent=0);
  QSize sizeHint() const;
  void setControlsLocked(bool state);
  void setMultirate(bool state);
  void makeArgs(QStringList *args);
  void addCodecTypes(const QString &types);
  void load(Profile *p);
  void save(FILE *f);

 protected:
  void resizeEvent(QResizeEvent *e);

 private slots:
  void codecTypeChanged(int n);
  void codecSamplerateChanged(int n);

 private:
  QLabel *gui_codec_type_label;
  ComboBox *gui_codec_type_box;
  QLabel *gui_codec_samplerate_label;
  ComboBox *gui_codec_samplerate_box;
  QLabel *gui_codec_channels_label;
  ComboBox *gui_codec_channels_box;
  QLabel *gui_codec_bitrate_label;
  ComboBox *gui_codec_bitrate_box[CODECDIALOG_MAX_SUBSTREAMS];
  QPushButton *gui_close_button;
};


#endif  // CODECDIALOG_H
