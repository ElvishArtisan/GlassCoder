// deletedialog.cpp
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

#include "deletedialog.h"

DeleteDialog::DeleteDialog(QDir *inst_dir,QWidget *parent)
  : QDialog(parent)
{
  setWindowTitle("GlassCommander - "+tr("Remove Instance"));

  //
  // Fonts
  //
  QFont bold_font(font().family(),font().pointSize(),QFont::Bold);

  dialog_delete_instance=NULL;;
  dialog_dir=inst_dir;

  dialog_name_label=new QLabel(this);
  dialog_name_label->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
  dialog_name_label->setFont(bold_font);

  dialog_delete_label=new QLabel(tr("Delete underlying instance data"),this);
  dialog_delete_label->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);

  dialog_delete_checkbox=new QCheckBox(this);

  dialog_yes_button=new QPushButton(tr("Yes"),this);
  dialog_yes_button->setFont(bold_font);
  connect(dialog_yes_button,SIGNAL(clicked()),this,SLOT(yesData()));

  dialog_no_button=new QPushButton(tr("No"),this);
  dialog_no_button->setFont(bold_font);
  connect(dialog_no_button,SIGNAL(clicked()),this,SLOT(noData()));
}


QSize DeleteDialog::sizeHint() const
{
  return QSize(400,140);
}


int DeleteDialog::exec(const QString &inst_name,bool *delete_instance)
{
  dialog_delete_instance=delete_instance;
  dialog_delete_checkbox->setChecked(*delete_instance);

  dialog_name_label->
    setText(tr("Remove instance")+" \""+inst_name+"\" "+
	    tr("from the list?"));

  return QDialog::exec();
}


void DeleteDialog::yesData()
{
  *dialog_delete_instance=dialog_delete_checkbox->isChecked();

  done(true);
}


void DeleteDialog::noData()
{
  done(false);
}


void DeleteDialog::closeEvent(QCloseEvent *e)
{
  noData();
}


void DeleteDialog::resizeEvent(QResizeEvent *e)
{
  dialog_name_label->setGeometry(10,10,size().width()-20,20);

  dialog_delete_checkbox->setGeometry(80,40,20,20);
  dialog_delete_label->setGeometry(105,40,size().width()-115,20);

  dialog_yes_button->setGeometry(size().width()-180,size().height()-60,80,50);
  dialog_no_button->setGeometry(size().width()-90,size().height()-60,80,50);
}
