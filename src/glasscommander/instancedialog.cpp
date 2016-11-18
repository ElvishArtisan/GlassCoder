// instancedialog.cpp
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

#include <QMessageBox>

#include "guiapplication.h"
#include "instancedialog.h"

InstanceDialog::InstanceDialog(QDir *inst_dir,QWidget *parent)
  : QDialog(parent)
{
  instance_dir=inst_dir;

  setWindowTitle("GlassCommander - "+tr("Pick Instance"));

  //
  // Fonts
  //
  QFont bold_font(font().family(),font().pointSize(),QFont::Bold);

  //
  // Instance Name
  //
  instance_name_label=new QLabel(tr("Instance")+":",this);
  instance_name_label->setFont(bold_font);
  instance_name_label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
  instance_name_edit=new QLineEdit(this);
  instance_validator=new FilenameValidator(this);
  instance_name_edit->setValidator(instance_validator);
  connect(instance_name_edit,SIGNAL(textChanged(const QString &)),
	  this,SLOT(textChangedData(const QString &)));

  //
  // Instance List
  //
  instance_list=new QListWidget(this);
  connect(instance_list,SIGNAL(itemClicked(QListWidgetItem *)),
	  this,SLOT(itemClickedData(QListWidgetItem *)));
  connect(instance_list,SIGNAL(itemDoubleClicked(QListWidgetItem *)),
	  this,SLOT(itemDoubleClickedData(QListWidgetItem *)));

  //
  // OK Button
  //
  instance_ok_button=new QPushButton(tr("OK"),this);
  instance_ok_button->setFont(bold_font);
  connect(instance_ok_button,SIGNAL(clicked()),this,SLOT(okData()));

  //
  // Cancel Button
  //
  instance_cancel_button=new QPushButton(tr("Cancel"),this);
  instance_cancel_button->setFont(bold_font);
  connect(instance_cancel_button,SIGNAL(clicked()),this,SLOT(cancelData()));
}


QSize InstanceDialog::sizeHint() const
{
  return QSize(350,400);
}


int InstanceDialog::exec(QString *inst_name,const QStringList &used_names)
{
  instance_name=inst_name;
  instance_used_names=used_names;
  instance_name_edit->setText(*inst_name);
  textChangedData(*inst_name);
  RefreshList();
  return QDialog::exec();
}


void InstanceDialog::textChangedData(const QString &str)
{
  instance_ok_button->setDisabled(str.isEmpty());
  QList<QListWidgetItem *>items=instance_list->findItems(str,Qt::MatchExactly);
  if(items.size()>0) {
    instance_list->setCurrentItem(items.at(0));
  }
  else {
    instance_list->clearSelection();
  }
}


void InstanceDialog::itemClickedData(QListWidgetItem *item)
{
  instance_name_edit->setText(item->data(Qt::DisplayRole).toString());
  instance_ok_button->setEnabled(true);
}


void InstanceDialog::itemDoubleClickedData(QListWidgetItem *item)
{
  instance_name_edit->setText(item->data(Qt::DisplayRole).toString());
  okData();
}


void InstanceDialog::okData()
{
  if(IsMemberOf(instance_used_names,instance_name_edit->text())) {
    QMessageBox::information(this,"GlassCommander - "+tr("Error"),
			     tr("That instance is already loaded."));
    return;
  }
  *instance_name=instance_name_edit->text();
  done(true);
}


void InstanceDialog::cancelData()
{
  done(false);
}


void InstanceDialog::closeEvent(QCloseEvent *e)
{
  cancelData();
}


void InstanceDialog::resizeEvent(QResizeEvent *e)
{
  instance_name_label->setGeometry(10,10,80,20);
  instance_name_edit->setGeometry(95,10,size().width()-105,20);
  instance_list->setGeometry(10,32,size().width()-20,size().height()-102);

  instance_ok_button->
    setGeometry(size().width()-180,size().height()-60,80,50);
  instance_cancel_button->
    setGeometry(size().width()-90,size().height()-60,80,50);
}


void InstanceDialog::RefreshList()
{
  QStringList filters;
  filters.push_back(QString(GUI_SETTINGS_FILE)+"-*");
  QStringList instances=
    instance_dir->entryList(filters,QDir::Files,QDir::Name|QDir::IgnoreCase);
  instance_list->clear();
  for(int i=0;i<instances.size();i++) {
    QString name=instances.at(i).right(instances.at(i).length()-11);
    if(!IsMemberOf(instance_used_names,name)) {
      instance_list->addItem(name);
    }
  }
}


bool InstanceDialog::IsMemberOf(const QStringList &list,const QString &str)
  const
{
  for(int i=0;i<list.size();i++) {
    if(list.at(i)==str) {
      return true;
    }
  }
  return false;
}
