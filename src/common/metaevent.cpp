// metaevent.cpp
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

#include <QObject>

#include "metaevent.h"

MetaEvent::MetaEvent()
{
  for(unsigned i=0;i<MetaEvent::LastField;i++) {
    meta_changed[i]=false;
  }
}


MetaEvent::MetaEvent(const MetaEvent &e)
{
  for(unsigned i=0;i<MetaEvent::LastField;i++) {
    meta_fields[i]=e.meta_fields[i];
    meta_changed[i]=e.meta_changed[i];
  }
}


QVariant MetaEvent::field(MetaEvent::Field f) const
{
  return meta_fields[f];
}


void MetaEvent::setField(MetaEvent::Field f,const QVariant v)
{
  meta_fields[f]=v;
  meta_changed[f]=true;
}


bool MetaEvent::isChanged(MetaEvent::Field f) const
{
  return meta_changed[f];
}


bool MetaEvent::isChanged() const
{
  for(unsigned i=0;i<MetaEvent::LastField;i++) {
    if(meta_changed[i]) {
      return true;
    }
  }
  return false;
}


void MetaEvent::processed()
{
  for(unsigned i=0;i<MetaEvent::LastField;i++) {
    meta_changed[i]=false;
  }
}


QString MetaEvent::fieldText(Field f)
{
  QString ret=QObject::tr("Unknown");

  switch(f) {
  case MetaEvent::Name:
    ret=QObject::tr("Name");
    break;

  case MetaEvent::Description:
    ret=QObject::tr("Description");
    break;

  case MetaEvent::Genre:
    ret=QObject::tr("Genre");
    break;

  case MetaEvent::Url:
    ret=QObject::tr("ChannelUrl");
    break;

  case MetaEvent::Irc:
    ret=QObject::tr("Irc");
    break;

  case MetaEvent::Aim:
    ret=QObject::tr("Aim");
    break;

  case MetaEvent::Icq:
    ret=QObject::tr("Icq");
    break;

  case MetaEvent::Public:
    ret=QObject::tr("Public");
    break;

  case MetaEvent::StreamTitle:
    ret=QObject::tr("StreamTitle");
    break;

  case MetaEvent::StreamUrl:
    ret=QObject::tr("StreamUrl");
    break;

  case MetaEvent::LastField:
    break;
  }

  return ret;
}
