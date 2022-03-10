// serverdialog.cpp
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

#include <QFileDialog>
#include <QUrl>

#include "serverdialog.h"

ServerDialog::ServerDialog(QDir *temp_dir,const QString &caption,QWidget *parent)
  : QDialog(parent)
{
  srv_temp_dir=temp_dir;
  srv_caption=caption;

  //
  // Fonts
  //
  QFont label_font("helvetica",14,QFont::Bold);
  label_font.setPixelSize(14);

  setWindowTitle(srv_caption+" - "+tr("Server Settings"));

  srv_identity_path="/";
  if(getenv("HOME")!=NULL) {
    srv_identity_path=QString(getenv("HOME"))+"/.ssh";
  }

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
  // SSH Identity
  //
  srv_use_identity_check=new QCheckBox(this);
  srv_use_identity_label=
    new QLabel(tr("Use SSH Identity To Authenticate"),this);
  srv_use_identity_label->setFont(label_font);
  srv_use_identity_label->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
  connect(srv_use_identity_check,SIGNAL(toggled(bool)),
	  this,SLOT(useIdentityChanged(bool)));
  srv_identity_label=new QLabel(tr("SSH Identity File")+":",this);
  srv_identity_label->setFont(label_font);
  srv_identity_label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
  srv_identity_edit=new QLineEdit(this);
  srv_identity_button=new QPushButton(tr("Select"),this);
  connect(srv_identity_button,SIGNAL(clicked()),
	  this,SLOT(selectIdentityFile()));

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
  // Publish Point Cleanup
  //
  srv_cleanup_check=new QCheckBox(this);
  srv_cleanup_label=
    new QLabel(tr("Purge stale files on publishing point"),this);
  srv_cleanup_label->setFont(label_font);
  srv_cleanup_label->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);

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
  return QSize(600,330);
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
  if(srv_use_identity_check->isChecked()&&(url.scheme().toLower()=="sftp")) {
    args->push_back("--ssh-identity="+srv_identity_edit->text());
  }

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
  if(!srv_cleanup_check->isChecked()) {
    args->push_back("--server-no-deletes");
  }
  if(srv_verbose_check->isChecked()) {
    args->push_back("--verbose");
  }

  return true;
}


bool ServerDialog::writeCredentials() const
{
  QUrl url(srv_server_location_edit->text());
  FILE *f=NULL;

  if((f=fopen(credentialsFilename().toUtf8(),"w"))==NULL) {
    return false;
  }
  fprintf(f,"[Credentials]\n");
  fprintf(f,"Username=%s\n",
	  srv_server_username_edit->text().toUtf8().constData());
  fprintf(f,"Password=%s\n",
	  srv_server_password_edit->text().toUtf8().constData());
  if(srv_use_identity_check->isChecked()&&(url.scheme().toLower()=="sftp")) {
    //args->push_back("--ssh-identity="+srv_identity_edit->text());
  }
  fclose(f);

  return true;
}


void ServerDialog::setControlsLocked(bool state)
{
  srv_server_type_box->setReadOnly(state);
  srv_server_location_edit->setReadOnly(state);
  srv_server_username_edit->setReadOnly(state);
  srv_server_password_edit->setReadOnly(state);
  srv_identity_edit->setReadOnly(state);
}


QString ServerDialog::credentialsFilename() const
{
  QUrl url(srv_server_location_edit->text());

  return srv_temp_dir->path()+"/creds-"+url.toString(QUrl::RemoveScheme).
    replace("//","").replace("/","_").replace("?","_");
}


void ServerDialog::load(Profile *p)
{
  bool ok=false;

  srv_server_type_box->
    setCurrentItemData(Connector::serverType(p->stringValue("GlassGui",
							    "ServerType")));
  serverTypeChanged(srv_server_type_box->currentIndex());
  srv_server_location_edit->
    setText(p->stringValue("GlassGui","ServerLocation"));
  srv_server_username_edit->
    setText(QByteArray::fromBase64(p->stringValue("GlassGui",
            "ServerUsernameSecret","",&ok).toUtf8()));
  if(!ok) {
    srv_server_username_edit->
      setText(p->stringValue("GlassGui","ServerUsername"));
  }
  srv_server_password_edit->
    setText(QByteArray::fromBase64(p->stringValue("GlassGui",
            "ServerPasswordSecret","",&ok).toUtf8()));
  if(!ok) {
    srv_server_password_edit->
      setText(p->stringValue("GlassGui","ServerPassword"));
  }
  srv_use_identity_check->
    setChecked(p->intValue("GlassGui","ServerUseIdentity")!=0);
  srv_identity_edit->
    setText(p->stringValue("GlassGui","ServerSshIdentity"));
  srv_server_script_down_edit->
    setText(p->stringValue("GlassGui","ServerScriptDown"));
  srv_server_script_up_edit->
    setText(p->stringValue("GlassGui","ServerScriptUp"));
  srv_server_maxconns_spin->
    setValue(p->intValue("GlassGui","ServerMaxConnections",-1));
  srv_cleanup_check->
    setChecked(p->intValue("GlassGui","ServerNoDeletes",1)!=0);
  srv_verbose_check->setChecked(p->boolValue("GlassGui","VerboseLogging"));
  srv_server_metadata_port_spin->
    setValue(p->intValue("GlassGui","MetadataPort",-1));

  useIdentityChanged(srv_use_identity_check->isChecked());
}


