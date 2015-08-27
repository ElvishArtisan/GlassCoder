// glassgui.cpp
//
// glassgui(1) Audio Encoder front end
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

#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QPixmap>
#include <QUrl>

#include "audiodevice.h"
#include "cmdswitch.h"
#include "codec.h"
#include "connector.h"
#include "logging.h"
#include "profile.h"

#include "glassgui.h"

#include "../../icons/glasscoder-16x16.xpm"

MainWidget::MainWidget(QWidget *parent)
  : QMainWindow(parent)
{
  CmdSwitch *cmd=
    new CmdSwitch(qApp->argc(),qApp->argv(),"glassgui",GLASSGUI_USAGE);
  for(unsigned i=0;i<cmd->keys();i++) {
  }

  setWindowIcon(QPixmap(glasscoder_16x16_xpm));
  setWindowTitle(QString("GlassGui v")+VERSION);
  gui_settings_dir=NULL;

  //
  // Fonts
  //
  QFont section_font("helvetica",16,QFont::Bold);
  section_font.setPixelSize(16);
  QFont label_font("helvetica",14,QFont::Bold);
  label_font.setPixelSize(14);
  QFont message_font("helvetica",14,QFont::Normal);
  message_font.setPixelSize(14);

  //
  // Set Size
  //
  setMinimumSize(sizeHint());
  setMaximumHeight(sizeHint().height());

  //
  // Dialogs
  //
  gui_codec_dialog=new CodecDialog(this);
  gui_codeviewer_dialog=new CodeViewer(this);
  gui_stream_dialog=new StreamDialog(this);

  //
  // Status Bar
  //
  gui_message_label=new QLabel(this);
  gui_message_label->setFont(message_font);
  gui_message_label->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
  gui_message_label->setFrameStyle(QFrame::Box|QFrame::Raised);
  gui_message_timer=new QTimer(this);
  gui_message_timer->setSingleShot(true);
  connect(gui_message_timer,SIGNAL(timeout()),this,SLOT(messageTimeoutData()));

  gui_status_frame_widget=new QLabel(this);
  gui_status_frame_widget->setFrameStyle(QFrame::Box|QFrame::Raised);
  gui_status_widget=new StatusWidget(this);
  gui_status_widget->setFont(label_font);

  //
  // Meter Section
  //
  gui_meter=new StereoMeter(this);
  gui_meter->setReference(800);
  gui_meter->setMode(SegMeter::Peak);
  gui_start_button=new QPushButton(tr("Start"),this);
  gui_start_button->setFont(section_font);
  connect(gui_start_button,SIGNAL(clicked()),this,SLOT(startEncodingData()));
  gui_start_button->setDisabled(true);
  gui_code_button=new QPushButton(tr("Show")+"\n"+tr("Code"),this);
  gui_code_button->setFont(section_font);
  connect(gui_code_button,SIGNAL(clicked()),this,SLOT(showCodeData()));
  gui_code_button->setDisabled(true);

  //
  // Server Section
  //
  gui_server_label=new QLabel(tr("Server Settings"),this);
  gui_server_label->setFont(section_font);

  //
  // Server Type
  //
  gui_server_type_label=new QLabel(tr("Type")+":",this);
  gui_server_type_label->setFont(label_font);
  gui_server_type_label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
  gui_server_type_box=new ComboBox(this);
  for(int i=0;i<Connector::LastServer;i++) {
    gui_server_type_box->
      insertItem(i,Connector::serverTypeText((Connector::ServerType)i),i);
  }
  connect(gui_server_type_box,SIGNAL(activated(int)),
	  this,SLOT(serverTypeChanged(int)));

  //
  // Server Location
  //
  gui_server_location_label=new QLabel(tr("Publish Point")+":",this);
  gui_server_location_label->setFont(label_font);
  gui_server_location_label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
  gui_server_location_edit=new QLineEdit(this);
  connect(gui_server_location_edit,SIGNAL(textEdited(const QString &)),
	  this,SLOT(checkArgs(const QString &)));

  //
  // Server Username
  //
  gui_server_username_label=new QLabel(tr("User Name")+":",this);
  gui_server_username_label->setFont(label_font);
  gui_server_username_label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
  gui_server_username_edit=new QLineEdit(this);

  //
  // Server Password
  //
  gui_server_password_label=new QLabel(tr("Password")+":",this);
  gui_server_password_label->setFont(label_font);
  gui_server_password_label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
  gui_server_password_edit=new QLineEdit(this);
  gui_server_password_edit->setEchoMode(QLineEdit::Password);

  //
  // Codec Settings
  //
  gui_codec_button=new QPushButton(tr("Codec Settings"),this);
  gui_codec_button->setFont(section_font);
  connect(gui_codec_button,SIGNAL(clicked()),this,SLOT(codecData()));

  //
  // Stream Settings
  //
  gui_stream_button=new QPushButton(tr("Stream Settings"),this);
  gui_stream_button->setFont(section_font);
  connect(gui_stream_button,SIGNAL(clicked()),this,SLOT(streamData()));

  //
  // Source Section
  //
  gui_source_label=new QLabel(tr("Audio Source"),this);
  gui_source_label->setFont(section_font);

  //
  // Source Type
  //
  gui_source_type_label=new QLabel(tr("Type")+":",this);
  gui_source_type_label->setFont(label_font);
  gui_source_type_label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
  gui_source_type_box=new ComboBox(this);
  connect(gui_source_type_box,SIGNAL(activated(int)),
	  this,SLOT(sourceTypeChanged(int)));

  //
  // ALSA Fields
  //
  gui_alsa_device_label=new QLabel(tr("ALSA Device")+":",this);
  gui_alsa_device_label->setFont(label_font);
  gui_alsa_device_label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
  gui_alsa_device_label->hide();
  gui_alsa_device_edit=new QLineEdit(this);
  connect(gui_alsa_device_edit,SIGNAL(textEdited(const QString &)),
	  this,SLOT(checkArgs(const QString &)));
  gui_alsa_device_edit->hide();

  //
  // FILE Fields
  //
  gui_file_name_label=new QLabel(tr("Filename")+":",this);
  gui_file_name_label->setFont(label_font);
  gui_file_name_label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
  gui_file_name_label->hide();
  gui_file_name_edit=new QLineEdit(this);
  connect(gui_file_name_edit,SIGNAL(textEdited(const QString &)),
	  this,SLOT(checkArgs(const QString &)));
  gui_file_name_edit->hide();
  gui_file_select_button=new QPushButton(tr("Select"),this);
  connect(gui_file_select_button,SIGNAL(clicked()),this,SLOT(fileSelectName()));
  gui_file_select_button->hide();

  //
  // HPI Fields
  //
  gui_asihpi_view=new HpiInputListView(this);
  gui_asihpi_view->hide();

  //
  // JACK Fields
  //
  gui_jack_server_name_label=new QLabel(tr("JACK Server Name")+":",this);
  gui_jack_server_name_label->setFont(label_font);
  gui_jack_server_name_label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
  gui_jack_server_name_label->hide();
  gui_jack_server_name_edit=new QLineEdit(this);
  gui_jack_server_name_edit->hide();

  gui_jack_client_name_label=new QLabel(tr("JACK Client Name")+":",this);
  gui_jack_client_name_label->setFont(label_font);
  gui_jack_client_name_label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
  gui_jack_client_name_label->hide();
  gui_jack_client_name_edit=new QLineEdit(this);
  gui_jack_client_name_edit->hide();

  //
  // Process Timers
  //
  gui_process_cleanup_timer=new QTimer(this);
  gui_process_cleanup_timer->setSingleShot(true);
  connect(gui_process_cleanup_timer,SIGNAL(timeout()),
	  this,SLOT(processCollectGarbageData()));

  gui_process_kill_timer=new QTimer(this);
  gui_process_kill_timer->setSingleShot(true);
  connect(gui_process_kill_timer,SIGNAL(timeout()),
	  this,SLOT(processKillData()));

  //
  // Get Codec List
  //
  gui_process=new QProcess(this);
  connect(gui_process,SIGNAL(error(QProcess::ProcessError)),
	  this,SLOT(processErrorData(QProcess::ProcessError)));
  connect(gui_process,SIGNAL(finished(int,QProcess::ExitStatus)),
	  this,SLOT(codecFinishedData(int,QProcess::ExitStatus)));
  
  QStringList args;
  args.push_back("--list-codecs");
  gui_process->start("glasscoder",args);
}


