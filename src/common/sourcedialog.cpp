// sourcedialog.cpp
//
// Configuration dialog for source settings
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

#include <QFileDialog>

#include "asihpi.h"
#include "sourcedialog.h"

SourceDialog::SourceDialog(QWidget *parent)
  : QDialog(parent)
{
  //
  // Fonts
  //
  QFont label_font("helvetica",14,QFont::Bold);
  label_font.setPixelSize(14);

  setWindowTitle("GlassGui - "+tr("Audio Sources"));

  //
  // Use StereoTool
  //
  gui_use_stereotool_check=new QCheckBox(this);
  connect(gui_use_stereotool_check,SIGNAL(toggled(bool)),
	  this,SLOT(stereotoolToggledData(bool)));
  gui_use_stereotool_label=new QLabel(tr("Use StereoTool Processor"),this);
  gui_use_stereotool_label->setFont(label_font);
  gui_use_stereotool_label->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);

  //
  // StereoTool Key
  //
  gui_stereotool_key_label=
    new QLabel(tr("StereoTool Registration Key")+":",this);
  gui_stereotool_key_label->setFont(label_font);
  gui_stereotool_key_label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
  gui_stereotool_key_edit=new QLineEdit(this);

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
  gui_asihpi_widget=new HpiWidget(this);
  gui_asihpi_widget->hide();

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
  // Close Button
  //
  gui_close_button=new QPushButton(tr("Close"),this);
  gui_close_button->setFont(label_font);
  connect(gui_close_button,SIGNAL(clicked()),this,SLOT(hide()));
}


QSize SourceDialog::sizeHint() const
{
  QSize ret=QSize(500,150);

  switch((AudioDevice::DeviceType)
   gui_source_type_box->itemData(gui_source_type_box->currentIndex()).toInt()) {
    break;

  case AudioDevice::AsiHpi:
    ret=QSize(500,320);
    break;

  case AudioDevice::Alsa:
  case AudioDevice::File:
  case AudioDevice::Jack:
    ret=QSize(500,220);
    break;

  case AudioDevice::LastType:
    break;
  }
  return ret;
}


