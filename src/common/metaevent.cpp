// metaevent.cpp
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

#include <QObject>

#include "metaevent.h"

MetaEvent::MetaEvent()
{
}


MetaEvent::MetaEvent(const MetaEvent &e)
{
  meta_fields=e.meta_fields;
}


QStringList MetaEvent::fieldKeys() const
{
  QStringList keys;

  for(QMap<QString,QString>::const_iterator it=meta_fields.begin();
      it!=meta_fields.end();it++) {
    keys.push_back(it.key());
  }

  return keys;
}


QString MetaEvent::field(const QString &key,bool *ok) const
{
  if(ok!=NULL) {
    *ok=meta_fields.find(key)!=meta_fields.end();
  }

  return meta_fields.value(key,QString());
}


void MetaEvent::setField(const QString &key,const QString &v)
{
  QString mkey=key;
  QString mv=v;
  if(key=="TXXX") {
    int index=v.indexOf("]");
    if(index>=2) {
      QString subkey=v.mid(1,v.indexOf("]")-1);
      mkey=key+subkey;
      mv=v.right(v.length()-index-2);
    }
  }
  meta_fields[mkey]=mv;
}


QString MetaEvent::exportFields() const
{
  QString ret="";

  for(QMap<QString,QString>::const_iterator it=meta_fields.begin();
      it!=meta_fields.end();it++) {
    QString value=it.value();
    ret+="Metadata|"+it.key()+":"+value+"\n";
  }

  return ret;
}


bool MetaEvent::isEmpty() const
{
  return meta_fields.size()==0;
}


QString MetaEvent::dump() const
{
  QString str="";

  for(QMap<QString,QString>::const_iterator it=meta_fields.begin();
      it!=meta_fields.end();it++) {
    str+=it.key()+": "+it.value()+"\n";
  }

  return str;
}


void MetaEvent::clear()
{
  meta_fields.clear();
}