QSize MainWidget::sizeHint() const
{
  return QSize(560,500);
}


void MainWidget::closeEvent(QCloseEvent *e)
{
  if(gui_process!=NULL) {
    if(QMessageBox::question(this,"GlassGui - "+tr("Exiting"),
			     tr("The encoder is currently running.")+" "+
			     tr("Close will shut it down.")+"\n\n"+
			     tr("Proceed?"),QMessageBox::Yes,QMessageBox::No)==
       QMessageBox::No) {
      e->ignore();
      return;
    }
    gui_process->terminate();
    gui_process_kill_timer->start(GLASSGUI_TERMINATE_TIMEOUT);
    SaveSettings();
    e->ignore();
    return;
  }
  if(!SaveSettings()) {
    QMessageBox::warning(this,"GlassGui - "+tr("Save Error"),
			 tr("Unable to save configuration!"));
  }
  e->accept();
}


void MainWidget::resizeEvent(QResizeEvent *e)
{
  int ypos=0;

  gui_meter->setGeometry(10,10,gui_meter->sizeHint().width(),
			 gui_meter->sizeHint().height());
  gui_start_button->setGeometry(gui_meter->sizeHint().width()+20,15,
				2*(size().width()-gui_meter->sizeHint().width()-30)/3-20,gui_meter->sizeHint().height()-10);
  gui_code_button->setGeometry(gui_meter->sizeHint().width()+10+2*(size().width()-gui_meter->sizeHint().width()-30)/3,15,
			       (size().width()-gui_meter->sizeHint().width()-30)/3+10,gui_meter->sizeHint().height()-10);
  ypos+=(gui_meter->sizeHint().height()+15);

  gui_server_label->setGeometry(10,ypos,size().width()-20,24);
  ypos+=23;

  gui_server_type_label->setGeometry(10,ypos,110,24);
  gui_server_type_box->setGeometry(125,ypos,250,24);
  ypos+=26;

  gui_server_location_label->setGeometry(10,ypos,145,24);
  gui_server_location_edit->setGeometry(160,ypos,size().width()-170,24);
  ypos+=26;

  gui_server_username_label->setGeometry(10,ypos,145,24);
  gui_server_username_edit->setGeometry(160,ypos,size().width()-170,24);
  ypos+=26;

  gui_server_password_label->setGeometry(10,ypos,145,24);
  gui_server_password_edit->setGeometry(160,ypos,size().width()-170,24);
  ypos+=35;

  gui_codec_button->setGeometry(10,ypos,(size().width()-40)/3,35);
  gui_stream_button->
    setGeometry(size().width()/3,ypos,(size().width()-40)/3,35);

  ypos+=55;

  gui_source_label->setGeometry(10,ypos,size().width()-20,24);
  ypos+=26;

  gui_source_type_label->setGeometry(10,ypos,110,20);
  gui_source_type_box->setGeometry(125,ypos,350,24);
  ypos+=26;

  int ypos_base=ypos;

  //
  // ALSA Controls
  //
  ypos=ypos_base;
  gui_alsa_device_label->setGeometry(10,ypos,160,20);
  gui_alsa_device_edit->setGeometry(175,ypos,100,24);
  ypos+=26;
  
  //
  // FILE Controls
  //
  ypos=ypos_base;
  gui_file_select_button->setGeometry(size().width()-90,ypos+2,80,40);
  ypos+=10;
  gui_file_name_label->setGeometry(10,ypos,160,20);
  gui_file_name_edit->setGeometry(175,ypos,size().width()-275,24);
  ypos+=26;
  
  //
  // ASIHPI Controls
  //
  ypos=ypos_base;
  gui_asihpi_view->setGeometry(125,ypos,350,100);

  //
  // JACK Controls
  //
  ypos=ypos_base;
  gui_jack_server_name_label->setGeometry(10,ypos,145,20);
  gui_jack_server_name_edit->setGeometry(160,ypos,size().width()-260,24);
  ypos+=26;
  gui_jack_client_name_label->setGeometry(10,ypos,145,20);
  gui_jack_client_name_edit->setGeometry(160,ypos,size().width()-260,24);
  ypos+=26;

  //
  // Status Bar
  //
  gui_message_label->setGeometry(0,size().height()-30,size().width()-140,30);
  gui_status_frame_widget->
    setGeometry(size().width()-140,size().height()-30,140,30);
  gui_status_widget->setGeometry(size().width()-143,size().height()-27,134,24);
}


