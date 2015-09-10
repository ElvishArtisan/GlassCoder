// serverdialog.cpp
//
// Configuration dialog for server settings
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

#include <QUrl>

#include "serverdialog.h"

ServerDialog::ServerDialog(QWidget *parent)
  : QDialog(parent)
{
  //
  // Fonts
  //
  QFont label_font("helvetica",14,QFont::Bold);
  label_font.setPixelSize(14);

  setWindowTitle("GlassGui - "+tr("Server Settings"));

  //
  // Server Type
  //
  srv_server_type_label=new QLabel(tr("Type")+":",this);
  srv_server_type_label->setFont(label_font);
  srv_server_type_label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
  srv_server_type_box=new ComboBox(this);
  for(int i=0;i<Connector::LastServer;i++) {
    srv_server_type_box->
      insertItem(i,Connector::serverTypeText((Connector::ServerType)i),i);
  }
  connect(srv_server_type_box,SIGNAL(activated(int)),
	  this,SLOT(serverTypeChanged(int)));

  //
  // Server Location
  //
  srv_server_location_label=new QLabel(tr("Publish Point")+":",this);
  srv_server_location_label->setFont(label_font);
  srv_server_location_label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
  srv_server_location_edit=new QLineEdit(this);
  connect(srv_server_location_edit,SIGNAL(textEdited(const QString &)),
	  this,SLOT(locationChanged(const QString &)));

  //
  // Server Username
  //
  srv_server_username_label=new QLabel(tr("User Name")+":",this);
  srv_server_username_label->setFont(label_font);
  srv_server_username_label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
  srv_server_username_edit=new QLineEdit(this);

  //
  // Server Password
  //
  srv_server_password_label=new QLabel(tr("Password")+":",this);
  srv_server_password_label->setFont(label_font);
  srv_server_password_label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
  srv_server_password_edit=new QLineEdit(this);
  srv_server_password_edit->setEchoMode(QLineEdit::Password);

  //
  // Server Script Up
  //
  srv_server_script_up_label=new QLabel(tr("CONNECTED Script")+":",this);
  srv_server_script_up_label->setFont(label_font);
  srv_server_script_up_label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
  srv_server_script_up_edit=new QLineEdit(this);

  //
  // Server Script Down
  //
  srv_server_script_down_label=new QLabel(tr("DISCONNECTED Script")+":",this);
  srv_server_script_down_label->setFont(label_font);
  srv_server_script_down_label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
  srv_server_script_down_edit=new QLineEdit(this);

  //
  // Close Button
  //
  srv_close_button=new QPushButton(tr("Close"),this);
  srv_close_button->setFont(label_font);
  connect(srv_close_button,SIGNAL(clicked()),this,SLOT(hide()));

  setMinimumHeight(sizeHint().height());
  setMaximumHeight(sizeHint().height());
  setMinimumSize(sizeHint());
}


QSize ServerDialog::sizeHint() const
{
  return QSize(600,230);
}


bool ServerDialog::makeArgs(QStringList *args,bool escape_args)
{
  QUrl url(srv_server_location_edit->text());
  if(!url.isValid()) {
    return false;
  }
  QString esc="";
  if(escape_args) {
    esc="\"";
  }
  Connector::ServerType type=(Connector::ServerType)
    srv_server_type_box->itemData(srv_server_type_box->currentIndex()).toInt();

  args->push_back("--server-type="+Connector::optionKeyword(type));
  args->push_back("--server-hostname="+url.host());
  if(url.port()>0) {
    args->push_back("--server-port="+QString().sprintf("%d",url.port()));
  }
  args->push_back("--server-mountpoint="+url.path());
  if(!srv_server_username_edit->text().isEmpty()) {
    args->push_back("--server-username="+srv_server_username_edit->text());
  }
  if(!srv_server_password_edit->text().isEmpty()) {
    args->push_back("--server-password="+srv_server_password_edit->text());
  }
  if(!srv_server_script_down_edit->text().isEmpty()) {
    args->push_back("--server-script-down="+
		    esc+srv_server_script_down_edit->text()+esc);
  }
  if(!srv_server_script_up_edit->text().isEmpty()) {
    args->push_back("--server-script-up="+
		    esc+srv_server_script_up_edit->text()+esc);
  }

  return true;
}