bool SourceDialog::makeArgs(QStringList *args,bool escape_args)
{
  QString quote="";
  if(escape_args) {
    quote="\"";
  }

  if(gui_use_stereotool_check->isChecked()) {
    args->push_back("--stereotool-enable");
  }
  args->push_back("--stereotool-key="+gui_stereotool_key_edit->text());

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
#ifdef ASIHPI
    if((gui_asihpi_widget->selectedAdapterIndex()==0)||
       (gui_asihpi_widget->selectedInputIndex()==0)) {
      return false;
    }
    args->push_back("--asihpi-adapter-index="+
	  QString().sprintf("%u",gui_asihpi_widget->selectedAdapterIndex()));
    args->push_back("--asihpi-input-index="+
	  QString().sprintf("%u",gui_asihpi_widget->selectedInputIndex()));
    args->push_back("--asihpi-input-gain="+
	  QString().sprintf("%d",gui_asihpi_widget->inputGain()/100));
    args->push_back("--asihpi-input-source="+
		    AsihpiSourceName(gui_asihpi_widget->inputSource()));
    if(!AsihpiSourceName(gui_asihpi_widget->inputType()).isEmpty()) {
      args->push_back("--asihpi-input-type="+
		      AsihpiSourceName(gui_asihpi_widget->inputType()));
    }
    switch(gui_asihpi_widget->channelMode()) {
    case HPI_CHANNEL_MODE_NORMAL:
      args->push_back("--asihpi-channel-mode=NORMAL");
      break;

    case HPI_CHANNEL_MODE_SWAP:
      args->push_back("--asihpi-channel-mode=SWAP");
      break;

    case HPI_CHANNEL_MODE_LEFT_TO_STEREO:
      args->push_back("--asihpi-channel-mode=LEFT");
      break;

    case HPI_CHANNEL_MODE_RIGHT_TO_STEREO:
      args->push_back("--asihpi-channel-mode=RIGHT");
      break;
    }
#endif  // ASIHPI
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


void SourceDialog::setControlsLocked(bool state)
{
  gui_stereotool_key_edit->setReadOnly(state);

  gui_source_type_box->setReadOnly(state);

  gui_jack_server_name_edit->setReadOnly(state);
  gui_jack_client_name_edit->setReadOnly(state);

  gui_file_name_edit->setReadOnly(state);
  gui_file_select_button->setDisabled(state);

  gui_asihpi_widget->setReadOnly(state);

  gui_alsa_device_edit->setReadOnly(state);
}


void SourceDialog::addSourceTypes(const QString &types)
{
  QStringList f0;

  f0=QString(types).split("\n");
  for(int i=0;i<f0.size();i++) {
    for(int j=0;j<AudioDevice::LastType;j++) {
      if(f0[i]==AudioDevice::optionKeyword((AudioDevice::DeviceType)j)) {
	gui_source_type_box->
	  insertItem(-1,AudioDevice::deviceTypeText((AudioDevice::DeviceType)j),
		     j);
      }
    }
  }
}


void SourceDialog::load(Profile *p)
{
  gui_use_stereotool_check->
    setChecked(p->intValue("GlassGui","StereotoolEnable"));
  gui_stereotool_key_edit->
    setText(p->stringValue("GlassGui","StereotoolKey"));
  stereotoolToggledData(gui_use_stereotool_check->isChecked());

  gui_source_type_box->
    setCurrentItemData(AudioDevice::deviceType(p->stringValue("GlassGui",
							      "AudioDevice")));
  sourceTypeChanged(gui_source_type_box->currentIndex());

#ifdef ALSA
  gui_alsa_device_edit->setText(p->stringValue("GlassGui","AlsaDevice"));
#endif  // ALSA

#ifdef ASIHPI
  gui_asihpi_widget->setSelected(p->intValue("GlassGui","AsihpiAdapterIndex"),
			       p->intValue("GlassGui","AsihpiInputIndex"));
  gui_asihpi_widget->
    setInputGain(100*p->intValue("GlassGui","AsihpiInputGain"));
  gui_asihpi_widget->setChannelMode(p->intValue("GlassGui","AsihpiChannelMode",
						HPI_CHANNEL_MODE_NORMAL));
  gui_asihpi_widget->setInputSource(p->intValue("GlassGui","AsihpiInputSource",
						HPI_SOURCENODE_LINEIN));
  gui_asihpi_widget->setInputType(p->intValue("GlassGui","AsihpiInputType",
						HPI_SOURCENODE_LINEIN));
#endif  // ASIHPI

#ifdef SNDFILE
  gui_file_name_edit->setText(p->stringValue("GlassGui","FileName"));
#endif  // SNDFILE

#ifdef JACK
  gui_jack_server_name_edit->
    setText(p->stringValue("GlassGui","JackServerName"));
  gui_jack_client_name_edit->
    setText(p->stringValue("GlassGui","JackClientName"));
#endif  // JACK
}


void SourceDialog::save(FILE *f)
{
  fprintf(f,"StereotoolEnable=%d\n",gui_use_stereotool_check->isChecked());
  fprintf(f,"StereotoolKey=%s\n",
	  (const char *)gui_stereotool_key_edit->text().toUtf8());
  fprintf(f,"AudioDevice=%s\n",
	  (const char *)AudioDevice::optionKeyword((AudioDevice::DeviceType)
		    gui_source_type_box->currentItemData().toInt()).toUtf8());
  fprintf(f,"AlsaDevice=%s\n",
	  (const char *)gui_alsa_device_edit->text().toUtf8());

  fprintf(f,"AsihpiAdapterIndex=%u\n",
	  gui_asihpi_widget->selectedAdapterIndex());
  fprintf(f,"AsihpiInputIndex=%u\n",gui_asihpi_widget->selectedInputIndex());
  fprintf(f,"AsihpiInputGain=%d\n",gui_asihpi_widget->inputGain()/100);
  fprintf(f,"AsihpiChannelMode=%u\n",gui_asihpi_widget->channelMode());
  fprintf(f,"AsihpiInputSource=%u\n",gui_asihpi_widget->inputSource());
  fprintf(f,"AsihpiInputType=%u\n",gui_asihpi_widget->inputType());

  fprintf(f,"FileName=%s\n",
	  (const char *)gui_file_name_edit->text().toUtf8());

  fprintf(f,"JackServerName=%s\n",
	  (const char *)gui_jack_server_name_edit->text().toUtf8());
  fprintf(f,"JackClientName=%s\n",
	  (const char *)gui_jack_client_name_edit->text().toUtf8());
}


void SourceDialog::show()
{
  ChangeSize();
  QDialog::show();
}


void SourceDialog::resizeEvent(QResizeEvent *e)
{
  int ypos=10;

  //
  // StereoTool Controls
  //
  gui_use_stereotool_check->setGeometry(10,ypos,20,20);
  gui_use_stereotool_label->setGeometry(35,ypos,300,20);
  ypos+=22;

  gui_stereotool_key_label->setGeometry(15,ypos,200,24);
  gui_stereotool_key_edit->setGeometry(220,ypos,size().width()-235,24);
  ypos+=33;

  int ypos_base=ypos;

  //
  // Device Type
  //
  gui_source_type_label->setGeometry(10,ypos,60,20);
  gui_source_type_box->setGeometry(75,ypos,350,24);
  ypos+=26;

  ypos_base=ypos;

  //
  // ALSA Controls
  //
  ypos=ypos_base;
  gui_alsa_device_label->setGeometry(70,ypos,110,24);
  gui_alsa_device_edit->setGeometry(185,ypos,100,24);
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
  gui_asihpi_widget->setGeometry(75,ypos,400,220);

  //
  // JACK Controls
  //
  ypos=ypos_base;
  gui_jack_server_name_label->setGeometry(75,ypos,145,20);
  gui_jack_server_name_edit->setGeometry(225,ypos,size().width()-260,24);
  ypos+=26;
  gui_jack_client_name_label->setGeometry(75,ypos,145,20);
  gui_jack_client_name_edit->setGeometry(225,ypos,size().width()-260,24);
  ypos+=26;

  gui_close_button->setGeometry(size().width()-80,size().height()-50,70,40);
}


void SourceDialog::stereotoolToggledData(bool state)
{
  gui_stereotool_key_label->setEnabled(state);
  gui_stereotool_key_edit->setEnabled(state);
}


void SourceDialog::sourceTypeChanged(int n)
{
  gui_alsa_device_label->hide();
  gui_alsa_device_edit->hide();

  gui_file_select_button->hide();
  gui_file_name_label->hide();
  gui_file_name_edit->hide();

  gui_asihpi_widget->hide();

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
    gui_asihpi_widget->show();
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
  ChangeSize();
}


void SourceDialog::fileSelectName()
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


void SourceDialog::checkArgs(const QString &str)
{
  emit updated();
}


void SourceDialog::ChangeSize()
{
  setMinimumHeight(sizeHint().height());
  setMaximumHeight(sizeHint().height());
  setMinimumWidth(sizeHint().width());
}
