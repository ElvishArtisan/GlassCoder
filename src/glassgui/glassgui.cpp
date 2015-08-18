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

#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QUrl>

#include "audiodevice.h"
#include "cmdswitch.h"
#include "codec.h"
#include "connector.h"

#include "glassgui.h"

MainWidget::MainWidget(QWidget *parent)
  : QMainWindow(parent)
{
  CmdSwitch *cmd=
    new CmdSwitch(qApp->argc(),qApp->argv(),"glassgui",GLASSGUI_USAGE);
  for(unsigned i=0;i<cmd->keys();i++) {
  }

  setWindowTitle(QString("GlassGui v")+VERSION);

  //
  // Fonts
  //
  QFont section_font("helvetica",16,QFont::Bold);
  section_font.setPixelSize(16);
  QFont label_font("helvetica",14,QFont::Bold);
  label_font.setPixelSize(14);

  //
  // Set Size
  //
  setMinimumSize(sizeHint());

  //
  // Meter
  //
  gui_meter=new StereoMeter(this);
  gui_start_button=new QPushButton(tr("Start"),this);
  gui_start_button->setFont(section_font);
  connect(gui_start_button,SIGNAL(clicked()),this,SLOT(startEncodingData()));
  //  gui_start_button->setDisabled(true);

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
  gui_server_type_box=new QComboBox(this);
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
  // Codec Section
  //
  gui_codec_label=new QLabel(tr("Codec Settings"),this);
  gui_codec_label->setFont(section_font);

  //
  // Codec Type
  //
  gui_codec_type_label=new QLabel(tr("Type")+":",this);
  gui_codec_type_label->setFont(label_font);
  gui_codec_type_label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
  gui_codec_type_box=new QComboBox(this);
  connect(gui_codec_type_box,SIGNAL(activated(int)),
	  this,SLOT(codecTypeChanged(int)));

  //
  // Codec Samplerate
  //
  gui_codec_samplerate_label=new QLabel(tr("Sample Rate")+":",this);
  gui_codec_samplerate_label->setFont(label_font);
  gui_codec_samplerate_label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
  gui_codec_samplerate_box=new QComboBox(this);
  connect(gui_codec_samplerate_box,SIGNAL(activated(int)),
	  this,SLOT(codecSamplerateChanged(int)));

  //
  // Codec Channels
  //
  gui_codec_channels_label=new QLabel(tr("Channels")+":",this);
  gui_codec_channels_label->setFont(label_font);
  gui_codec_channels_label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
  gui_codec_channels_box=new QComboBox(this);

  //
  // Codec Bitrate
  //
  gui_codec_bitrate_label=new QLabel(tr("Bit Rate")+":",this);
  gui_codec_bitrate_label->setFont(label_font);
  gui_codec_bitrate_label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
  gui_codec_bitrate_box=new QComboBox(this);

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
  gui_source_type_box=new QComboBox(this);
  connect(gui_source_type_box,SIGNAL(activated(int)),
	  this,SLOT(sourceTypeChanged(int)));

  //
  // FILE Fields
  //
  gui_file_name_label=new QLabel(tr("Filename")+":",this);
  gui_file_name_label->setFont(label_font);
  gui_file_name_label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
  gui_file_name_label->hide();
  gui_file_name_edit=new QLineEdit(this);
  gui_file_name_edit->hide();
  gui_file_select_button=new QPushButton(tr("Select"),this);
  connect(gui_file_select_button,SIGNAL(clicked()),this,SLOT(fileSelectName()));

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
  return QSize(560,600);
}


