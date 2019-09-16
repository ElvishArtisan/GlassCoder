// metaevent.h
//
// Container class for metadata updates.
//
//   (C) Copyright 2016-2019 Fred Gleason <fredg@paravelsystems.com>
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

#ifndef METAEVENT_H
#define METAEVENT_H

#include <stdint.h>

#include <QString>
#include <QStringList>
#include <QMap>

class MetaEvent
{
 public:
  MetaEvent();
  MetaEvent(const MetaEvent &e);
  QStringList fieldKeys() const;
  QString field(const QString &key,bool *ok=NULL) const;
  void setField(const QString &key,const QString &v);
  QString exportFields() const;
  bool isEmpty() const;
  QString dump() const;
  void clear();

 private:
  QMap<QString,QString> meta_fields;
};


#endif  // METAEVENT_H
