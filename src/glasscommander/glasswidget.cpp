// glasswidget.cpp
//
// Encoder widget for GlassCommander(1)
//
//   (C) Copyright 2016-2022 Fred Gleason <fredg@paravelsystems.com>
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

#include "glasslimits.h"
#include "glasswidget.h"
#include "logging.h"

GlassWidget::GlassWidget(const QString &instance_name,QDir *temp_dir,
			 QWidget *parent)
  : QFrame(parent)
{
  gw_temp_dir=temp_dir;
  gw_process=NULL;
  gw_auto_start=false;

  setFrameStyle(QFrame::Box|QFrame::Raised);
  setMidLineWidth(2);

  //
  // Fonts
  //
  QFont bold_font(font().family(),font().pointSize(),QFont::Bold);

  //
  // Dialogs
  //
  gw_server_dialog=new ServerDialog(gw_temp_dir,"GlassCommander",this);
  gw_codec_dialog=new CodecDialog("GlassCommander",this);
  gw_source_dialog=new SourceDialog("GlassCommander",this);
  connect(gw_source_dialog,SIGNAL(updated()),this,SLOT(checkArgs()));
  gw_stream_dialog=new StreamDialog("GlassCommander",this);
  gw_code_dialog=new CodeViewer("GlassCommander",this);
  gw_config_dialog=new ConfigDialog(instance_name,gw_server_dialog,
				    gw_codec_dialog,gw_stream_dialog,
				    gw_source_dialog,gw_code_dialog,this);
  connect(gw_server_dialog,SIGNAL(typeChanged(Connector::ServerType)),
	  this,SLOT(serverTypeChangedData(Connector::ServerType)));
  connect(gw_server_dialog,SIGNAL(settingsChanged()),this,SLOT(checkArgs()));

  for(int i=0;i<2;i++) {
    gw_meters[i]=new PlayMeter(SegMeter::Right,this);
    gw_meters[i]->setRange(-3000,0);
    gw_meters[i]->setHighThreshold(-800);
    gw_meters[i]->setClipThreshold(-100);
    gw_meters[i]->setMode(SegMeter::Peak);
  }
  gw_meters[0]->setLabel(tr("L"));
  gw_meters[1]->setLabel(tr("R"));


  gw_status_frame_widget=new QLabel(this);
  gw_status_frame_widget->setFrameStyle(QFrame::Box|QFrame::Raised);
  gw_status_widget=new StatusWidget(this);

  gw_name_label=new QLabel(instance_name,this);
  gw_name_label->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
  gw_name_label->setFont(bold_font);

  gw_message_widget=new MessageWidget(this);
  gw_message_widget->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);

  gw_start_button=new QPushButton(tr("Start"),this);
  gw_start_button->setFont(bold_font);
  connect(gw_start_button,SIGNAL(clicked()),this,SLOT(startEncodingData()));

  gw_config_button=new QPushButton(tr("Settings"),this);
  gw_config_button->setFont(bold_font);
  connect(gw_config_button,SIGNAL(clicked()),this,SLOT(configData()));

  gw_insert_button=new QPushButton(tr("Insert"),this);
  gw_insert_button->setFont(bold_font);
  gw_insert_button->setStyleSheet("background-color: yellow");
  connect(gw_insert_button,SIGNAL(clicked()),this,SLOT(insertData()));
  gw_insert_button->hide();

  gw_remove_button=new QPushButton(tr("Remove"),this);
  gw_remove_button->setFont(bold_font);
  gw_remove_button->setStyleSheet("background-color: violet");
  connect(gw_remove_button,SIGNAL(clicked()),this,SLOT(removeData()));
  gw_remove_button->hide();

  //
  // Kill Timer
  //
  gw_kill_timer=new QTimer(this);
  gw_kill_timer->setSingleShot(true);
  connect(gw_kill_timer,SIGNAL(timeout()),this,SLOT(killData()));
}


QSize GlassWidget::sizeHint() const
{
  return QSize(800,36);
}


bool GlassWidget::autoStart() const
{
  return gw_auto_start;
}


void GlassWidget::setAutoStart(bool state)
{
  gw_auto_start=state;
}


