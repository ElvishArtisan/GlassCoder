// codecdialog.cpp
//
// Configuration dialog for codec settings
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

#include "codecdialog.h"

CodecDialog::CodecDialog(const QString &caption,QWidget *parent)
  : QDialog(parent,Qt::Dialog)
{
  gui_caption=caption;

  //
  // Fonts
  //
  QFont label_font("helvetica",14,QFont::Bold);
  label_font.setPixelSize(14);

  setWindowTitle(caption+" - "+tr("Codec Settings"));

  //
  // Codec Type
  //
  gui_codec_type_label=new QLabel(tr("Type")+":",this);
  gui_codec_type_label->setFont(label_font);
  gui_codec_type_label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
  gui_codec_type_box=new ComboBox(this);
  connect(gui_codec_type_box,SIGNAL(activated(int)),
	  this,SLOT(codecTypeChanged(int)));

  //
  // Codec Samplerate
  //
  gui_codec_samplerate_label=new QLabel(tr("Sample Rate")+":",this);
  gui_codec_samplerate_label->setFont(label_font);
  gui_codec_samplerate_label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
  gui_codec_samplerate_box=new ComboBox(this);
  connect(gui_codec_samplerate_box,SIGNAL(activated(int)),
	  this,SLOT(codecSamplerateChanged(int)));

  //
  // Codec Channels
  //
  gui_codec_channels_label=new QLabel(tr("Channels")+":",this);
  gui_codec_channels_label->setFont(label_font);
  gui_codec_channels_label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
  gui_codec_channels_box=new ComboBox(this);

  //
  // Codec Bitrate
  //
  gui_codec_bitrate_label=new QLabel(tr("Bit Rate")+":",this);
  gui_codec_bitrate_label->setFont(label_font);
  gui_codec_bitrate_label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
  gui_codec_bitrate_box=new ComboBox(this);

  //
  // Close Button
  //
  gui_close_button=new QPushButton(tr("Close"),this);
  gui_close_button->setFont(label_font);
  connect(gui_close_button,SIGNAL(clicked()),this,SLOT(hide()));

  setMaximumSize(sizeHint());
  setMinimumSize(sizeHint());
}


QSize CodecDialog::sizeHint() const
{
  return QSize(400,230-52);
}


void CodecDialog::setControlsLocked(bool state)
{
  gui_codec_type_box->setReadOnly(state);
  gui_codec_samplerate_box->setReadOnly(state);
  gui_codec_channels_box->setReadOnly(state);
  gui_codec_bitrate_box->setReadOnly(state);
}


void CodecDialog::resizeEvent(QResizeEvent *e)
{
  int ypos=10;

  gui_codec_type_label->setGeometry(10,ypos,110,24);
  gui_codec_type_box->setGeometry(125,ypos,250,24);
  ypos+=26;

  gui_codec_samplerate_label->setGeometry(10,ypos,145,24);
  gui_codec_samplerate_box->setGeometry(160,ypos,200,24);
  ypos+=26;

  gui_codec_channels_label->setGeometry(10,ypos,145,24);
  gui_codec_channels_box->setGeometry(160,ypos,50,24);
  ypos+=26;

  gui_codec_bitrate_label->setGeometry(10,ypos,145,24);
  gui_codec_bitrate_box->setGeometry(160,ypos,150,24);
  ypos+=26;

  ypos+=9;

  gui_close_button->setGeometry(size().width()-80,size().height()-50,70,40);
}


void CodecDialog::codecTypeChanged(int n)
{
  Codec::Type type=(Codec::Type)gui_codec_type_box->itemData(n).toInt();

  gui_codec_samplerate_box->clear();
  gui_codec_channels_box->clear();
  gui_codec_bitrate_box->clear();

  switch(type) {
  case Codec::TypeFdk:
  case Codec::TypeMpegL2:
  case Codec::TypeMpegL3:
    gui_codec_samplerate_box->insertItem(-1,"16000 samples/sec",16000);
    gui_codec_samplerate_box->insertItem(-1,"22050 samples/sec",22050);
    gui_codec_samplerate_box->insertItem(-1,"24000 samples/sec",24000);
    gui_codec_samplerate_box->insertItem(-1,"32000 samples/sec",32000);
    gui_codec_samplerate_box->insertItem(-1,"44100 samples/sec",44100);
    gui_codec_samplerate_box->insertItem(-1,"48000 samples/sec",48000);

    gui_codec_channels_box->insertItem(-1,"1",1);
    gui_codec_channels_box->insertItem(-1,"2",2);

    gui_codec_bitrate_label->setText(tr("Bit Rate(s)")+":");
    break;

  case Codec::TypePcm16:
    gui_codec_samplerate_box->insertItem(-1,"16000 samples/sec",16000);
    gui_codec_samplerate_box->insertItem(-1,"22050 samples/sec",22050);
    gui_codec_samplerate_box->insertItem(-1,"24000 samples/sec",24000);
    gui_codec_samplerate_box->insertItem(-1,"32000 samples/sec",32000);
    gui_codec_samplerate_box->insertItem(-1,"44100 samples/sec",44100);
    gui_codec_samplerate_box->insertItem(-1,"48000 samples/sec",48000);

    gui_codec_channels_box->insertItem(-1,"1",1);
    gui_codec_channels_box->insertItem(-1,"2",2);

    gui_codec_bitrate_label->setText(tr("Bit Rate(s)")+":");
    break;

  case Codec::TypeVorbis:
    gui_codec_samplerate_box->insertItem(-1,"16000 samples/sec",16000);
    gui_codec_samplerate_box->insertItem(-1,"22050 samples/sec",22050);
    gui_codec_samplerate_box->insertItem(-1,"24000 samples/sec",24000);
    gui_codec_samplerate_box->insertItem(-1,"32000 samples/sec",32000);
    gui_codec_samplerate_box->insertItem(-1,"44100 samples/sec",44100);
    gui_codec_samplerate_box->insertItem(-1,"48000 samples/sec",48000);

    gui_codec_channels_box->insertItem(-1,"1",1);
    gui_codec_channels_box->insertItem(-1,"2",2);

    gui_codec_bitrate_label->setText(tr("Quality")+":");
    break;

  case Codec::TypeOpus:
    gui_codec_samplerate_box->insertItem(-1,"48000 samples/sec",48000);

    gui_codec_channels_box->insertItem(-1,"1",1);
    gui_codec_channels_box->insertItem(-1,"2",2);

    gui_codec_bitrate_label->setText(tr("Quality")+":");
    break;

  case Codec::TypeLast:
    break;
  }
  codecSamplerateChanged(0);
}