void MainWidget::startEncodingData()
{
  QStringList args;

  if(gui_process!=NULL) {
    QMessageBox::warning(this,"GlassGui - "+tr("Process Error"),
			 tr("Process is not in ready state!"));
    return;
  }
  gui_process=new QProcess(this);
  gui_process->setReadChannel(QProcess::StandardOutput);
  connect(gui_process,SIGNAL(readyRead()),
	  this,SLOT(processReadyReadStandardOutputData()));
  connect(gui_process,SIGNAL(error(QProcess::ProcessError)),
	  this,SLOT(processErrorData(QProcess::ProcessError)));
  connect(gui_process,SIGNAL(finished(int,QProcess::ExitStatus)),
	  this,SLOT(processFinishedData(int,QProcess::ExitStatus)));
  MakeServerArgs(&args);
  gui_codec_dialog->makeArgs(&args);
  gui_stream_dialog->makeArgs(&args,false);
  MakeSourceArgs(&args,false);
  args.push_back("--meter-data");
  args.push_back("--errors-to=STDOUT");
  gui_process->start("glasscoder",args);
  gui_start_button->disconnect();
  connect(gui_start_button,SIGNAL(clicked()),this,SLOT(stopEncodingData()));
  gui_start_button->setText(tr("Stop"));
  LockControls(true);
}


