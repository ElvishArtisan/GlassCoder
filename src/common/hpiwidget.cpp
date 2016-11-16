// hpiwidget.cpp
//
// Widget for configuring ASIHPI sources 
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

#include <stdio.h>

#include "hpiwidget.h"

HpiWidget::HpiWidget(QWidget *parent)
  : QWidget(parent)
{
#ifdef ASIHPI
  hpi_adapter_index=0;
  hpi_input_index=0;
  hpi_mixer_handle=0;

  //
  // Fonts
  //
  QFont label_font("helvetica",14,QFont::Bold);
  label_font.setPixelSize(14);
  QFont readout_font("helvetica",14,QFont::Normal);
  readout_font.setPixelSize(14);

  //
  // Source
  //
  hpi_source_label=new QLabel(tr("Input Source")+":",this);
  hpi_source_label->setFont(label_font);
  hpi_source_label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
  hpi_source_box=new ComboBox(this);
  connect(hpi_source_box,SIGNAL(activated(int)),
	  this,SLOT(sourceActivatedData(int)));

  //
  // Input Type
  //
  hpi_type_label=new QLabel(tr("Input Type")+":",this);
  hpi_type_label->setFont(label_font);
  hpi_type_label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
  hpi_type_box=new ComboBox(this);
  connect(hpi_type_box,SIGNAL(activated(int)),
	  this,SLOT(typeActivatedData(int)));

  //
  // Input Mode
  //
  hpi_mode_label=new QLabel(tr("Input Mode")+":",this);
  hpi_mode_label->setFont(label_font);
  hpi_mode_label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
  hpi_mode_box=new ComboBox(this);
  connect(hpi_mode_box,SIGNAL(activated(int)),
	  this,SLOT(modeActivatedData(int)));

  //
  // Input Gain
  //
  hpi_volume_label=new QLabel(tr("Input Gain")+":",this);
  hpi_volume_label->setFont(label_font);
  hpi_volume_label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
  hpi_volume_slider=new QSlider(Qt::Horizontal,this);
  connect(hpi_volume_slider,SIGNAL(valueChanged(int)),
	  this,SLOT(volumeChangedData(int)));
  hpi_volume_readout_label=new QLabel(this);
  hpi_volume_readout_label->setFont(readout_font);
  hpi_volume_readout_label->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);

  hpi_view=new HpiInputListView(this);
  connect(hpi_view,SIGNAL(clicked(const QModelIndex &)),
	  this,SLOT(listClickedData(const QModelIndex &)));
#endif  // ASIHPI
}


void HpiWidget::setReadOnly(bool state)
{
#ifdef ASIHPI
  hpi_view->setReadOnly(state);
#endif  // ASIHPI
}


unsigned HpiWidget::selectedAdapterIndex() const
{
#ifdef ASIHPI
  return hpi_view->selectedAdapterIndex();
#else
  return 0;
#endif  // ASIHPI
}


unsigned HpiWidget::selectedInputIndex() const
{
#ifdef ASIHPI
  return hpi_view->selectedInputIndex();
#else
  return 0;
#endif  // ASIHPI
}


void HpiWidget::setSelected(unsigned adapter,unsigned input)
{
#ifdef ASIHPI
  hpi_view->setSelected(adapter,input);
  listClickedData(QModelIndex());
#endif  // ASIHPI
}


int HpiWidget::inputGain() const
{
#ifdef ASIHPI
  return hpi_volume_slider->value();
#else
  return 0;
#endif  // ASIHPI
}


void HpiWidget::setInputGain(int gain)
{
#ifdef ASIHPI
  hpi_volume_slider->setValue(gain);
  if(hpi_volume_found) {
    volumeChangedData(gain);
  }
#endif  // ASIHPI
}


unsigned HpiWidget::channelMode() const
{
#ifdef ASIHPI
  return hpi_mode_box->currentItemData().toUInt();
#else
  return 0;
#endif  // ASIHPI
}


void HpiWidget::setChannelMode(unsigned mode)
{
#ifdef ASIHPI
  if(hpi_mode_box->setCurrentItemData(mode)) {
    modeActivatedData(0);
  }
#endif  // ASIHPI
}


unsigned HpiWidget::inputSource() const
{
#ifdef ASIHPI
  return ASIHPI_UNPACK_TYPE(hpi_source_box->currentItemData().toUInt());
#else
  return 0;
#endif  // ASIHPI
}


void HpiWidget::setInputSource(unsigned src)
{
#ifdef ASIHPI
  if(hpi_source_box->
     setCurrentItemData(ASIHPI_PACK_CONTROL(hpi_input_index,src))) {
    sourceActivatedData(0);
  }
#endif  // ASIHPI
}


