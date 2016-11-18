// glasscommander.cpp
//
// glasscommander(1) Audio Encoder front end
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

#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
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
  gui_process=NULL;

  CmdSwitch *cmd=
    new CmdSwitch(qApp->argc(),qApp->argv(),"glasscommander",GLASSCOMMANDER_USAGE);
  for(unsigned i=0;i<cmd->keys();i++) {
    if(!cmd->processed(i)) {
      QMessageBox::critical(this,"GlassCommander - "+tr("Error"),
			    tr("Unknown argument")+" \""+cmd->key(i)+"\".");
      exit(256);
    }
  }
  setWindowIcon(QPixmap(glasscoder_16x16_xpm));
  setWindowTitle(QString("GlassCommander v")+VERSION);

  //
  // Stop Timer
  //
  gui_stop_timer=new QTimer(this);
  gui_stop_timer->setSingleShot(true);
  connect(gui_stop_timer,SIGNAL(timeout()),this,SLOT(stopTimeoutData()));

  //
  // Fonts
  //
  QFont bold_font(font().family(),font().pointSize(),QFont::Bold);

  //
  // Tool Bar
  //
  gui_toolbar=addToolBar("foo");
  gui_toolbar->setAllowedAreas(Qt::TopToolBarArea);
  QAction *action=new QAction(QIcon(plussign_xpm),tr("Add Instance"),this);
  action->setStatusTip(tr("Add an encoder instance"));
  connect(action,SIGNAL(triggered()),this,SLOT(addInstanceData()));
  gui_toolbar->addAction(action);

  action=new QAction(QIcon(minussign_xpm),tr("Remove Instance"),this);
  action->setStatusTip(tr("Remove an encoder instance"));
  connect(action,SIGNAL(triggered()),this,SLOT(removeInstanceData()));
  gui_toolbar->addAction(action);

  action=new QAction(QIcon(back_xpm),tr("Abandon change"),this);
  action->setStatusTip(tr("Abandon Add or Removal of encoder instance"));
  connect(action,SIGNAL(triggered()),this,SLOT(abandonInstanceData()));
  gui_toolbar->addAction(action);

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
  gui_startall_button->setDisabled(true);
  gui_stopall_button->setDisabled(true);
  for(int i=0;i<gui_encoders.size();i++) {
    gui_encoders.at(i)->setMode(GlassWidget::InsertMode);
  }
  gui_insert_button->show();
}


void MainWidget::removeInstanceData()
{
  gui_startall_button->setDisabled(true);
  gui_stopall_button->setDisabled(true);
  for(int i=0;i<gui_encoders.size();i++) {
    gui_encoders.at(i)->setMode(GlassWidget::RemoveMode);
  }
  gui_insert_button->hide();
}


void MainWidget::abandonInstanceData()
{
  gui_startall_button->setEnabled(true);
  gui_stopall_button->setEnabled(true);
  for(int i=0;i<gui_encoders.size();i++) {
    gui_encoders.at(i)->setMode(GlassWidget::NormalMode);
  }
  gui_insert_button->hide();
}


void MainWidget::topInsertClickedData()
{
  insertClickedData("");
}


void MainWidget::insertClickedData(const QString &instance_name)
{
  int pos=1+GetEncoderPosition(instance_name);
  gui_encoders.insert(pos,new GlassWidget("new instance",this));
  InitEncoder(gui_encoders.at(pos));
  int w=size().width();
  int h=size().height()+gui_encoders.at(pos)->sizeHint().height();
  setMaximumHeight(h);
  setMinimumSize(w,h);
  abandonInstanceData();
  SaveEncoders();
}


void MainWidget::removeClickedData(const QString &instance_name)
{
  int pos=GetEncoderPosition(instance_name);
  int w=size().width();
  int h=size().height()-gui_encoders.at(pos)->sizeHint().height();
  delete gui_encoders.at(pos);
  gui_encoders.erase(gui_encoders.begin()+pos);

  setMaximumHeight(h);
  setMinimumSize(w,h);
  abandonInstanceData();
  SaveEncoders();
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
    Profile *p=new Profile();
    p->setSource(settingsFilename(gui_encoders.at(i)->instanceName()));
    gui_encoders.at(i)->load(p);
    delete p;
  }
}


void MainWidget::processErrorData(QProcess::ProcessError err)
{
  QMessageBox::warning(this,"GlassCommander - "+tr("Process Error"),
		       tr("Received QProcess error")+
		       QString().sprintf(": %d",err));
  exit(256);
}


void MainWidget::configurationChangedData(GlassWidget *encoder)
{
  FILE *f;

  if(checkSettingsDirectory()) {
    QString basepath=settingsFilename(encoder->instanceName());
    if((f=fopen((basepath+".tmp").toUtf8(),"w"))==NULL) {
      return;
    }
    fprintf(f,"[GlassGui]\n");
    encoder->save(f);
    fclose(f);
    rename((basepath+".tmp").toUtf8(),basepath.toUtf8());
  }
}


void MainWidget::startAllData()
{
  for(int i=0;i<gui_encoders.size();i++) {
    if(!gui_encoders.at(i)->isActive()) {
      gui_encoders.at(i)->start();
    }
  }
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
    exit(0);
  }
}


void MainWidget::stopTimeoutData()
{
  for(int i=0;i<gui_encoders.size();i++) {
    if(gui_encoders.at(i)->isActive()) {
      gui_encoders.at(i)->kill();
    }
  }
  exit(1);
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
  e->accept();
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


void MainWidget::InitEncoder(GlassWidget *encoder)
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
    gui_encoders.push_back(new GlassWidget(name,this));
    InitEncoder(gui_encoders.back());
    count++;
    section=QString().sprintf("Encoder%d",count+1);
    name=p->stringValue(section,"InstanceName","",&ok);
  }

  delete p;
}


void MainWidget::SaveEncoders()
{
  FILE *f;

  if(checkSettingsDirectory()) {
    QString basepath=
      settingsDirectory()->path()+"/"+GLASSCOMMANDER_SETTINGS_FILE;
    if((f=fopen((basepath+".tmp").toUtf8(),"w"))==NULL) {
      return;
    }
    for(int i=0;i<gui_encoders.size();i++) {
      fprintf(f,"[Encoder%d]\n",i+1);
      fprintf(f,"InstanceName=%s\n",
	      (const char *)gui_encoders.at(i)->instanceName().toUtf8());
      fprintf(f,"\n");
    }
    fclose(f);
    rename((basepath+".tmp").toUtf8(),basepath.toUtf8());
  }
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
  exit(256);
}


int main(int argc,char *argv[])
{
  QApplication a(argc,argv);
  MainWidget *w=new MainWidget();
  w->show();
  return a.exec();
}
