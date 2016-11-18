// deletedialog.h
//
// Confirm deletion of a GlassCoder instance.
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

#ifndef DELETEDIALOG_H
#define DELETEDIALOG_H

#include <QCheckBox>
#include <QDialog>
#include <QDir>
#include <QLabel>
#include <QPushButton>

class DeleteDialog : public QDialog
{
 Q_OBJECT;
 public:
  DeleteDialog(QDir *inst_dir,QWidget *parent=0);
  QSize sizeHint() const;

 public slots:
  int exec(const QString &inst_name,bool *delete_instance);

 private slots:
  void yesData();
  void noData();

 protected:
  void closeEvent(QCloseEvent *e);
  void resizeEvent(QResizeEvent *e);

 private:
  bool *dialog_delete_instance;
  QLabel *dialog_name_label;
  QLabel *dialog_delete_label;
  QCheckBox *dialog_delete_checkbox;
  QPushButton *dialog_yes_button;
  QPushButton *dialog_no_button;
  QDir *dialog_dir;
};


#endif  // DELETEDIALOG_H