void ServerDialog::setControlsLocked(bool state)
{
  srv_server_type_box->setReadOnly(state);
  srv_server_location_edit->setReadOnly(state);
  srv_server_username_edit->setReadOnly(state);
  srv_server_password_edit->setReadOnly(state);
}


void ServerDialog::load(Profile *p)
{
  srv_server_type_box->
    setCurrentItemData(Connector::serverType(p->stringValue("GlassGui",
							    "ServerType")));
  serverTypeChanged(srv_server_type_box->currentIndex());
  srv_server_location_edit->
    setText(p->stringValue("GlassGui","ServerLocation"));
  srv_server_username_edit->
    setText(p->stringValue("GlassGui","ServerUsername"));
  srv_server_password_edit->
    setText(p->stringValue("GlassGui","ServerPassword"));
  srv_server_script_down_edit->
    setText(p->stringValue("GlassGui","ServerScriptDown"));
  srv_server_script_up_edit->
    setText(p->stringValue("GlassGui","ServerScriptUp"));
}


void ServerDialog::save(FILE *f)
{
  fprintf(f,"ServerType=%s\n",
	  (const char *)Connector::optionKeyword((Connector::ServerType)
	     srv_server_type_box->currentItemData().toInt()).toUtf8()); 
  fprintf(f,"ServerLocation=%s\n",
	  (const char *)srv_server_location_edit->text().toUtf8());
  fprintf(f,"ServerUsername=%s\n",
	  (const char *)srv_server_username_edit->text().toUtf8());
  fprintf(f,"ServerPassword=%s\n",
	  (const char *)srv_server_password_edit->text().toUtf8());
  fprintf(f,"ServerScriptDown=%s\n",
	  (const char *)srv_server_script_down_edit->text().toUtf8());
  fprintf(f,"ServerScriptUp=%s\n",
	  (const char *)srv_server_script_up_edit->text().toUtf8());
}


void ServerDialog::resizeEvent(QResizeEvent *e)
{
  int ypos=10;

  srv_server_type_label->setGeometry(10,ypos,110,24);
  srv_server_type_box->setGeometry(125,ypos,250,24);
  ypos+=26;

  srv_server_location_label->setGeometry(10,ypos,180,24);
  srv_server_location_edit->setGeometry(195,ypos,size().width()-205,24);
  ypos+=26;

  srv_server_username_label->setGeometry(10,ypos,180,24);
  srv_server_username_edit->setGeometry(195,ypos,size().width()-205,24);
  ypos+=26;

  srv_server_password_label->setGeometry(10,ypos,180,24);
  srv_server_password_edit->setGeometry(195,ypos,size().width()-205,24);
  ypos+=26;

  srv_server_script_up_label->setGeometry(10,ypos,180,24);
  srv_server_script_up_edit->setGeometry(195,ypos,size().width()-205,24);
  ypos+=26;

  srv_server_script_down_label->setGeometry(10,ypos,180,24);
  srv_server_script_down_edit->setGeometry(195,ypos,size().width()-205,24);
  ypos+=35;

  srv_close_button->setGeometry(size().width()-80,size().height()-50,70,40);
}


void ServerDialog::serverTypeChanged(int index)
{
  Connector::ServerType type=
    (Connector::ServerType)srv_server_type_box->itemData(index).toInt();
  bool multirate=false;

  switch(type) {
  case Connector::HlsServer:
    multirate=true;
    break;

  case Connector::Shoutcast1Server:
    multirate=false;
    break;

  case Connector::Shoutcast2Server:
    multirate=false;
    break;

  case Connector::Icecast2Server:
    multirate=false;
    break;

  case Connector::LastServer:
    break;
  }
  emit typeChanged(type,multirate);
}


void ServerDialog::locationChanged(const QString &str)
{
  emit settingsChanged();
}