void GlassWidget::setMode(GlassWidget::Mode mode)
{
  if(mode!=gw_mode) {
    switch(mode) {
    case GlassWidget::NormalMode:
      gw_insert_button->hide();
      gw_remove_button->hide();
      gw_start_button->show();
      gw_config_button->show();
      break;

    case GlassWidget::InsertMode:
      gw_insert_button->show();
      gw_remove_button->hide();
      gw_start_button->hide();
      gw_config_button->hide();
      break;

    case GlassWidget::RemoveMode:
      gw_insert_button->hide();
      gw_remove_button->show();
      gw_remove_button->setDisabled((gw_process!=NULL)&&
				    (gw_process->state()==QProcess::Running));
      gw_start_button->hide();
      gw_config_button->hide();
      break;
    }
    gw_mode=mode;
  }
}


QString GlassWidget::instanceName() const
{
  return gw_name_label->text();
}


void GlassWidget::addCodecTypes(const QString &codecs)
{
  gw_codec_dialog->addCodecTypes(codecs);
}


void GlassWidget::addSourceTypes(const QString &sources)
{
  gw_source_dialog->addSourceTypes(sources);
}


bool GlassWidget::isActive()
{
  return (gw_process!=NULL)&&(gw_process->state()==QProcess::Running);
}


void GlassWidget::start()
{
  startEncodingData();
}


void GlassWidget::terminate()
{
  if(isActive()) {
    gw_process->terminate();
  }
}


void GlassWidget::kill()
{
  if(isActive()) {
    gw_process->kill();
  }
}


void GlassWidget::load(Profile *p)
{
  gw_server_dialog->load(p);
  gw_codec_dialog->load(p);
  gw_source_dialog->load(p);
  gw_stream_dialog->load(p);
}


void GlassWidget::save(FILE *f) const
{
  gw_server_dialog->save(f);
  gw_codec_dialog->save(f);
  gw_source_dialog->save(f);
  gw_stream_dialog->save(f);
}


void GlassWidget::setNormalMode()
{
  setMode(GlassWidget::NormalMode);
}


void GlassWidget::setInsertMode()
{
  setMode(GlassWidget::InsertMode);
}


void GlassWidget::setRemoveMode()
{
  setMode(GlassWidget::RemoveMode);
}


void GlassWidget::startEncodingData()
{
  QStringList args;

  gw_status_widget->setStatus(CONNECTION_PENDING);

  //
  // Generate Credentials File
  //
  if(!gw_server_dialog->writeCredentials()) {
    QMessageBox::warning(this,"GlassCommander - "+tr("Process Error"),
			 tr("Unable to create credentials file!"));
    return;
  }

  gw_process=new QProcess(this);
  gw_process->setReadChannel(QProcess::StandardOutput);
  connect(gw_process,SIGNAL(readyRead()),
	  this,SLOT(processReadyReadStandardOutputData()));
  connect(gw_process,SIGNAL(error(QProcess::ProcessError)),
	  this,SLOT(processErrorData(QProcess::ProcessError)));
  connect(gw_process,SIGNAL(finished(int,QProcess::ExitStatus)),
	  this,SLOT(processFinishedData(int,QProcess::ExitStatus)));
  gw_server_dialog->makeArgs(&args,false);
  gw_codec_dialog->makeArgs(&args);
  gw_stream_dialog->makeArgs(&args,false);
  gw_source_dialog->makeArgs(&args,false);
  args.push_back("--meter-data");
  args.push_back("--errors-to=STDOUT");
  args.push_back("--errors-string="+gw_name_label->text());

  gw_process->start("glasscoder",args);
  gw_start_button->disconnect();
  connect(gw_start_button,SIGNAL(clicked()),this,SLOT(stopEncodingData()));
  gw_start_button->setText(tr("Stop"));
  LockControls(true);
}


void GlassWidget::stopEncodingData()
{
  gw_status_widget->setStatus(CONNECTION_STOPPING);
  gw_process->terminate();
  gw_kill_timer->start(PROCESS_TERMINATION_TIMEOUT);
}


void GlassWidget::processReadyReadStandardOutputData()
{
  char data[1500];
  int n=0;

  if((n=gw_process->read(data,1500))>0) {
    data[n]=0;
    for(int i=0;i<n;i++) {
      switch(0xFF&data[i]) {
      case 13:
	break;

      case 10:
	ProcessFeedback(gw_process_accum);
	gw_process_accum="";
	break;

      default:
	gw_process_accum+=data[i];
      }
    }
  }
}