unsigned HpiWidget::inputType() const
{
#ifdef ASIHPI
  return ASIHPI_UNPACK_TYPE(hpi_type_box->currentItemData().toUInt());
#else
  return 0;
#endif  // ASIHPI
}


void HpiWidget::setInputType(unsigned type)
{
#ifdef ASIHPI
  if(hpi_type_box->
     setCurrentItemData(ASIHPI_PACK_CONTROL(hpi_input_index,type))) {
    typeActivatedData(0);
  }
#endif  // ASIHPI
}


QString HpiWidget::sourceNodeText(uint16_t src_node)
{
  //
  // From HPI v4.16
  //
  QString ret=tr("Unknown");
  switch(src_node) {
#ifdef ASIHPI
  case HPI_SOURCENODE_NONE:
    ret=tr("None");
    break;

  case HPI_SOURCENODE_OSTREAM:
    ret=tr("Output Stream");
    break;

  case HPI_SOURCENODE_LINEIN:
    ret=tr("Line In");
    break;

  case HPI_SOURCENODE_AESEBU_IN:
    ret=tr("AES3 Input");
    break;

  case HPI_SOURCENODE_TUNER:
    ret=tr("Tuner");
    break;

  case HPI_SOURCENODE_RF:
    ret=tr("RF Input");
    break;

  case HPI_SOURCENODE_CLOCK_SOURCE:
    ret=tr("Clock source");
    break;

  case HPI_SOURCENODE_RAW_BITSTREAM:
    ret=tr("Raw bitstream");
    break;

  case HPI_SOURCENODE_MICROPHONE:
    ret=tr("Microphone");
    break;

  case HPI_SOURCENODE_COBRANET:
    ret=tr("CobraNet input");
    break;

  case HPI_SOURCENODE_ANALOG:
    ret=tr("Analog input");
    break;

  case HPI_SOURCENODE_ADAPTER:
    ret=tr("Adapter");
    break;

  case HPI_SOURCENODE_RTP_DESTINATION:
    ret=tr("RTP stream input");
    break;

  case HPI_SOURCENODE_INTERNAL:
    ret=tr("Internal");
    break;

    /*
  case HPI_SOURCENODE_BLULINK:
    ret=tr("BLU-Link input");
    break;
  case HPI_SOURCENODE_AVB_INPUT:
    ret=tr("AVB input");
    break;

  case HPI_SOURCENODE_AVB_STREAM:
    ret=tr("AVB audio");
    break;
    */
#endif  // ASIHPI
  default:
    ret=tr("Unknown")+QString().sprintf(" [%u]",src_node);
    break;
  }

  return ret;
}


QString HpiWidget::channelModeText(uint16_t mode)
{
  //
  // From HPI v4.16
  //
  QString ret=tr("Unknown");

  switch(mode) {
#ifdef ASIHPI
  case HPI_CHANNEL_MODE_NORMAL:
    ret=tr("Normal");
    break;

  case HPI_CHANNEL_MODE_SWAP:
    ret=tr("Swapped");
    break;

  case HPI_CHANNEL_MODE_LEFT_TO_STEREO:
    ret=tr("Left Only");
    break;

  case HPI_CHANNEL_MODE_RIGHT_TO_STEREO:
    ret=tr("Right Only");
    break;

  case HPI_CHANNEL_MODE_STEREO_TO_LEFT:
    ret=tr("Left Only");
    break;

  case HPI_CHANNEL_MODE_STEREO_TO_RIGHT:
    ret=tr("Right Only");
    break;
#endif  // ASIHPI
  default:
    ret=tr("Unknown mode")+QString().sprintf(" [%u]",mode);
    break;
  }

  return ret;
}


void HpiWidget::resizeEvent(QResizeEvent *e)
{
#ifdef ASIHPI
  Redraw();

  int ypos=0;

  hpi_view->setGeometry(0,0,size().width(),100);
  ypos+=105;

  if(hpi_source_box->isVisible()) {
    hpi_source_label->setGeometry(0,ypos,100,25);
    hpi_source_box->setGeometry(105,ypos,size().width()-115,25);
    ypos+=30;
  }

  if(hpi_mode_box->isVisible()) {
    hpi_mode_label->setGeometry(0,ypos,100,25);
    hpi_mode_box->setGeometry(105,ypos,size().width()-115,25);
    ypos+=30;
  }

#endif  // ASIHPI
}


