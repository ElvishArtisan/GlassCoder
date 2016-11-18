// instancedialog.h
//
// Pick an existing or new GlassGui instance.
//
//   (C) Copyright 2016 Fred Gleason <fredg@paravelsystems.com>
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

#ifndef INSTANCEDIALOG_H
#define INSTANCEDIALOG_H

#include <QDialog>
#include <QDir>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QStringList>

#include "filenamevalidator.h"

class InstanceDialog : public QDialog
{
 Q_OBJECT;
 public:
  InstanceDialog(QDir *inst_dir,QWidget *parent=0);
  QSize sizeHint() const;

 public slots:
  int exec(QString *inst_name,const QStringList &used_names);

 private slots:
  void textChangedData(const QString &str);
  void itemClickedData(QListWidgetItem *item);
  void itemDoubleClickedData(QListWidgetItem *item);
  void okData();
  void cancelData();

 protected:
  void closeEvent(QCloseEvent *e);
  void resizeEvent(QResizeEvent *e);

 private:
  void RefreshList();
  bool IsMemberOf(const QStringList &list,const QString &str) const;
  QLabel *instance_name_label;
  QLineEdit *instance_name_edit;
  QListWidget *instance_list;
  QPushButton *instance_ok_button;
  QPushButton *instance_cancel_button;
  QString *instance_name;
  QStringList instance_used_names;
  QDir *instance_dir;
  FilenameValidator *instance_validator;
};


#endif  // INSTANCEDIALOG_H