void ServerDialog::save(FILE *f)
{
  fprintf(f,"ServerType=%s\n",
	 Connector::optionKeyword((Connector::ServerType)
	 srv_server_type_box->currentItemData().toInt()).toUtf8().constData()); 
  fprintf(f,"ServerLocation=%s\n",
	  srv_server_location_edit->text().toUtf8().constData());
  fprintf(f,"ServerUsernameSecret=%s\n",
	  srv_server_username_edit->text().toUtf8().toBase64().constData());
  fprintf(f,"ServerPasswordSecret=%s\n",
	  srv_server_password_edit->text().toUtf8().toBase64().constData());
  fprintf(f,"ServerUseIdentity=%u\n",srv_use_identity_check->isChecked());
  fprintf(f,"ServerSshIdentity=%s\n",
	  srv_identity_edit->text().toUtf8().constData());
  fprintf(f,"ServerScriptDown=%s\n",
	  srv_server_script_down_edit->text().toUtf8().constData());
  fprintf(f,"ServerScriptUp=%s\n",
	  srv_server_script_up_edit->text().toUtf8().constData());
  fprintf(f,"ServerMaxConnections=%d\n",srv_server_maxconns_spin->value());
  fprintf(f,"ServerNoDeletes=%d\n",srv_cleanup_check->isChecked());
  fprintf(f,"MetadataPort=%d\n",srv_server_metadata_port_spin->value());
  fprintf(f,"VerboseLogging=%d\n",srv_verbose_check->isChecked());
}


void ServerDialog::resizeEvent(QResizeEvent *e)
{
  int ypos=10;

  srv_server_type_label->setGeometry(10,ypos,110,24);
  srv_server_type_box->setGeometry(125,ypos,250,24);

  srv_verbose_check->setGeometry(395,ypos,25,25);
  srv_verbose_label->setGeometry(420,ypos+1,size().width()-430,24);
  ypos+=26;

  srv_server_location_label->setGeometry(10,ypos,180,24);
  srv_server_location_edit->setGeometry(195,ypos,size().width()-205,24);
  ypos+=26;

  srv_server_username_label->setGeometry(10,ypos,180,24);
  srv_server_username_edit->setGeometry(195,ypos,size().width()-205,24);
  ypos+=26;

  srv_server_password_label->setGeometry(10,ypos,180,24);
  srv_server_password_edit->setGeometry(195,ypos,size().width()-205,24);
  ypos+=22;

  srv_use_identity_check->setGeometry(195,ypos+5,15,15);
  srv_use_identity_label->setGeometry(215,ypos+1,size().width()-225,24);
  ypos+=26;

  srv_identity_label->setGeometry(10,ypos,180,24);
  srv_identity_edit->setGeometry(195,ypos,size().width()-275,24);
  srv_identity_button->setGeometry(size().width()-70,ypos-2,60,28);
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
  ypos+=26;

  srv_cleanup_check->setGeometry(105,ypos+5,15,15);
  srv_cleanup_label->setGeometry(125,ypos+1,size().width()-225,24);
  ypos+=35;

  srv_close_button->setGeometry(size().width()-80,size().height()-50,70,40);
}


void ServerDialog::serverTypeChanged(int index)
{
  Connector::ServerType type=
    (Connector::ServerType)srv_server_type_box->itemData(index).toInt();
  bool authfields=false;
  bool cleanup=false;

  switch(type) {
  case Connector::HlsServer:
    authfields=true;
    cleanup=true;
    break;

  case Connector::IcecastStreamerServer:
    authfields=false;
    cleanup=false;
    break;

  case Connector::Shoutcast1Server:
  case Connector::Shoutcast2Server:
  case Connector::IcecastOutServer:
  case Connector::Icecast2Server:
  case Connector::FileServer:
  case Connector::FileArchiveServer:
    authfields=true;
    cleanup=false;
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
  srv_cleanup_check->setEnabled(cleanup);
  srv_cleanup_label->setEnabled(cleanup);
  emit typeChanged(type);
}


void ServerDialog::locationChanged(const QString &str)
{
  useIdentityChanged(srv_use_identity_check->isChecked());
  emit settingsChanged();
}


void ServerDialog::useIdentityChanged(bool state)
{
  bool is_sftp=
    QUrl(srv_server_location_edit->text()).scheme().toLower()=="sftp";

  if(state&&is_sftp) {
    srv_server_password_label->setText(tr("Identity Passphrase")+":");
  }
  else {
    srv_server_password_label->setText(tr("Password")+":");
  }
  srv_use_identity_check->setEnabled(is_sftp);
  srv_use_identity_label->setEnabled(is_sftp);

  srv_identity_label->setEnabled(state&&is_sftp);
  srv_identity_edit->setEnabled(state&&is_sftp);
  srv_identity_button->setEnabled(state&&is_sftp);
}


void ServerDialog::selectIdentityFile()
{
  QString filename=srv_identity_edit->text();
  if(filename.isEmpty()) {
    filename=srv_identity_path;
  }
  filename=QFileDialog::getOpenFileName(this,"GlassGui - "+
					tr("Choose SSH Identity"),
					filename);
  if(!filename.isNull()) {
    srv_identity_edit->setText(filename);
  }
}