void HpiWidget::listClickedData(const QModelIndex &index)
{
#ifdef ASIHPI
  LoadMixer(hpi_view->selectedAdapterIndex(),hpi_view->selectedInputIndex());
  hpi_adapter_index=hpi_view->selectedAdapterIndex();
  hpi_input_index=hpi_view->selectedInputIndex();
#endif  // ASIHPI
}


void HpiWidget::sourceActivatedData(int n)
{
#ifdef ASIHPI
  uint16_t type=ASIHPI_UNPACK_TYPE(hpi_source_box->currentItemData().toUInt());
  uint16_t index=
    ASIHPI_UNPACK_INDEX(hpi_source_box->currentItemData().toUInt());

  HpiLog(HPI_Multiplexer_SetSource(NULL,hpi_mult_handle,type,index));
#endif  // ASIHPI
}


void HpiWidget::typeActivatedData(int n)
{
#ifdef ASIHPI
  uint16_t type=ASIHPI_UNPACK_TYPE(hpi_type_box->currentItemData().toUInt());
  uint16_t index=
    ASIHPI_UNPACK_INDEX(hpi_type_box->currentItemData().toUInt());

  HpiLog(HPI_Multiplexer_SetSource(NULL,hpi_type_handle,type,index));
#endif  // ASIHPI
}


void HpiWidget::modeActivatedData(int n)
{
#ifdef ASIHPI
  uint16_t mode=hpi_mode_box->currentItemData().toUInt();

  HpiLog(HPI_ChannelModeSet(NULL,hpi_mode_handle,mode));
#endif  // ASIHPI
}


void HpiWidget::volumeChangedData(int level)
{
#ifdef ASIHPI
  short lvls[HPI_MAX_CHANNELS];

  for(unsigned i=0;i<HPI_MAX_CHANNELS;i++) {
    lvls[i]=level;
  }

  if(level<=-10000) {
    hpi_volume_readout_label->setText(tr("OFF"));
  }
  else {
    hpi_volume_readout_label->setText(QString().sprintf("%d dB",level/100));
  }
  HpiLog(HPI_VolumeSetGain(NULL,hpi_volume_handle,lvls));
#endif  // ASIHPI
}


#ifdef ASIHPI
void HpiWidget::LoadMixer(unsigned adapter,unsigned input)
{
  uint16_t index=0;
  uint16_t type=0;
  uint16_t mindex=0;
  short min=0;
  short max;
  short step=0;
  short gain=0;
  hpi_mode_found=false;
  hpi_mult_found=false;
  hpi_type_found=false;
  hpi_volume_found=false;

  if((hpi_adapter_index>0)&&(hpi_input_index>0)) {
    HpiLog(HPI_MixerClose(NULL,hpi_mixer_handle));
  }

  if((adapter>0)&&(input>0)) {
    if((HpiLog(HPI_MixerOpen(NULL,adapter-1,&hpi_mixer_handle)))==0) {

      //
      // Input Source Multiplexer
      //
      hpi_source_box->clear();	
      if(HPI_MixerGetControl(NULL,hpi_mixer_handle,0,0,
			     HPI_DESTNODE_ISTREAM,input-1,
			     HPI_CONTROL_MULTIPLEXER,
			     &hpi_mult_handle)==0) {
	hpi_mult_found=true;
	index=0;
	while(HPI_Multiplexer_QuerySource(NULL,hpi_mult_handle,index,&type,
					  &mindex)==0) {
	  hpi_source_box->insertItem(index,HpiWidget::sourceNodeText(type)+
				     QString().sprintf(" %u",mindex+1),
				     ASIHPI_PACK_CONTROL(type,mindex));
	  index++;
	}
	if(HpiLog(HPI_Multiplexer_GetSource(NULL,hpi_mult_handle,&type,
					    &mindex))==0) {
	  hpi_source_box->setCurrentItemData(ASIHPI_PACK_CONTROL(type,mindex));
	}
      }

      //
      // Input Type Multiplexer
      //
      if(HPI_MixerGetControl(NULL,hpi_mixer_handle,
			     HPI_SOURCENODE_LINEIN,input-1,
			     HPI_DESTNODE_NONE,0,
			     HPI_CONTROL_MULTIPLEXER,
			     &hpi_type_handle)==0) {
	hpi_type_found=true;
	index=0;
	while(HPI_Multiplexer_QuerySource(NULL,hpi_type_handle,index,&type,
					  &mindex)==0) {
	  hpi_type_box->insertItem(index,HpiWidget::sourceNodeText(type),
				   ASIHPI_PACK_CONTROL(type,mindex));
	  index++;
	}
	if(HpiLog(HPI_Multiplexer_GetSource(NULL,hpi_type_handle,&type,
					    &mindex))==0) {
	  hpi_type_box->setCurrentItemData(ASIHPI_PACK_CONTROL(type,mindex));
	}
      }

      //
      // Mode Control
      //
      hpi_mode_box->clear();
      if(HPI_MixerGetControl(NULL,hpi_mixer_handle,0,0,
			     HPI_DESTNODE_ISTREAM,input-1,
			     HPI_CONTROL_CHANNEL_MODE,
			     &hpi_mode_handle)==0) {
	hpi_mode_found=true;
	index=0;
	while(HPI_ChannelMode_QueryMode(NULL,hpi_mode_handle,index,&type)==0) {
	  hpi_mode_box->insertItem(index,HpiWidget::channelModeText(type),type);
	  index++;
	}
	if(HpiLog(HPI_ChannelModeGet(NULL,hpi_mode_handle,&type))==0) {
	  hpi_mode_box->setCurrentItemData(type);
	}
      }

      //
      // Volume Control
      //
      if(HPI_MixerGetControl(NULL,hpi_mixer_handle,
			     HPI_SOURCENODE_LINEIN,input-1,
			     HPI_DESTNODE_NONE,0,
			     HPI_CONTROL_VOLUME,
			     &hpi_volume_handle)==0) {
	if(HpiLog(HPI_VolumeQueryRange(NULL,hpi_volume_handle,
				       &min,&max,&step))==0) {
	  hpi_volume_slider->setRange(min,max);
	  hpi_volume_found=true;
	}
	if(HpiLog(HPI_VolumeGetGain(NULL,hpi_volume_handle,&gain))==0) {
	  hpi_volume_slider->setValue(gain);
	  volumeChangedData(gain);
	}
      }

      /*
      for(uint16_t i=HPI_SOURCENODE_NONE;i<HPI_SOURCENODE_LAST_INDEX;i++) {
	for(uint16_t j=HPI_DESTNODE_NONE;j<HPI_DESTNODE_LAST_INDEX;j++) {
	  if(HPI_MixerGetControl(NULL,hpi_mixer_handle,i,0,
				 j,input-1,
				 HPI_CONTROL_MULTIPLEXER,
				 &hpi_volume_handle)==0) {
	    printf("Found: i: %u  j: %u\n",i,j);
	  }
	}
      }
      */
    }
  }
  Redraw();
}


