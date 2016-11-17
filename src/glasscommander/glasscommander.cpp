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

#include <QApplication>
#include <QCloseEvent>
#include <QMessageBox>

#include "cmdswitch.h"
#include "logging.h"
#include "profile.h"

#include "glasscommander.h"

#include "../../icons/glasscoder-16x16.xpm"

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
  if(gui_encoders.size()>0) {
    return 
      QSize(1000,gui_encoders.at(0)->sizeHint().height()*gui_encoders.size());
  }
  return QSize(1000,10);
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


void MainWidget::closeEvent(QCloseEvent *e)
{
  e->accept();
}


void MainWidget::resizeEvent(QResizeEvent *e)
{
  for(int i=0;i<gui_encoders.size();i++) {
    GlassWidget *enc=gui_encoders.at(i);
    enc->setGeometry(0,i*enc->sizeHint().height(),size().width(),enc->sizeHint().height());
  }
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
    connect(gui_encoders.back(),SIGNAL(configurationChanged(GlassWidget *)),
	    this,SLOT(configurationChangedData(GlassWidget *)));
    count++;
    section=QString().sprintf("Encoder%d",count+1);
    name=p->stringValue(section,"InstanceName","",&ok);
  }

  delete p;
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
