// urlencode.cpp
//
// Test URL encoding methods
//
//   (C) Copyright 2020 Fred Gleason <fredg@paravelsystems.com>
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
#include <stdlib.h>

#include <QCoreApplication>

#include "urlencode.h"

MainObject::MainObject(QObject *parent)
{
  QStringList args=QCoreApplication::arguments();

  if(args.size()!=2) {
    fprintf(stderr,
	    "urlencode: you must provide exactly one string to encode\n");
    exit(1);
  }
  printf("%s => %s\n",
	 (const char *)args.at(1).toUtf8(),
	 (const char *)Connector::urlEncode(args.at(1)).
	 toUtf8());

  exit(0);
}


int main(int argc,char *argv[])
{
  QCoreApplication a(argc,argv);

  new MainObject();

  return a.exec();
}