void MainWidget::stopEncodingData()
{
  gui_process->terminate();
}


void MainWidget::showCodeData()
{
  QStringList args;

  args.push_back("glasscoder");
  MakeServerArgs(&args);
  gui_codec_dialog->makeArgs(&args);
  //  MakeStreamArgs(&args,true);
  gui_stream_dialog->makeArgs(&args,true);
  MakeSourceArgs(&args,true);

  gui_codeviewer_dialog->exec(args);
}


void MainWidget::serverTypeChanged(int n)
{
  Connector::ServerType type=
    (Connector::ServerType)gui_server_type_box->itemData(n).toInt();
  bool multirate=false;

  gui_stream_dialog->setServerType(type);

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

  gui_codec_dialog->setMultirate(multirate);
}


void MainWidget::codecData()
{
  if(gui_codec_dialog->isVisible()) {
    gui_codec_dialog->hide();
  }
  else {
    gui_codec_dialog->show();
  }
}


void MainWidget::streamData()
{
  if(gui_stream_dialog->isVisible()) {
    gui_stream_dialog->hide();
  }
  else {
    gui_stream_dialog->show();
  }
}


void MainWidget::sourceTypeChanged(int n)
{
  gui_alsa_device_label->hide();
  gui_alsa_device_edit->hide();

  gui_file_select_button->hide();
  gui_file_name_label->hide();
  gui_file_name_edit->hide();

  gui_asihpi_view->hide();

  gui_jack_server_name_label->hide();
  gui_jack_server_name_edit->hide();
  gui_jack_client_name_label->hide();
  gui_jack_client_name_edit->hide();

  AudioDevice::DeviceType type=
    (AudioDevice::DeviceType)gui_source_type_box->itemData(n).toInt();  

  switch(type) {
  case AudioDevice::Alsa:
    gui_alsa_device_label->show();
    gui_alsa_device_edit->show();
    break;

  case AudioDevice::AsiHpi:
    gui_asihpi_view->show();
    break;

  case AudioDevice::File:
    gui_file_select_button->show();
    gui_file_name_label->show();
    gui_file_name_edit->show();
    break;

  case AudioDevice::Jack:
    gui_jack_server_name_label->show();
    gui_jack_server_name_edit->show();
    gui_jack_client_name_label->show();
    gui_jack_client_name_edit->show();
    break;

  case AudioDevice::LastType:
    break;
  }
}


