// glasscommander.cpp
//
// glasscommander(1) Audio Encoder front end
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

#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QMessageBox>

#include "cmdswitch.h"
#include "logging.h"
#include "profile.h"

#include "glasscommander.h"

#include "../../icons/back.xpm"
#include "../../icons/glasscoder-16x16.xpm"
#include "../../icons/minussign.xpm"
#include "../../icons/plussign.xpm"

MainWidget::MainWidget(QWidget *parent)
  : GuiApplication(parent)
{
  gui_temp_dir=NULL;
  gui_process=NULL;
  gui_starting_all=false;

  CmdSwitch *cmd=new CmdSwitch("glasscommander",GLASSCOMMANDER_USAGE);
  for(unsigned i=0;i<cmd->keys();i++) {
    if(cmd->key(i)=="--instance-directory") {
      if(!setSettingsDirectory(cmd->value(i))) {
	QMessageBox::critical(this,"GlassCommander - "+tr("Error"),
			   tr("Unable to access specified instance directory")+
			      ":\n\""+cmd->value(i)+"\".");
	ExitProgram(1);
      }
      cmd->setProcessed(i,true);
    }
    if(!cmd->processed(i)) {
      QMessageBox::critical(this,"GlassCommander - "+tr("Error"),
			    tr("Unknown argument")+" \""+cmd->key(i)+"\".");
      ExitProgram(256);
    }
  }
  setWindowIcon(QPixmap(glasscoder_16x16_xpm));
  setWindowTitle(QString("GlassCommander v")+VERSION);

  //
  // Create Temp Directory
  //
  char tempdir[PATH_MAX];
  strncpy(tempdir,"/tmp",PATH_MAX);
  if(getenv("TEMP")!=NULL) {
    strncpy(tempdir,getenv("TEMP"),PATH_MAX-1);
  }
  strncat(tempdir,"/glasscommander-XXXXXX",PATH_MAX-strlen(tempdir));
  if(mkdtemp(tempdir)==NULL) {
    QMessageBox::critical(this,"GlassCommander - "+tr("Error"),
			  tr("Unable to create temporary directory in")+
			  "\""+tempdir+"\". ["+strerror(errno)+"]");
    ExitProgram(256);
  }
  gui_temp_dir=new QDir(tempdir);

  //
  // Temp Directory Keepalive
  // (To keep Systemd from "helpfully" deleting the "stale" temp dir)
  //
  QString keepalive_pathname=
    gui_temp_dir->absolutePath()+"/"+GLASSCOMMANDER_TEMP_KEEPALIVE_FILENAME;
  if((gui_temp_keepalive_fd=open(keepalive_pathname.toUtf8(),
				 O_WRONLY|O_CREAT|O_TRUNC,S_IRUSR|S_IWUSR))<0) {
    QMessageBox::critical(this,"GlassCommander - "+tr("Error"),
			  tr("Unable to create keepalive file")+
			  " \""+keepalive_pathname+"\". ["+strerror(errno)+"]");
    ExitProgram(256);
  }
  updateKeepaliveData();
  gui_temp_keepalive_timer=new QTimer(this);
  connect(gui_temp_keepalive_timer,SIGNAL(timeout()),
    this,SLOT(updateKeepaliveData()));
  gui_temp_keepalive_timer->start(GLASSCOMMANDER_TEMP_KEEPALIVE_INTERVAL);

  //
  // Timers
  //
  gui_stop_timer=new QTimer(this);
  gui_stop_timer->setSingleShot(true);
  connect(gui_stop_timer,SIGNAL(timeout()),this,SLOT(stopTimeoutData()));

  gui_autostart_timer=new QTimer(this);
  gui_autostart_timer->setSingleShot(true);
  connect(gui_autostart_timer,SIGNAL(timeout()),this,SLOT(autostartData()));
  gui_autostart_index=0;

  //
  // Fonts
  //
  QFont bold_font(font().family(),font().pointSize(),QFont::Bold);

  //
  // Dialogs
  //
  gui_instance_dialog=new InstanceDialog(settingsDirectory(),this);
  gui_delete_dialog=new DeleteDialog(settingsDirectory(),this);

  //
  // Tool Bar
  //
  gui_toolbar=addToolBar("foo");
  gui_toolbar->setAllowedAreas(Qt::TopToolBarArea);
  QAction *action=
    new QAction(QIcon(QPixmap(plussign_xpm)),tr("Add Instance"),this);
  action->setStatusTip(tr("Add an encoder instance"));
  connect(action,SIGNAL(triggered()),this,SLOT(addInstanceData()));
  gui_toolbar->addAction(action);

  gui_remove_action=
    new QAction(QIcon(QPixmap(minussign_xpm)),tr("Remove Instance"),this);
  gui_remove_action->setStatusTip(tr("Remove an encoder instance"));
  connect(gui_remove_action,SIGNAL(triggered()),
	  this,SLOT(removeInstanceData()));
  gui_toolbar->addAction(gui_remove_action);

  gui_abandon_action=
    new QAction(QIcon(QPixmap(back_xpm)),tr("Abandon change"),this);
  gui_abandon_action->
    setStatusTip(tr("Abandon Add or Removal of encoder instance"));
  gui_abandon_action->setDisabled(true);
  connect(gui_abandon_action,SIGNAL(triggered()),
	  this,SLOT(abandonInstanceData()));
  gui_toolbar->addAction(gui_abandon_action);

  gui_toolbar->addSeparator();

  gui_startall_button=new QPushButton(tr("Start All"),this);
  connect(gui_startall_button,SIGNAL(clicked()),this,SLOT(startAllData()));
  gui_toolbar->addWidget(gui_startall_button);

  gui_stopall_button=new QPushButton(tr("Stop All"),this);
  connect(gui_stopall_button,SIGNAL(clicked()),this,SLOT(stopAllData()));
  gui_toolbar->addWidget(gui_stopall_button);

  gui_insert_button=new QPushButton(tr("Insert"),this);
  gui_insert_button->setFont(bold_font);
  gui_insert_button->setStyleSheet("background-color: yellow");
  connect(gui_insert_button,SIGNAL(clicked()),
	  this,SLOT(topInsertClickedData()));
  gui_insert_button->hide();


  LoadEncoders();

  //
  // Set Size
  //
  setMinimumSize(sizeHint());
  setMaximumHeight(sizeHint().height());

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
  int encoder_height=0;
  if(gui_encoders.size()>0) {
    encoder_height=gui_encoders.at(0)->sizeHint().height()*gui_encoders.size();
  }
  return QSize(1000,10+encoder_height+gui_toolbar->size().height());
}