void MainWidget::resizeEvent(QResizeEvent *e)
{
  int ypos=0;

  gui_meter->setGeometry(10,10,gui_meter->sizeHint().width(),
			 gui_meter->sizeHint().height());
  gui_start_button->setGeometry(gui_meter->sizeHint().width()+20,15,size().width()-gui_meter->sizeHint().width()-30,gui_meter->sizeHint().height()-10);
  ypos+=(gui_meter->sizeHint().height()+15);

  gui_server_label->setGeometry(10,ypos,size().width()-20,24);
  ypos+=23;

  gui_server_type_label->setGeometry(10,ypos,110,24);
  gui_server_type_box->setGeometry(125,ypos,size().width()-135,24);
  ypos+=26;

  gui_server_location_label->setGeometry(10,ypos,110,24);
  gui_server_location_edit->setGeometry(125,ypos,size().width()-135,24);
  ypos+=26;

  gui_server_username_label->setGeometry(10,ypos,110,24);
  gui_server_username_edit->setGeometry(125,ypos,size().width()-135,24);
  ypos+=26;

  gui_server_password_label->setGeometry(10,ypos,110,24);
  gui_server_password_edit->setGeometry(125,ypos,size().width()-135,24);
  ypos+=35;

  gui_codec_label->setGeometry(10,ypos,size().width()-20,24);
  ypos+=23;

  gui_codec_type_label->setGeometry(10,ypos,110,24);
  gui_codec_type_box->setGeometry(125,ypos,size().width()-135,24);
  ypos+=26;

  gui_codec_samplerate_label->setGeometry(10,ypos,110,24);
  gui_codec_samplerate_box->setGeometry(125,ypos,200,24);
  ypos+=26;

  gui_codec_channels_label->setGeometry(10,ypos,110,24);
  gui_codec_channels_box->setGeometry(125,ypos,50,24);
  ypos+=26;

  gui_codec_bitrate_label->setGeometry(10,ypos,110,24);
  gui_codec_bitrate_box->setGeometry(125,ypos,150,24);
  ypos+=35;

  gui_source_label->setGeometry(10,ypos,size().width()-20,24);
  ypos+=23;

  gui_source_type_label->setGeometry(10,ypos,110,20);
  gui_source_type_box->setGeometry(125,ypos,size().width()-135,24);
  ypos+=26;

  int ypos_base=ypos;

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
  // JACK Controls
  //
  ypos=ypos_base;
  gui_jack_server_name_label->setGeometry(10,ypos,160,20);
  gui_jack_server_name_edit->setGeometry(175,ypos,size().width()-275,24);
  ypos+=26;
  gui_jack_client_name_label->setGeometry(10,ypos,160,20);
  gui_jack_client_name_edit->setGeometry(175,ypos,size().width()-275,24);
  ypos+=26;
  
}


void MainWidget::startEncodingData()
{
  QStringList args;

  if(!MakeServerArgs(&args)) {
    printf("SERVER BAD!\n");
    return;
  }
  if(!MakeCodecArgs(&args)) {
    printf("CODEC BAD!\n");
    return;
  }
  if(!MakeSourceArgs(&args)) {
    printf("SOURCE BAD!\n");
    return;
  }
  printf("glasscoder %s\n",(const char *)args.join(" ").toUtf8());
}


void MainWidget::stopEncodingData()
{
}


void MainWidget::serverTypeChanged(int n)
{
  Connector::ServerType type=(Connector::ServerType)gui_server_type_box->itemData(n).toInt();

  gui_server_username_edit->setText(Connector::defaultUsername(type));
}