void MainWidget::codecFinishedData(int exit_code,
				   QProcess::ExitStatus exit_status)
{
  QStringList f0;

  if(exit_code==0) {
    //
    // Populate Codec Types
    //
    gui_codec_dialog->addCodecTypes(gui_process->readAllStandardOutput());

    //
    // Get Device List
    //
    gui_process=new QProcess(this);
    connect(gui_process,SIGNAL(error(QProcess::ProcessError)),
	    this,SLOT(processErrorData(QProcess::ProcessError)));
    connect(gui_process,SIGNAL(finished(int,QProcess::ExitStatus)),
	    this,SLOT(deviceFinishedData(int,QProcess::ExitStatus)));
    
    QStringList args;
    args.push_back("--list-devices");
    gui_process->start("glasscoder",args);
  }
  else {
    ProcessError(exit_code,exit_status);
  }
}


void MainWidget::checkArgs(const QString &str)
{
  QStringList args;
  bool state;

  state=MakeServerArgs(&args);
  state=state&&MakeSourceArgs(&args,false);
  gui_start_button->setEnabled(state);
  gui_code_button->setEnabled(state);
}


void MainWidget::deviceFinishedData(int exit_code,
				    QProcess::ExitStatus exit_status)
{
  QStringList f0;

  if(exit_code==0) {
    //
    // Populate Device Types
    //
    f0=QString(gui_process->readAllStandardOutput()).split("\n");
    for(int i=0;i<f0.size();i++) {
      for(int j=0;j<AudioDevice::LastType;j++) {
	if(f0[i]==AudioDevice::optionKeyword((AudioDevice::DeviceType)j)) {
	  gui_source_type_box->insertItem(-1,
		  AudioDevice::deviceTypeText((AudioDevice::DeviceType)j),j);
	}
      }
    }
    gui_process=NULL;
    LoadSettings();
  }
  else {
    ProcessError(exit_code,exit_status);
  }
}


void MainWidget::processReadyReadStandardOutputData()
{
  char data[1500];
  int n=0;

  if((n=gui_process->read(data,1500))>0) {
    data[n]=0;
    for(int i=0;i<n;i++) {
      switch(0xFF&data[i]) {
      case 13:
	break;

      case 10:
	ProcessFeedback(gui_process_accum);
	gui_process_accum="";
	break;

      default:
	gui_process_accum+=data[i];
      }
    }
  }
}


