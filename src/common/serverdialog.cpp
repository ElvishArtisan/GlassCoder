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

ServerDialog::ServerDialog(QDir *temp_dir,QWidget *parent)
  : QDialog(parent)
{
  srv_temp_dir=temp_dir;

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
  // Verbose Logging
  //
  srv_verbose_check=new QCheckBox(this);
  srv_verbose_label=new QLabel(tr("Enable verbose logging"),this);
  srv_verbose_label->setFont(label_font);
  srv_verbose_label->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);

  //
  // Server Location
  //
  srv_server_location_label=new QLabel(tr("Server URL")+":",this);
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
  // Metadata Port
  //
  srv_server_metadata_port_label=
    new QLabel(tr("Local MetaData Port")+":",this);
  srv_server_metadata_port_label->setFont(label_font);
  srv_server_metadata_port_label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
  srv_server_metadata_port_spin=new QSpinBox(this);
  srv_server_metadata_port_spin->setRange(0,0xFFFF);
  srv_server_metadata_port_spin->setSpecialValueText(tr("Disabled"));

  //
  // Maximum Player Connections
  //
  srv_server_maxconns_label=
    new QLabel(tr("Max Player Connections")+":",this);
  srv_server_maxconns_label->setFont(label_font);
  srv_server_maxconns_label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
  srv_server_maxconns_spin=new QSpinBox(this);
  srv_server_maxconns_spin->setRange(-1,0xFFFF);
  srv_server_maxconns_spin->setSpecialValueText(tr("Unlimited"));

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
  return QSize(600,256);
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
  args->push_back("--server-url="+url.toString());
  args->push_back("--credentials-file="+credentialsFilename());

  if(!srv_server_script_down_edit->text().isEmpty()) {
    args->push_back("--server-script-down="+
		    esc+srv_server_script_down_edit->text()+esc);
  }
  if(!srv_server_script_up_edit->text().isEmpty()) {
    args->push_back("--server-script-up="+
		    esc+srv_server_script_up_edit->text()+esc);
  }
  if(srv_server_metadata_port_spin->value()>0) {
    args->push_back("--metadata-port="+
	     QString().sprintf("%d",srv_server_metadata_port_spin->value()));
  }
  if(srv_server_maxconns_spin->value()>=0) {
    args->push_back("--server-max-connections="+
	     QString().sprintf("%d",srv_server_maxconns_spin->value()));
  }
  if(srv_verbose_check->isChecked()) {
    args->push_back("--verbose");
  }

  return true;
}


bool ServerDialog::writeCredentials() const
{
  FILE *f=NULL;

  if((f=fopen(credentialsFilename().toUtf8(),"w"))==NULL) {
    return false;
  }
  fprintf(f,"[Credentials]\n");
  fprintf(f,"Username=%s\n",
	  srv_server_username_edit->text().toUtf8().constData());
  fprintf(f,"Password=%s\n",
	  srv_server_password_edit->text().toUtf8().constData());
  fclose(f);

  return true;
}


void ServerDialog::setControlsLocked(bool state)
{
  srv_server_type_box->setReadOnly(state);
  srv_server_location_edit->setReadOnly(state);
  srv_server_username_edit->setReadOnly(state);
  srv_server_password_edit->setReadOnly(state);
}


QString ServerDialog::credentialsFilename() const
{
  QUrl url(srv_server_location_edit->text());

  return srv_temp_dir->path()+"/creds-"+url.toString(QUrl::RemoveScheme).
    replace("//","").replace("/","_").replace("?","_");
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
  srv_server_maxconns_spin->
    setValue(p->intValue("GlassGui","ServerMaxConnections",-1));
  srv_verbose_check->setChecked(p->boolValue("GlassGui","VerboseLogging"));
  srv_server_metadata_port_spin->
    setValue(p->intValue("GlassGui","MetadataPort",-1));
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
  fprintf(f,"ServerMaxConnections=%d\n",srv_server_maxconns_spin->value());
  fprintf(f,"MetadataPort=%d\n",srv_server_metadata_port_spin->value());
  fprintf(f,"VerboseLogging=%d\n",srv_verbose_check->isChecked());
}


void ServerDialog::resizeEvent(QResizeEvent *e)
{
  int ypos=10;

  srv_server_type_label->setGeometry(10,ypos,110,24);
  srv_server_type_box->setGeometry(125,ypos,250,24);

  srv_verbose_check->setGeometry(395,ypos,25,25);
  srv_verbose_label->setGeometry(420,ypos,size().width()-430,24);
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
  ypos+=26;

  srv_server_metadata_port_label->setGeometry(10,ypos,180,24);
  srv_server_metadata_port_spin->setGeometry(195,ypos,100,24);
  ypos+=26;

  srv_server_maxconns_label->setGeometry(10,ypos,180,24);
  srv_server_maxconns_spin->setGeometry(195,ypos,100,24);
  ypos+=35;

  srv_close_button->setGeometry(size().width()-80,size().height()-50,70,40);
}


void ServerDialog::serverTypeChanged(int index)
{
  Connector::ServerType type=
    (Connector::ServerType)srv_server_type_box->itemData(index).toInt();
  bool multirate=false;
  bool authfields=false;

  switch(type) {
  case Connector::HlsServer:
    multirate=true;
    authfields=true;
    break;

  case Connector::IcecastStreamerServer:
    multirate=false;
    authfields=false;
    break;

  case Connector::Shoutcast1Server:
  case Connector::Shoutcast2Server:
  case Connector::IcecastOutServer:
  case Connector::Icecast2Server:
  case Connector::FileServer:
  case Connector::FileArchiveServer:
    multirate=false;
    authfields=true;
    break;

  case Connector::LastServer:
    break;
  }
  srv_server_script_up_label->setEnabled(authfields);
  srv_server_script_up_edit->setEnabled(authfields);
  srv_server_script_down_label->setEnabled(authfields);
  srv_server_script_down_edit->setEnabled(authfields);
  srv_server_maxconns_label->setDisabled(authfields);
  srv_server_maxconns_spin->setDisabled(authfields);
  emit typeChanged(type,multirate);
}


void ServerDialog::locationChanged(const QString &str)
{
  emit settingsChanged();
}