void HpiWidget::Redraw()
{
  int ypos=0;

  hpi_view->setGeometry(0,0,size().width(),100);
  ypos+=105;

  if(hpi_mult_found) {
    hpi_source_label->show();
    hpi_source_label->setGeometry(0,ypos,100,25);
    hpi_source_box->show();
    hpi_source_box->setGeometry(105,ypos,size().width()-115,25);
    ypos+=30;
  }
  else {
    hpi_source_label->hide();
    hpi_source_box->hide();
  }

  if(hpi_type_found) {
    hpi_type_label->show();
    hpi_type_label->setGeometry(0,ypos,100,25);
    hpi_type_box->show();
    hpi_type_box->setGeometry(105,ypos,size().width()-115,25);
    ypos+=30;
  }
  else {
    hpi_type_label->hide();
    hpi_type_box->hide();
  }

  if(hpi_mode_found) {
    hpi_mode_label->setGeometry(0,ypos,100,25);
    hpi_mode_label->show();
    hpi_mode_box->setGeometry(105,ypos,size().width()-115,25);
    hpi_mode_box->show();
    ypos+=30;
  }
  else {
    hpi_mode_label->hide();
    hpi_mode_box->hide();
  }

  if(hpi_volume_found) {
    hpi_volume_label->show();
    hpi_volume_label->setGeometry(0,ypos,100,25);
    hpi_volume_slider->show();
    hpi_volume_slider->setGeometry(105,ypos,size().width()-175,25);
    hpi_volume_readout_label->show();
    hpi_volume_readout_label->setGeometry(size().width()-55,ypos,45,25);
    ypos+=30;
  }
  else {
    hpi_volume_label->hide();
    hpi_volume_slider->hide();
    hpi_volume_readout_label->hide();
  }
}


hpi_err_t HpiWidget::HpiLog(hpi_err_t err,int priority) const
{
  if(err!=0) {
    fprintf(stderr,"HPI error %d: \"%s\"\n",err,hpi_strerror(err));
  }
  return err;
}


const char *HpiWidget::hpi_strerror(hpi_err_t err) const
{
  static char err_text[200];

  HPI_GetErrorText(err,err_text);
  return err_text;
}
#endif  // ASIHPI