void MainWidget::addInstanceData()
{
  QStringList used_names;
  for(int i=0;i<gui_encoders.size();i++) {
    used_names.push_back(gui_encoders.at(i)->instanceName());
  }
  if(gui_instance_dialog->exec(&gui_new_instance_name,used_names)) {
    gui_startall_button->setDisabled(true);
    gui_stopall_button->setDisabled(true);
    for(int i=0;i<gui_encoders.size();i++) {
      gui_encoders.at(i)->setMode(GlassWidget::InsertMode);
    }
    gui_abandon_action->setEnabled(true);
    gui_insert_button->show();
  }
}


void MainWidget::removeInstanceData()
{
  gui_startall_button->setDisabled(true);
  gui_stopall_button->setDisabled(true);
  for(int i=0;i<gui_encoders.size();i++) {
    gui_encoders.at(i)->setMode(GlassWidget::RemoveMode);
  }
  gui_insert_button->hide();
  gui_abandon_action->setEnabled(true);
}


void MainWidget::abandonInstanceData()
{
  gui_startall_button->setEnabled(true);
  gui_stopall_button->setEnabled(true);
  for(int i=0;i<gui_encoders.size();i++) {
    gui_encoders.at(i)->setMode(GlassWidget::NormalMode);
  }
  gui_insert_button->hide();
  gui_abandon_action->setDisabled(true);
}


void MainWidget::topInsertClickedData()
{
  insertClickedData("");
}


void MainWidget::insertClickedData(const QString &instance_name)
{
  int pos=1+GetEncoderPosition(instance_name);
  gui_encoders.
    insert(pos,new GlassWidget(gui_new_instance_name,gui_temp_dir,this));
  ConnectEncoder(gui_encoders.at(pos));
  gui_encoders.at(pos)->addCodecTypes(gui_codec_types);
  gui_encoders.at(pos)->addSourceTypes(gui_source_types);

  LoadEncoderConfig(gui_encoders.at(pos));

  int w=size().width();
  int h=size().height()+gui_encoders.at(pos)->sizeHint().height();
  setMaximumHeight(h);
  setMinimumSize(w,h);
  abandonInstanceData();
  gui_remove_action->setEnabled(gui_encoders.size()>0);
  gui_abandon_action->setDisabled(true);
  SaveEncoders();
}


void MainWidget::removeClickedData(const QString &instance_name)
{
  bool delete_instance=false;
  if(gui_delete_dialog->exec(instance_name,&delete_instance)) {
    int pos=GetEncoderPosition(instance_name);
    int w=size().width();
    int h=size().height()-gui_encoders.at(pos)->sizeHint().height();
    delete gui_encoders.at(pos);
    gui_encoders.erase(gui_encoders.begin()+pos);

    setMaximumHeight(h);
    setMinimumSize(w,h);
    abandonInstanceData();
    SaveEncoders();

    if(delete_instance) {
      deleteInstance(instance_name);
    }
  }
  gui_remove_action->setEnabled(gui_encoders.size()>0);
  gui_abandon_action->setDisabled(true);
}


