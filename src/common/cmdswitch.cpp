// cmdswitch.cpp
//
// Process Command-Line Switches
//
//   (C) Copyright 2012-2015 Fred Gleason <fredg@paravelsystems.com>
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

#include <stdlib.h>
#include <stdio.h>

#include <QCoreApplication>
#include <QStringList>

#include "cmdswitch.h"
#include "logging.h"

CmdSwitch::CmdSwitch(const char *modname,const char *usage)
{
  int l=0;
  bool handled=false;
  QStringList args=qApp->arguments();

  for(int i=1;i<args.size();i++) {
#ifndef WIN32
    if(args.at(i)=="--version") {
      printf("%s v%s\n",modname,VERSION);
      exit(0);
    }
#endif  // WIN32
    if(args.at(i)=="--help") {
      printf("\n%s %s\n",modname,usage);
      exit(0);
    }
    l=args.at(i).length();
    handled=false;
    for(int j=0;j<l;j++) {
      if(args.at(i).at(j)==QChar('=')) {
	switch_keys.push_back(args.at(i).left(j));
	switch_values.push_back(args.at(i).right(l-(j+1)));
	switch_processed.push_back(false);
	j=l;
	handled=true;
      }
    }
    if(!handled) {
      switch_keys.push_back(args.at(i));
      switch_values.push_back(QString(""));
      switch_processed.push_back(false);
    }
  }
}


unsigned CmdSwitch::keys() const
{
  return switch_keys.size();
}


QString CmdSwitch::key(unsigned n) const
{
  return switch_keys[n];
}


QString CmdSwitch::value(unsigned n) const
{
  return switch_values[n];
}


bool CmdSwitch::processed(unsigned n) const
{
  return switch_processed[n];
}


void CmdSwitch::setProcessed(unsigned n,bool state)
{
  switch_processed[n]=state;
}


bool CmdSwitch::allProcessed() const
{
  for(unsigned i=0;i<switch_processed.size();i++) {
    if(!switch_processed[i]) {
      return false;
    }
  }
  return true;
}


bool CmdSwitch::addOverlay(const QString &filename)
{
  FILE *f=NULL;
  char line[1024];

  if((f=fopen(filename.toUtf8(),"r"))==NULL) {
    return false;
  }
  while(fgets(line,1024,f)!=NULL) {
    bool unique=true;
    QStringList f0=QString(line).trimmed().split("=",QString::KeepEmptyParts);
    if((f0[0].length()>0)&&(f0[0].left(1)!="#")) {
      for(unsigned i=0;i<switch_keys.size();i++) {
	if(f0[0]==switch_keys[i]) {
	  unique=false;
	}
      }
      if(unique) {
	switch_keys.insert(switch_keys.begin(),f0[0].trimmed());
	if(f0.size()>=2) {
	  f0.erase(f0.begin());
	  switch_values.insert(switch_values.begin(),f0.join("=").trimmed());
	}
	else {
	  switch_values.insert(switch_values.begin(),QString());
	}
      }
    }
  }

  fclose(f);

  return true;
}