void MainWidget::processFinishedData(int exit_code,
				     QProcess::ExitStatus exit_status)
{
  if(exit_code==0) {
    if(gui_process_kill_timer->isActive()) {
      exit(0);
    }
    gui_meter->setLeftPeakBar(-10000);
    gui_meter->setRightPeakBar(-10000);
    gui_start_button->disconnect();
    connect(gui_start_button,SIGNAL(clicked()),this,SLOT(startEncodingData()));
    gui_start_button->setText(tr("Start"));
    LockControls(false);
  }
  else {
    ProcessError(exit_code,exit_status);
  }
  gui_status_widget->setStatus(CONNECTION_IDLE);
  gui_process_cleanup_timer->start(0);
}


void MainWidget::processErrorData(QProcess::ProcessError err)
{
  QMessageBox::warning(this,"GlassGui - "+tr("Process Error"),
		       tr("Received QProcess error")+
		       QString().sprintf(": %d",err));
  exit(256);
}


void MainWidget::processCollectGarbageData()
{
  delete gui_process;
  gui_process=NULL;
}


void MainWidget::processKillData()
{
  gui_process->kill();
  qApp->processEvents();
  exit(0);
}


void MainWidget::messageTimeoutData()
{
  gui_message_label->setText("");
}


void MainWidget::fileSelectName()
{
  QString filename;

  if(getenv("HOME")!=NULL) {
    filename=getenv("HOME");
  }
  if(!gui_file_name_edit->text().isEmpty()) {
    filename=gui_file_name_edit->text();
  }
  filename=QFileDialog::getOpenFileName(this,"GlassGui - "+tr("Select File"),
					filename,
	   tr("Audio Files")+" (*.aiff *.AIFF *.wav *.WAV);;All Files (*)");
  if(!filename.isEmpty()) {
    gui_file_name_edit->setText(filename);
  }
}


void MainWidget::LockControls(bool state)
{
  gui_server_type_box->setReadOnly(state);
  gui_server_location_edit->setReadOnly(state);
  gui_server_username_edit->setReadOnly(state);
  gui_server_password_edit->setReadOnly(state);

  gui_codec_dialog->setControlsLocked(state);

  gui_stream_dialog->setControlsLocked(state);

  gui_source_type_box->setReadOnly(state);

  gui_jack_server_name_edit->setReadOnly(state);
  gui_jack_client_name_edit->setReadOnly(state);

  gui_file_name_edit->setReadOnly(state);
  gui_file_select_button->setDisabled(state);

  gui_asihpi_view->setReadOnly(state);

  gui_alsa_device_edit->setReadOnly(state);
}


void MainWidget::ProcessFeedback(const QString &str)
{
  QStringList f0;
  bool ok=false;
  int level;
  int prio;
  int status;
  QString msg;

  f0=str.split(" ");

  if((f0[0]=="CS")&&(f0.size()==2)) {  // Connection Status
    status=f0[1].toInt(&ok);
    if(ok) {
      if(!gui_status_widget->setStatus(status)) {
	gui_message_label->setText(tr("Unknown status code")+
				   QString().sprintf(" \"%d\" ",status)+
				   tr("received."));
      }
    }
  }

  if((f0[0]=="ER")&&(f0.size()>=2)) {  // Error Message
    prio=f0[1].toInt();
    f0.erase(f0.begin());
    f0.erase(f0.begin());
    msg=f0.join(" ");

    switch(prio) {
    case LOG_EMERG:
    case LOG_ALERT:
    case LOG_CRIT:
    case LOG_ERR:
      QMessageBox::critical(this,"GlassGui - "+tr("Error"),msg);
      break;

    case LOG_WARNING:
    case LOG_NOTICE:
    case LOG_INFO:
      gui_message_label->setText(msg);
      gui_message_timer->start(10000);
      break;
    }
    return;
  }

  if(f0[0]=="ME") {  // Meter Levels
    if((f0.size()==2)&&(f0[1].length()==8)) {
      level=f0[1].left(4).toInt(&ok,16);
      if(ok) {
	gui_meter->setLeftPeakBar(-level);
      }
      level=f0[1].right(4).toInt(&ok,16);
      if(ok) {
	gui_meter->setRightPeakBar(-level);
      }
    }
  }
}