void MainWidget::codecTypeChanged(int n)
{
  Codec::Type type=(Codec::Type)gui_codec_type_box->itemData(n).toInt();

  gui_codec_samplerate_box->clear();
  gui_codec_channels_box->clear();
  gui_codec_bitrate_box->clear();

  switch(type) {
  case Codec::TypeAac:
    gui_codec_samplerate_box->insertItem(-1,"32000 samples/sec",32000);
    gui_codec_samplerate_box->insertItem(-1,"44100 samples/sec",44100);
    gui_codec_samplerate_box->insertItem(-1,"48000 samples/sec",48000);

    gui_codec_channels_box->insertItem(-1,"1",2);
    gui_codec_channels_box->insertItem(-1,"2",2);

    gui_codec_bitrate_label->setText(tr("Bit Rate")+":");
    break;

  case Codec::TypeHeAac:
    gui_codec_samplerate_box->insertItem(-1,"32000 samples/sec",32000);
    gui_codec_samplerate_box->insertItem(-1,"44100 samples/sec",44100);
    gui_codec_samplerate_box->insertItem(-1,"48000 samples/sec",48000);

    gui_codec_channels_box->insertItem(-1,"2",2);

    gui_codec_bitrate_label->setText(tr("Bit Rate")+":");
    break;

  case Codec::TypeMpegL2:
  case Codec::TypeMpegL3:
    gui_codec_samplerate_box->insertItem(-1,"16000 samples/sec",16000);
    gui_codec_samplerate_box->insertItem(-1,"22050 samples/sec",22050);
    gui_codec_samplerate_box->insertItem(-1,"24000 samples/sec",24000);
    gui_codec_samplerate_box->insertItem(-1,"32000 samples/sec",32000);
    gui_codec_samplerate_box->insertItem(-1,"44100 samples/sec",44100);
    gui_codec_samplerate_box->insertItem(-1,"48000 samples/sec",48000);

    gui_codec_channels_box->insertItem(-1,"1",2);
    gui_codec_channels_box->insertItem(-1,"2",2);

    gui_codec_bitrate_label->setText(tr("Bit Rate")+":");
    break;

  case Codec::TypeVorbis:
    gui_codec_samplerate_box->insertItem(-1,"16000 samples/sec",16000);
    gui_codec_samplerate_box->insertItem(-1,"22050 samples/sec",22050);
    gui_codec_samplerate_box->insertItem(-1,"24000 samples/sec",24000);
    gui_codec_samplerate_box->insertItem(-1,"32000 samples/sec",32000);
    gui_codec_samplerate_box->insertItem(-1,"44100 samples/sec",44100);
    gui_codec_samplerate_box->insertItem(-1,"48000 samples/sec",48000);

    gui_codec_channels_box->insertItem(-1,"1",2);
    gui_codec_channels_box->insertItem(-1,"2",2);

    gui_codec_bitrate_label->setText(tr("Quality")+":");
    break;

  case Codec::TypeOpus:
    break;

  case Codec::TypeLast:
    break;
  }
  codecSamplerateChanged(0);
}


