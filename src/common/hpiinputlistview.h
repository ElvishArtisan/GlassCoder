// hpiinputlistview.h
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

#ifndef HPIINPUTLISTVIEW_H
#define HPIINPUTLISTVIEW_H

#include <vector>

#include <QAbstractListModel>
#include <QKeyEvent>
#include <QListView>

class HpiInputListViewModel : public QAbstractListModel
{
 public:
  enum UserRole {AdapterRole=Qt::UserRole,InputRole=Qt::UserRole+1};
  HpiInputListViewModel(QObject *parent=0);
  int rowCount(const QModelIndex &parent=QModelIndex()) const;
  QVariant data(const QModelIndex &index,int role=Qt::DisplayRole) const;

 private:
  std::vector<QString> hpi_input_names;
  std::vector<unsigned> hpi_adapter_indices;
  std::vector<unsigned> hpi_input_indices;
};

class HpiInputListView : public QListView
{
  Q_OBJECT;
 public:
  HpiInputListView(QWidget *parent);
  void setReadOnly(bool state);
  unsigned selectedAdapterIndex() const;
  unsigned selectedInputIndex() const;
  void setSelected(unsigned adapter,unsigned input);

 signals:
  void inputSelected(unsigned adapter,unsigned input);

 protected:
  void keyPressEvent(QKeyEvent *e);
  void mouseMoveEvent(QMouseEvent *e);
  void mousePressEvent(QMouseEvent *e);

 private:
  HpiInputListViewModel *hpi_model;
  bool hpi_read_only;
};

#endif  // HPIINPUTLISTVIEW_H