void MainWidget::codecFinishedData(int exit_code,
				   QProcess::ExitStatus exit_status)
{
  QStringList f0;

  if(exit_code==0) {
    //
    // Populate Codec Types
    //
    gui_codec_types=gui_process->readAllStandardOutput();
    for(int i=0;i<gui_encoders.size();i++) {
      gui_encoders.at(i)->addCodecTypes(gui_codec_types);
    }
    gui_process->deleteLater();

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


void MainWidget::deviceFinishedData(int exit_code,
				    QProcess::ExitStatus exit_status)
{
  if(exit_code==0) {
    //
    // Populate Device Types
    //
    gui_source_types=gui_process->readAllStandardOutput();
    for(int i=0;i<gui_encoders.size();i++) {
      gui_encoders.at(i)->addSourceTypes(gui_source_types);
    }
    gui_process->deleteLater();
    gui_process=NULL;
  }
  else {
    ProcessError(exit_code,exit_status);
  }
  for(int i=0;i<gui_encoders.size();i++) {
    LoadEncoderConfig(gui_encoders.at(i));
  }
  autostartData();
}


void MainWidget::processErrorData(QProcess::ProcessError err)
{
  QMessageBox::warning(this,"GlassCommander - "+tr("Process Error"),
		       tr("Received QProcess error")+
		       QString().sprintf(": %d",err));
  ExitProgram(256);
}


void MainWidget::configurationChangedData(GlassWidget *encoder)
{
  FILE *f;
  mode_t mask;

  if(checkSettingsDirectory()) {
    mask=umask(077);
    QString basepath=settingsFilename(encoder->instanceName());
    if((f=fopen((basepath+".tmp").toUtf8(),"w"))==NULL) {
      return;
    }
    fprintf(f,"[GlassGui]\n");
    encoder->save(f);
    fclose(f);
    rename((basepath+".tmp").toUtf8(),basepath.toUtf8());
    umask(mask);
  }
  SaveEncoders();
}


void MainWidget::autostartData()
{
  for(int i=gui_autostart_index;i<gui_encoders.size();i++) {
    if((!gui_encoders.at(i)->isActive())&&
       (gui_starting_all||gui_encoders.at(i)->autoStart())) {
      gui_encoders.at(i)->start();
      gui_autostart_timer->start(1000);
      gui_autostart_index=i+1;
      return;
    }
  }
  gui_starting_all=false;
}


void MainWidget::startAllData()
{
  gui_starting_all=true;
  gui_autostart_index=0;
  autostartData();
}


void MainWidget::stopAllData()
{
  for(int i=0;i<gui_encoders.size();i++) {
    if(gui_encoders.at(i)->isActive()) {
      gui_encoders.at(i)->terminate();
    }
  }
}


void MainWidget::encoderStoppedData()
{
  if(--gui_stop_count==0) {
    ExitProgram(0);
  }
}


void MainWidget::stopTimeoutData()
{
  for(int i=0;i<gui_encoders.size();i++) {
    if(gui_encoders.at(i)->isActive()) {
      gui_encoders.at(i)->kill();
    }
  }
  ExitProgram(1);
}


void MainWidget::updateKeepaliveData()
{
  QByteArray now_stamp=
    QDateTime::currentDateTime().toString("yyyy-MM-ddThh:mm:ss").toUtf8();

  ftruncate(gui_temp_keepalive_fd,0);
  if(write(gui_temp_keepalive_fd,now_stamp,now_stamp.size())<0) {
    syslog(LOG_WARNING,"unable to update keepalive file \"%s\" [%s]",
	   gui_temp_keepalive_pathname.toUtf8().constData(),
	   strerror(errno));
  }
}


void MainWidget::closeEvent(QCloseEvent *e)
{
  bool active=false;

  for(int i=0;i<gui_encoders.size();i++) {
    active=active||gui_encoders.at(i)->isActive();
  }
  if(active) {
    if(QMessageBox::question(this,"GlassCommander - "+tr("Close"),
			     tr("There are still streams active.")+" "+
			     tr("Shutting down will cause them to be stopped.")+
			     "\n"+tr("Shut down?"),
			     QMessageBox::Yes,QMessageBox::No)!=
       QMessageBox::Yes) {
      e->ignore();
      return;
    }
    gui_stop_count=0;
    for(int i=0;i<gui_encoders.size();i++) {
      if(gui_encoders.at(i)->isActive()) {
	connect(gui_encoders.at(i),SIGNAL(stopped()),
		this,SLOT(encoderStoppedData()));
	gui_encoders.at(i)->terminate();
	gui_stop_count++;
      }
    }
    gui_stop_timer->start(10000);
    e->ignore();
    return;
  }
  ExitProgram(0);
}


void MainWidget::resizeEvent(QResizeEvent *e)
{
  gui_insert_button->setGeometry(size().width()-80,5,70,27);

  for(int i=0;i<gui_encoders.size();i++) {
    GlassWidget *enc=gui_encoders.at(i);
    enc->setGeometry(0,gui_toolbar->size().height()+i*enc->sizeHint().height(),
		     size().width(),enc->sizeHint().height());
  }
}


void MainWidget::ConnectEncoder(GlassWidget *encoder)
{
  connect(encoder,SIGNAL(configurationChanged(GlassWidget *)),
	  this,SLOT(configurationChangedData(GlassWidget *)));
  connect(encoder,SIGNAL(insertClicked(const QString &)),
	  this,SLOT(insertClickedData(const QString &)));
  connect(encoder,SIGNAL(removeClicked(const QString &)),
	  this,SLOT(removeClickedData(const QString &)));
  encoder->show();
}


void MainWidget::LoadEncoders()
{
  Profile *p=new Profile();
  int count=0;
  QString name;
  QString section=QString().sprintf("Encoder%d",count+1);
  bool ok=false;

  p->setSource(settingsDirectory()->path()+"/"+GLASSCOMMANDER_SETTINGS_FILE);
  name=p->stringValue(section,"InstanceName","",&ok);
  while(ok) {
    gui_encoders.push_back(new GlassWidget(name,gui_temp_dir,this));
    gui_encoders.back()->setAutoStart(p->boolValue(section,"AutoStart"));
    ConnectEncoder(gui_encoders.back());
    count++;
    section=QString().sprintf("Encoder%d",count+1);
    name=p->stringValue(section,"InstanceName","",&ok);
  }
  delete p;

  gui_remove_action->setEnabled(gui_encoders.size()>0);
}


void MainWidget::SaveEncoders()
{
  FILE *f;
  mode_t mask;

  if(checkSettingsDirectory()) {
    mask=umask(077);
    QString basepath=
      settingsDirectory()->path()+"/"+GLASSCOMMANDER_SETTINGS_FILE;
    if((f=fopen((basepath+".tmp").toUtf8(),"w"))==NULL) {
      umask(mask);
      return;
    }
    for(int i=0;i<gui_encoders.size();i++) {
      fprintf(f,"[Encoder%d]\n",i+1);
      fprintf(f,"InstanceName=%s\n",
	      (const char *)gui_encoders.at(i)->instanceName().toUtf8());
      fprintf(f,"AutoStart=%d\n",gui_encoders.at(i)->autoStart());
      fprintf(f,"\n");
    }
    fclose(f);
    rename((basepath+".tmp").toUtf8(),basepath.toUtf8());
    umask(mask);
  }
}


void MainWidget::LoadEncoderConfig(GlassWidget *encoder)
{
  Profile *p=new Profile();
  p->setSource(settingsFilename(encoder->instanceName()));
  encoder->load(p);
  delete p;  
}


int MainWidget::GetEncoderPosition(const QString &instance_name) const
{
  for(int i=0;i<gui_encoders.size();i++) {
    if(gui_encoders.at(i)->instanceName()==instance_name) {
      return i;
    }
  }
  return -1;
}


void MainWidget::ProcessError(int exit_code,QProcess::ExitStatus exit_status)
{
  if(exit_status==QProcess::CrashExit) {
    QMessageBox::warning(this,"GlassCommander - "+tr("GlassCoder Error"),
			 tr("GlassCoder crashed!"));
  }
  else {
    QString msg=gui_process->readAllStandardError();
    QMessageBox::warning(this,"GlassCommander - "+tr("GlassCoder Error"),
			 tr("GlassCoder returned a non-zero exit code")+
			 "\n\""+msg+"\".");
  }
  ExitProgram(256);
}


void MainWidget::ExitProgram(int exit_code) const
{
  //
  // Clean up temp directory
  //
  if(gui_temp_dir!=NULL) {
    QStringList files=
      gui_temp_dir->entryList(QDir::Files|QDir::NoDotAndDotDot);
    for(int i=0;i<files.size();i++) {
      unlink((gui_temp_dir->path()+"/"+files.at(i)).toUtf8());
    }
    rmdir(gui_temp_dir->path().toUtf8());
  }
  exit(exit_code);
}


int main(int argc,char *argv[])
{
  QApplication a(argc,argv);
  MainWidget *w=new MainWidget();
  w->show();
  return a.exec();
}
