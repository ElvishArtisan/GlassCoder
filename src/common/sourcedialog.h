// sourcedialog.h
//
// Configuration dialog for source settings
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

#ifndef SOURCEDIALOG_H
#define SOURCEDIALOG_H

#include <stdio.h>

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QString>
#include <QStringList>

#include "audiodevice.h"
#include "combobox.h"
#include "hpiwidget.h"
#include "profile.h"

class SourceDialog : public QDialog
{
 Q_OBJECT;
 public:
 SourceDialog(const QString &caption,QWidget *parent);
  QSize sizeHint() const;
  bool makeArgs(QStringList *args,bool escape_args);
  void setControlsLocked(bool state);
  void addSourceTypes(const QString &types);
  void load(Profile *p);
  void save(FILE *f);

 public slots:
  void show();

 signals:
  void updated();

 protected:
  void resizeEvent(QResizeEvent *e);

 private slots:
  void sourceTypeChanged(int n);
  void fileSelectName();
  void checkArgs(const QString &str);

 private:
  void ChangeSize();

  QLabel *gui_source_type_label;
  ComboBox *gui_source_type_box;

  QLabel *gui_alsa_device_label;
  QLineEdit *gui_alsa_device_edit;

  QLabel *gui_file_name_label;
  QLineEdit *gui_file_name_edit;
  QPushButton *gui_file_select_button;

  HpiWidget *gui_asihpi_widget;

  QLabel *gui_jack_server_name_label;
  QLineEdit *gui_jack_server_name_edit;
  QLabel *gui_jack_client_name_label;
  QLineEdit *gui_jack_client_name_edit;
  QLabel *gui_jack_gain_label;
  QSpinBox *gui_jack_gain_spin;

  QPushButton *gui_close_button;

  QString gui_caption;
};


#endif  // SOURCEDIALOG_H
