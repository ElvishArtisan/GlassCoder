// metaevent.h
//
// Container class for metadata updates.
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

#ifndef METAEVENT_H
#define METAEVENT_H

#include <stdint.h>

#include <QString>
#include <QVariant>

class MetaEvent
{
 public:
  enum Field {Name=0,Description=1,Genre=2,Url=3,Irc=4,Aim=5,Icq=6,
	      Public=7,StreamTitle=8,StreamUrl=9,LastField=10};
  MetaEvent();
  MetaEvent(const MetaEvent &e);
  QVariant field(Field f) const;
  void setField(Field f,const QVariant v);
  bool isChanged(Field f) const;
  bool isChanged() const;
  void processed();
  static QString fieldText(Field f);

 private:
  QVariant meta_fields[MetaEvent::LastField];
  bool meta_changed[MetaEvent::LastField];
};


#endif  // METAEVENT_H
