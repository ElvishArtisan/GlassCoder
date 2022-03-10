// serverdialog.h
//
// Configuration dialog for server settings
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

#ifndef SERVERDIALOG_H
#define SERVERDIALOG_H

#include <stdio.h>

#include <QCheckBox>
#include <QDialog>
#include <QDir>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QString>
#include <QStringList>

#include "combobox.h"
#include "connector.h"
#include "profile.h"

class ServerDialog : public QDialog
{
 Q_OBJECT;
 public:
  ServerDialog(QDir *temp_dir,const QString &caption,QWidget *parent);
  QSize sizeHint() const;
  bool makeArgs(QStringList *args,bool escape_args);
  bool writeCredentials() const;
  void setControlsLocked(bool state);
  QString credentialsFilename() const;
  void load(Profile *p);
  void save(FILE *f);

 signals:
  void typeChanged(Connector::ServerType type);
  void settingsChanged();

 protected:
  void resizeEvent(QResizeEvent *e);

 private slots:
  void serverTypeChanged(int index);
  void locationChanged(const QString &str);
  void useIdentityChanged(bool state);
  void selectIdentityFile();

 private:
  QLabel *srv_server_label;
  QLabel *srv_server_type_label;
  ComboBox *srv_server_type_box;
  QCheckBox *srv_verbose_check;
  QLabel *srv_verbose_label;
  QLabel *srv_server_location_label;
  QLineEdit *srv_server_location_edit;
  QLabel *srv_server_username_label;
  QLineEdit *srv_server_username_edit;
  QLabel *srv_server_password_label;
  QLineEdit *srv_server_password_edit;
  QCheckBox *srv_use_identity_check;
  QLabel *srv_use_identity_label;
  QLabel *srv_identity_label;
  QLineEdit *srv_identity_edit;
  QPushButton *srv_identity_button;
  QLabel *srv_server_script_up_label;
  QLineEdit *srv_server_script_up_edit;
  QLabel *srv_server_script_down_label;
  QLineEdit *srv_server_script_down_edit;
  QLabel *srv_server_metadata_port_label;
  QSpinBox *srv_server_metadata_port_spin;
  QLabel *srv_server_maxconns_label;
  QSpinBox *srv_server_maxconns_spin;
  QCheckBox *srv_cleanup_check;
  QLabel *srv_cleanup_label;
  QPushButton *srv_close_button;
  QDir *srv_temp_dir;
  QString srv_identity_path;
  QString srv_caption;
};


#endif  // SERVERDIALOG_H
