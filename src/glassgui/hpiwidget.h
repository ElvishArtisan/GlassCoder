// hpiwidget.h
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

#ifndef HPIWIDGET_H
#define HPIWIDGET_H

#include <stdint.h>
#include <syslog.h>

#ifdef ASIHPI
#include <asihpi/hpi.h>
#endif  // ASIHPI

#include <QLabel>
#include <QSlider>

#include "combobox.h"
#include "hpiinputlistview.h"

#define ASIHPI_PACK_CONTROL(type,index) ((0xFFFF&(type))<<16)|(0xFFFF&(index))
#define ASIHPI_UNPACK_TYPE(value) (((value)>>16)&0xFFFF)
#define ASIHPI_UNPACK_INDEX(value) ((value)&0xFFFF)

class HpiWidget : public QWidget
{
 Q_OBJECT;
 public:
  HpiWidget(QWidget *parent=0);
  void setReadOnly(bool state);
  unsigned selectedAdapterIndex() const;
  unsigned selectedInputIndex() const;
  void setSelected(unsigned adapter,unsigned input);
  int inputGain() const;
  void setInputGain(int gain);
  unsigned channelMode() const;
  void setChannelMode(unsigned mode);
  unsigned inputSource() const;
  void setInputSource(unsigned src);
  unsigned inputType() const;
  void setInputType(unsigned type);
  static QString sourceNodeText(uint16_t src_node);
  static QString channelModeText(uint16_t mode);

 protected:
  void resizeEvent(QResizeEvent *e);

 private slots:
  void listClickedData(const QModelIndex &index);
  void sourceActivatedData(int n);
  void typeActivatedData(int n);
  void modeActivatedData(int n);
  void volumeChangedData(int level);

 private:
#ifdef ASIHPI
  void LoadMixer(unsigned adapter,unsigned input);
  void Redraw();
  hpi_err_t HpiLog(hpi_err_t err,int priority=LOG_DEBUG) const;
  const char *hpi_strerror(hpi_err_t err) const;
  HpiInputListView *hpi_view;
  QLabel *hpi_source_label;
  ComboBox *hpi_source_box;
  QLabel *hpi_mode_label;
  ComboBox *hpi_mode_box;
  QLabel *hpi_type_label;
  ComboBox *hpi_type_box;
  QLabel *hpi_volume_label;
  QSlider *hpi_volume_slider;
  QLabel *hpi_volume_readout_label;
  unsigned hpi_adapter_index;
  unsigned hpi_input_index;
  uint32_t hpi_mixer_handle;
  hpi_handle_t hpi_mult_handle;
  bool hpi_mult_found;
  hpi_handle_t hpi_type_handle;
  bool hpi_type_found;
  hpi_handle_t hpi_mode_handle;
  bool hpi_mode_found;
  hpi_handle_t hpi_volume_handle;
  bool hpi_volume_found;
#endif  // ASIHPI
};


#endif  // HPIWIDGET_H