bool MainWidget::MakeServerArgs(QStringList *args)
{
  QUrl url(gui_server_location_edit->text());
  if(!url.isValid()) {
    return false;
  }
  Connector::ServerType type=(Connector::ServerType)
    gui_server_type_box->itemData(gui_server_type_box->currentIndex()).toInt();

  args->push_back("--server-type="+Connector::optionKeyword(type));
  args->push_back("--server-hostname="+url.host());
  if(url.port()>0) {
    args->push_back("--server-port="+QString().sprintf("%d",url.port()));
  }
  args->push_back("--server-mountpoint="+url.path());
  if(!gui_server_username_edit->text().isEmpty()) {
    args->push_back("--server-username="+gui_server_username_edit->text());
  }
  if(!gui_server_password_edit->text().isEmpty()) {
    args->push_back("--server-password="+gui_server_password_edit->text());
  }

  return true;
}


bool MainWidget::MakeSourceArgs(QStringList *args,bool escape_args)
{
  QString quote="";
  if(escape_args) {
    quote="\"";
  }
  AudioDevice::DeviceType type=(AudioDevice::DeviceType)
    gui_source_type_box->itemData(gui_source_type_box->currentIndex()).toInt();
  args->push_back("--audio-device="+AudioDevice::optionKeyword(type));

  switch(type) {
  case AudioDevice::Alsa:
    if(!gui_alsa_device_edit->text().isEmpty()) {
      args->push_back("--alsa-device="+gui_alsa_device_edit->text());
    }
    break;

  case AudioDevice::AsiHpi:
    if((gui_asihpi_view->selectedAdapterIndex()==0)||
       (gui_asihpi_view->selectedInputIndex()==0)) {
      return false;
    }
    args->push_back("--asihpi-adapter-index="+
		  QString().sprintf("%u",gui_asihpi_view->selectedAdapterIndex()));
    args->push_back("--asihpi-input-index="+
		    QString().sprintf("%u",gui_asihpi_view->selectedInputIndex()));
    break;

  case AudioDevice::File:
    if(gui_file_name_edit->text().isEmpty()) {
      return false;
    }
    args->push_back("--file-name="+quote+gui_file_name_edit->text()+quote);
    break;

  case AudioDevice::Jack:
    if(!gui_jack_server_name_edit->text().isEmpty()) {
      args->push_back("--jack-server-name="+quote+
		      gui_jack_server_name_edit->text()+quote);
    }
    if(!gui_jack_client_name_edit->text().isEmpty()) {
      args->push_back("--jack-client-name="+quote+
		      gui_jack_client_name_edit->text()+quote);
    }
    break;

  case AudioDevice::LastType:
    break;
  }
  return true;
}


void MainWidget::ProcessError(int exit_code,QProcess::ExitStatus exit_status)
{
  if(exit_status==QProcess::CrashExit) {
    QMessageBox::warning(this,"GlassGui - "+tr("GlassCoder Error"),
			 tr("GlassCoder crashed!"));
  }
  else {
    QString msg=gui_process->readAllStandardError();
    QMessageBox::warning(this,"GlassGui - "+tr("GlassCoder Error"),
			 tr("GlassCoder returned a non-zero exit code")+
			 "\n\""+msg+"\".");
  }
  exit(256);
}