void MainWidget::codecSamplerateChanged(int n)
{
  Codec::Type type=(Codec::Type)gui_codec_type_box->
    itemData(gui_codec_type_box->currentIndex()).toInt();
  unsigned samprate=gui_codec_samplerate_box->itemData(n).toUInt();

  gui_codec_bitrate_box->clear();

  switch(type) {
  case Codec::TypeAac:
    gui_codec_bitrate_box->insertItem(-1,"16 kbits/sec",16);
    gui_codec_bitrate_box->insertItem(-1,"24 kbits/sec",24);
    gui_codec_bitrate_box->insertItem(-1,"32 kbits/sec",32);
    gui_codec_bitrate_box->insertItem(-1,"40 kbits/sec",40);
    gui_codec_bitrate_box->insertItem(-1,"48 kbits/sec",48);
    break;

  case Codec::TypeHeAac:
    switch(samprate) {
    case 32000:
      gui_codec_bitrate_box->insertItem(-1,"32 kbits/sec",32);
      break;

    case 44100:
    case 48000:
      gui_codec_bitrate_box->insertItem(-1,"32 kbits/sec",32);
      gui_codec_bitrate_box->insertItem(-1,"48 kbits/sec",48);
      gui_codec_bitrate_box->insertItem(-1,"56 kbits/sec",56);
      gui_codec_bitrate_box->insertItem(-1,"64 kbits/sec",64);
      gui_codec_bitrate_box->insertItem(-1,"96 kbits/sec",96);
      gui_codec_bitrate_box->insertItem(-1,"128 kbits/sec",128);
      break;
    }
    break;

  case Codec::TypeMpegL2:
    gui_codec_bitrate_box->insertItem(-1,"8 kbits/sec",8);
    gui_codec_bitrate_box->insertItem(-1,"16 kbits/sec",16);
    gui_codec_bitrate_box->insertItem(-1,"24 kbits/sec",24);
    gui_codec_bitrate_box->insertItem(-1,"32 kbits/sec",32);
    gui_codec_bitrate_box->insertItem(-1,"40 kbits/sec",40);
    gui_codec_bitrate_box->insertItem(-1,"48 kbits/sec",48);
    gui_codec_bitrate_box->insertItem(-1,"56 kbits/sec",56);
    gui_codec_bitrate_box->insertItem(-1,"64 kbits/sec",64);
    gui_codec_bitrate_box->insertItem(-1,"80 kbits/sec",80);
    gui_codec_bitrate_box->insertItem(-1,"96 kbits/sec",96);
    gui_codec_bitrate_box->insertItem(-1,"112 kbits/sec",112);
    gui_codec_bitrate_box->insertItem(-1,"128 kbits/sec",128);
    gui_codec_bitrate_box->insertItem(-1,"144 kbits/sec",144);
    gui_codec_bitrate_box->insertItem(-1,"160 kbits/sec",160);
    gui_codec_bitrate_box->insertItem(-1,"192 kbits/sec",192);
    gui_codec_bitrate_box->insertItem(-1,"224 kbits/sec",224);
    gui_codec_bitrate_box->insertItem(-1,"256 kbits/sec",256);
    gui_codec_bitrate_box->insertItem(-1,"320 kbits/sec",320);
    gui_codec_bitrate_box->insertItem(-1,"384 kbits/sec",384);
    break;

  case Codec::TypeMpegL3:
    gui_codec_bitrate_box->insertItem(-1,"8 kbits/sec",8);
    gui_codec_bitrate_box->insertItem(-1,"16 kbits/sec",16);
    gui_codec_bitrate_box->insertItem(-1,"24 kbits/sec",24);
    gui_codec_bitrate_box->insertItem(-1,"32 kbits/sec",32);
    gui_codec_bitrate_box->insertItem(-1,"40 kbits/sec",40);
    gui_codec_bitrate_box->insertItem(-1,"48 kbits/sec",48);
    gui_codec_bitrate_box->insertItem(-1,"56 kbits/sec",56);
    gui_codec_bitrate_box->insertItem(-1,"64 kbits/sec",64);
    gui_codec_bitrate_box->insertItem(-1,"80 kbits/sec",80);
    gui_codec_bitrate_box->insertItem(-1,"96 kbits/sec",96);
    gui_codec_bitrate_box->insertItem(-1,"112 kbits/sec",112);
    gui_codec_bitrate_box->insertItem(-1,"128 kbits/sec",128);
    gui_codec_bitrate_box->insertItem(-1,"144 kbits/sec",144);
    gui_codec_bitrate_box->insertItem(-1,"160 kbits/sec",160);
    gui_codec_bitrate_box->insertItem(-1,"192 kbits/sec",192);
    gui_codec_bitrate_box->insertItem(-1,"224 kbits/sec",224);
    gui_codec_bitrate_box->insertItem(-1,"256 kbits/sec",256);
    gui_codec_bitrate_box->insertItem(-1,"320 kbits/sec",320);
    break;

  case Codec::TypeVorbis:
    gui_codec_bitrate_box->insertItem(-1,"0",0);
    gui_codec_bitrate_box->insertItem(-1,"1",1);
    gui_codec_bitrate_box->insertItem(-1,"2",2);
    gui_codec_bitrate_box->insertItem(-1,"3",3);
    gui_codec_bitrate_box->insertItem(-1,"4",4);
    gui_codec_bitrate_box->insertItem(-1,"5",5);
    gui_codec_bitrate_box->insertItem(-1,"6",6);
    gui_codec_bitrate_box->insertItem(-1,"7",7);
    gui_codec_bitrate_box->insertItem(-1,"8",8);
    gui_codec_bitrate_box->insertItem(-1,"9",9);
    gui_codec_bitrate_box->insertItem(-1,"10",10);
    break;

  case Codec::TypeOpus:
    break;

  case Codec::TypeLast:
    break;
  }
  
}