void GlassWidget::processFinishedData(int exit_code,
				     QProcess::ExitStatus exit_status)
{
  if(exit_code==0) {
    gw_meters[0]->setPeakBar(-10000);
    gw_meters[1]->setPeakBar(-10000);
    gw_start_button->disconnect();
    connect(gw_start_button,SIGNAL(clicked()),this,SLOT(startEncodingData()));
    gw_start_button->setText(tr("Start"));
    LockControls(false);
  }
  else {
    ProcessError(exit_code,exit_status);
  }
  gw_kill_timer->stop();
  gw_status_widget->setStatus(CONNECTION_IDLE);
  gw_process->deleteLater();
  gw_process=NULL;
  emit stopped();
}


void GlassWidget::processErrorData(QProcess::ProcessError err)
{
  printf("processErrorData(%u)\n",err);
  /*
  QMessageBox::warning(this,"GlassGui - "+tr("Process Error"),
		       tr("Received QProcess error")+
		       QString().sprintf(": %d",err));
  exit(256);
  */
}


void GlassWidget::insertData()
{
  emit insertClicked(gw_name_label->text());
}


void GlassWidget::removeData()
{
  emit removeClicked(gw_name_label->text());
}


void GlassWidget::configData()
{
  bool autostart=autoStart();
  gw_config_dialog->exec(&autostart);
  setAutoStart(autostart);
  emit configurationChanged(this);
}


void GlassWidget::checkArgs()
{
  QStringList args;
  bool state;

  state=gw_server_dialog->makeArgs(&args,false);
  state=state&&gw_source_dialog->makeArgs(&args,false);
  gw_start_button->setEnabled(state);
}


void GlassWidget::serverTypeChangedData(Connector::ServerType type)
{
  gw_stream_dialog->setServerType(type);
}


void GlassWidget::killData()
{
  gw_message_widget->addMessage(tr("Invoked process kill!"));
  kill();
}


void GlassWidget::resizeEvent(QResizeEvent *e)
{
  int w=size().width();
  int h=size().height();

  gw_meters[0]->setGeometry(4,4,300,h/2-4);
  gw_meters[1]->setGeometry(4,h/2,300,h/2-4);

  gw_status_frame_widget->setGeometry(307,4,134,h-8);
  gw_status_widget->setGeometry(310,7,128,h-14);

  gw_name_label->setGeometry(445,2,190,h-4);

  gw_message_widget->setGeometry(640,2,w-810,h-4);

  gw_start_button->setGeometry(w-160,5,70,h-9);

  gw_config_button->setGeometry(w-80,5,70,h-9);
  gw_insert_button->setGeometry(w-80,5,70,h-9);
  gw_remove_button->setGeometry(w-80,5,70,h-9);
}


void GlassWidget::ProcessFeedback(const QString &str)
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
      if(!gw_status_widget->setStatus(status)) {
	gw_message_widget->addMessage(tr("Unknown status code")+
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
      gw_message_widget->addMessage(msg);
      break;

    case LOG_WARNING:
    case LOG_NOTICE:
    case LOG_INFO:
      gw_message_widget->addMessage(msg);
      break;
    }
    return;
  }

  if(f0[0]=="ME") {  // Meter Levels
    if((f0.size()==2)&&(f0[1].length()==8)) {
      level=f0[1].left(4).toInt(&ok,16);
      if(ok) {
	gw_meters[0]->setPeakBar(-level);
      }
      level=f0[1].right(4).toInt(&ok,16);
      if(ok) {
	gw_meters[1]->setPeakBar(-level);
      }
    }
  }
}


void GlassWidget::ProcessError(int exit_code,QProcess::ExitStatus exit_status)
{
  printf("ProcessError(%d,%d)\n",exit_code,exit_status);
  /*
  if(exit_status==QProcess::CrashExit) {
    QMessageBox::warning(this,"GlassGui - "+tr("GlassCoder Error"),
			 tr("GlassCoder crashed!"));
  }
  else {
    QString msg=gw_process->readAllStandardError();
    QMessageBox::warning(this,"GlassGui - "+tr("GlassCoder Error"),
			 tr("GlassCoder returned a non-zero exit code")+
			 "\n\""+msg+"\".");
  }
  exit(256);
  */
}


void GlassWidget::LockControls(bool state)
{
  gw_server_dialog->setControlsLocked(state);
  gw_codec_dialog->setControlsLocked(state);
  gw_stream_dialog->setControlsLocked(state);
  gw_source_dialog->setControlsLocked(state);
}