void MainWidget::LoadSettings()
{
  if(CheckSettingsDirectory()) {
    Profile *p=new Profile();
    p->setSource(gui_settings_dir->path()+"/"+GLASSGUI_SETTINGS_FILE);
    gui_server_type_box->
      setCurrentItemData(Connector::serverType(p->stringValue("GlassGui",
							      "ServerType")));
    serverTypeChanged(gui_server_type_box->currentIndex());
    gui_server_location_edit->
      setText(p->stringValue("GlassGui","ServerLocation"));
    gui_server_username_edit->
      setText(p->stringValue("GlassGui","ServerUsername"));
    gui_server_password_edit->
      setText(p->stringValue("GlassGui","ServerPassword"));

    gui_codec_dialog->load(p);

    gui_source_type_box->
      setCurrentItemData(AudioDevice::deviceType(p->stringValue("GlassGui",
	       						"AudioDevice")));
    sourceTypeChanged(gui_source_type_box->currentIndex());

    gui_alsa_device_edit->setText(p->stringValue("GlassGui","AlsaDevice"));

    gui_asihpi_view->setSelected(p->intValue("GlassGui","AsihpiAdapterIndex"),
				 p->intValue("GlassGui","AsihpiInputIndex"));

    gui_file_name_edit->setText(p->stringValue("GlassGui","FileName"));

    gui_jack_server_name_edit->
      setText(p->stringValue("GlassGui","JackServerName"));
    gui_jack_client_name_edit->
      setText(p->stringValue("GlassGui","JackClientName"));
    delete p;
  }
  checkArgs("");
}


bool MainWidget::SaveSettings()
{
  FILE *f;

  if(CheckSettingsDirectory()) {
    QString basepath=gui_settings_dir->path()+"/"+GLASSGUI_SETTINGS_FILE;
    if((f=fopen((basepath+".tmp").toUtf8(),"w"))==NULL) {
      return false;
    }
    fprintf(f,"[GlassGui]\n");
    fprintf(f,"ServerType=%s\n",
	    (const char *)Connector::optionKeyword((Connector::ServerType)
		   gui_server_type_box->currentItemData().toInt()).toUtf8()); 
    fprintf(f,"ServerLocation=%s\n",
	    (const char *)gui_server_location_edit->text().toUtf8());
    fprintf(f,"ServerUsername=%s\n",
	    (const char *)gui_server_username_edit->text().toUtf8());
    fprintf(f,"ServerPassword=%s\n",
	    (const char *)gui_server_password_edit->text().toUtf8());

    gui_codec_dialog->save(f);

    fprintf(f,"AudioDevice=%s\n",
	    (const char *)AudioDevice::optionKeyword((AudioDevice::DeviceType)
		     gui_source_type_box->currentItemData().toInt()).toUtf8()); 
    fprintf(f,"AlsaDevice=%s\n",
	    (const char *)gui_alsa_device_edit->text().toUtf8());

    fprintf(f,"AsihpiAdapterIndex=%u\n",
	    gui_asihpi_view->selectedAdapterIndex());
    fprintf(f,"AsihpiInputIndex=%u\n",gui_asihpi_view->selectedInputIndex());

    fprintf(f,"FileName=%s\n",
	    (const char *)gui_file_name_edit->text().toUtf8());

    fprintf(f,"JackServerName=%s\n",
	    (const char *)gui_jack_server_name_edit->text().toUtf8());
    fprintf(f,"JackClientName=%s\n",
	    (const char *)gui_jack_client_name_edit->text().toUtf8());
    fclose(f);
    rename((basepath+".tmp").toUtf8(),basepath.toUtf8());
  }

  return true;
}


bool MainWidget::CheckSettingsDirectory()
{
  QString path=QString("/")+GLASSGUI_SETTINGS_DIR;

  if(getenv("HOME")!=NULL) {
    path=QString(getenv("HOME"))+"/"+GLASSGUI_SETTINGS_DIR;
  }
  gui_settings_dir=new QDir(path);
  if(!gui_settings_dir->exists()) {
    mkdir(path.toUtf8(),
	  S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
    if(!gui_settings_dir->exists()) {
      return false;
    }
  }
  return true;
}


int main(int argc,char *argv[])
{
  QApplication a(argc,argv);
  MainWidget *w=new MainWidget();
  w->show();
  return a.exec();
}