void MainWidget::sourceTypeChanged(int n)
{
  gui_file_select_button->hide();
  gui_file_name_label->hide();
  gui_file_name_edit->hide();

  gui_jack_server_name_label->hide();
  gui_jack_server_name_edit->hide();
  gui_jack_client_name_label->hide();
  gui_jack_client_name_edit->hide();

  AudioDevice::DeviceType type=
    (AudioDevice::DeviceType)gui_source_type_box->itemData(n).toInt();  

  switch(type) {
  case AudioDevice::Alsa:
    break;

  case AudioDevice::AsiHpi:
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
    f0=QString(gui_process->readAllStandardOutput()).split("\n");
    for(int i=0;i<f0.size();i++) {
      for(int j=0;j<Codec::TypeLast;j++) {
	if(f0[i]==Codec::optionKeyword((Codec::Type)j)) {
	  gui_codec_type_box->
	    insertItem(-1,Codec::codecTypeText((Codec::Type)j),j);
	}
      }
    }
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
    codecTypeChanged(0);
    sourceTypeChanged(0);
  }
  else {
    ProcessError(exit_code,exit_status);
  }
}


void MainWidget::processFinishedData(int exit_code,
				     QProcess::ExitStatus exit_status)
{
  if(exit_code==0) {
  }
  else {
    ProcessError(exit_code,exit_status);
  }
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
  args->push_back("--server-username="+gui_server_username_edit->text());
  if(!gui_server_password_edit->text().isEmpty()) {
    args->push_back("--server-password="+gui_server_password_edit->text());
  }

  return true;
}


bool MainWidget::MakeCodecArgs(QStringList *args)
{
  Codec::Type type=(Codec::Type)
    gui_codec_type_box->itemData(gui_codec_type_box->currentIndex()).toInt();

  args->push_back("--audio-format="+Codec::optionKeyword(type));
  args->push_back("--audio-samplerate="+QString().sprintf("%u",
	     gui_codec_samplerate_box->
		 itemData(gui_codec_samplerate_box->currentIndex()).toUInt()));
  args->push_back("--audio-channels="+QString().sprintf("%u",
	     gui_codec_channels_box->
		 itemData(gui_codec_channels_box->currentIndex()).toUInt()));
  switch(type) {
  case Codec::TypeMpegL2:
  case Codec::TypeMpegL3:
  case Codec::TypeAac:
  case Codec::TypeHeAac:
  case Codec::TypeOpus:
  case Codec::TypeLast:
    args->push_back("--audio-bitrate="+QString().sprintf("%u",
	     gui_codec_bitrate_box->
		 itemData(gui_codec_bitrate_box->currentIndex()).toUInt()));
    break;

  case Codec::TypeVorbis:
    args->push_back("--audio-quality="+QString().sprintf("%u",
	     gui_codec_bitrate_box->
		 itemData(gui_codec_bitrate_box->currentIndex()).toUInt()));
    break;
  }
  return true;
}


bool MainWidget::MakeSourceArgs(QStringList *args)
{
  AudioDevice::DeviceType type=(AudioDevice::DeviceType)
    gui_source_type_box->itemData(gui_source_type_box->currentIndex()).toInt();
  args->push_back("--audio-device="+AudioDevice::optionKeyword(type));

  switch(type) {
  case AudioDevice::Alsa:
    break;

  case AudioDevice::AsiHpi:
    break;

  case AudioDevice::File:
    if(gui_file_name_edit->text().isEmpty()) {
      return false;
    }
    args->push_back("--file-name="+gui_file_name_edit->text());
    break;

  case AudioDevice::Jack:
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


int main(int argc,char *argv[])
{
  QApplication a(argc,argv);
  MainWidget *w=new MainWidget();
  w->show();
  return a.exec();
}
