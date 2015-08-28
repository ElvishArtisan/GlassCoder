// hpiinputlistview.cpp
//
// List widget for selecting HPI input streams
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

#include <stdint.h>
#include <stdio.h>

#ifdef ASIHPI
#include <asihpi/hpi.h>
#endif  // ASIHPI

#include "hpiinputlistview.h"

HpiInputListViewModel::HpiInputListViewModel(QObject *parent)
  : QAbstractListModel(parent)
{
#ifdef ASIHPI
  uint32_t index;
  uint16_t type;
  uint16_t inputs;
  uint16_t outputs;
  uint16_t version;
  uint32_t serial;
  int num=0;

  while(HPI_SubSysGetAdapter(NULL,num,&index,&type)==0) {
    if(HPI_AdapterGetInfo(NULL,index,&outputs,&inputs,&version,&serial,&type)==0) {
      for(uint16_t i=0;i<inputs;i++) {
	hpi_input_names.
	  push_back(QString().sprintf("ASI%04X[%u] - ",type,index+1)+
		    tr("Input Stream")+QString().sprintf(" %u",i+1));
	hpi_adapter_indices.push_back(index+1);
	hpi_input_indices.push_back(i+1);
      }
    }

    num++;
  }
#endif  // ASIHPI
}


int HpiInputListViewModel::rowCount(const QModelIndex &parent) const
{
  return hpi_input_names.size();
}


QVariant HpiInputListViewModel::data(const QModelIndex &index,int role) const
{
  switch(role) {
  case Qt::DisplayRole:
    return hpi_input_names[index.row()];

  case HpiInputListViewModel::AdapterRole:
    return hpi_adapter_indices[index.row()];

  case HpiInputListViewModel::InputRole:
    return hpi_input_indices[index.row()];
  }
  return QVariant();
}


HpiInputListView::HpiInputListView(QWidget *parent)
  : QListView(parent)
{
  hpi_read_only=false;
  hpi_model=new HpiInputListViewModel(parent);
  setModel(hpi_model);
}


void HpiInputListView::setReadOnly(bool state)
{
  hpi_read_only=state;
}


unsigned HpiInputListView::selectedAdapterIndex() const
{
  if(!currentIndex().isValid()) {
    return 0;
  }
  return hpi_model->data(currentIndex(),
			 HpiInputListViewModel::AdapterRole).toUInt();
}


unsigned HpiInputListView::selectedInputIndex() const
{
  if(!currentIndex().isValid()) {
    return 0;
  }
  return hpi_model->data(currentIndex(),
			 HpiInputListViewModel::InputRole).toUInt();
}


void HpiInputListView::setSelected(unsigned adapter,unsigned input)
{
  for(int i=0;i<model()->rowCount();i++) {
    QModelIndex index=model()->index(i,0);
    if((model()->data(index,HpiInputListViewModel::AdapterRole)==adapter)&&
       (model()->data(index,HpiInputListViewModel::InputRole)==input)) {
      setCurrentIndex(index);
    }
  }
}


void HpiInputListView::keyPressEvent(QKeyEvent *e)
{
  if(hpi_read_only) {
    e->accept();
  }
  else {
    QListView::keyPressEvent(e);
  }
}


void HpiInputListView::mouseMoveEvent(QMouseEvent *e)
{
  if(hpi_read_only) {
    e->accept();
  }
  else {
    QListView::mouseMoveEvent(e);
  }
}


void HpiInputListView::mousePressEvent(QMouseEvent *e)
{
  if(hpi_read_only) {
    e->accept();
  }
  else {
    QListView::mousePressEvent(e);
  }
}