void CodecDialog::codecSamplerateChanged(int n)
{
  Codec::Type type=(Codec::Type)gui_codec_type_box->
    itemData(gui_codec_type_box->currentIndex()).toInt();

  gui_codec_bitrate_box->clear();
  switch(type) {
  case Codec::TypeFdk:
    gui_codec_bitrate_box->insertItem(-1,"16 kbits/sec",16);
    gui_codec_bitrate_box->insertItem(-1,"24 kbits/sec",24);
    gui_codec_bitrate_box->insertItem(-1,"32 kbits/sec",32);
    gui_codec_bitrate_box->insertItem(-1,"40 kbits/sec",40);
    gui_codec_bitrate_box->insertItem(-1,"48 kbits/sec",48);
    gui_codec_bitrate_box->insertItem(-1,"96 kbits/sec",96);
    gui_codec_bitrate_box->insertItem(-1,"128 kbits/sec",128);
    gui_codec_bitrate_box->insertItem(-1,"196 kbits/sec",196);
    gui_codec_bitrate_box->insertItem(-1,"256 kbits/sec",256);
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

  case Codec::TypeOpus:
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

  case Codec::TypePcm16:
    gui_codec_bitrate_box->insertItem(-1,tr("None",0));
    break;

  case Codec::TypeLast:
    break;
  }  
}


void CodecDialog::makeArgs(QStringList *args)
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
  case Codec::TypeFdk:
  case Codec::TypeOpus:
  case Codec::TypeLast:
    if(gui_codec_bitrate_box->isEnabled()&&
       (gui_codec_bitrate_box->currentItemData().toUInt()!=0)) {
      args->push_back("--audio-bitrate="+QString().sprintf("%u",
			   gui_codec_bitrate_box->currentItemData().toUInt()));
    }
    break;

  case Codec::TypePcm16:
    break;

  case Codec::TypeVorbis:
    args->push_back("--audio-quality="+QString().sprintf("%u",
			   gui_codec_bitrate_box->currentItemData().toUInt()));
    break;
  }
}


void CodecDialog::addCodecTypes(const QString &types)
{
  QStringList f0;

  f0=types.split("\n");
  for(int i=0;i<f0.size();i++) {
    for(int j=0;j<Codec::TypeLast;j++) {
      if(f0[i]==Codec::optionKeyword((Codec::Type)j)) {
	gui_codec_type_box->
	  insertItem(-1,Codec::codecTypeText((Codec::Type)j),j);
      }
    }
  }
}


void CodecDialog::load(Profile *p)
{
  gui_codec_type_box->
    setCurrentItemData(Codec::codecType(p->stringValue("GlassGui",
						       "AudioFormat")));
  codecTypeChanged(gui_codec_type_box->currentIndex());
  gui_codec_samplerate_box->
    setCurrentItemData(p->intValue("GlassGui","AudioSamplerate"));
  codecSamplerateChanged(gui_codec_samplerate_box->currentIndex());
  gui_codec_channels_box->
    setCurrentItemData(p->intValue("GlassGui","AudioChannels"));
  gui_codec_bitrate_box->
    setCurrentItemData(p->intValue("GlassGui","AudioBitrate1"));
}


void CodecDialog::save(FILE *f)
{
  fprintf(f,"AudioFormat=%s\n",
	  (const char *)Codec::optionKeyword((Codec::Type)
		    gui_codec_type_box->currentItemData().toInt()).toUtf8()); 
  fprintf(f,"AudioSamplerate=%u\n",
	  gui_codec_samplerate_box->currentItemData().toUInt());
  fprintf(f,"AudioChannels=%u\n",
	  gui_codec_channels_box->currentItemData().toUInt());
  fprintf(f,"AudioBitrate1=%u\n",
	  gui_codec_bitrate_box->currentItemData().toUInt());
}

