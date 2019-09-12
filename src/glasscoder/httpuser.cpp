// httpuser.cpp
//
// Abstract an HTTP user
//
// (C) Copyright 2016 Fred Gleason <fredg@paravelsystems.com>
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

#include "httpuser.h"

HttpUser::HttpUser(const QString &name,const QString &passwd)
{
  user_name=name;
  user_password=passwd;
}


QString HttpUser::name() const
{
  return user_name;
}


QString HttpUser::password() const
{
  return user_password;
}


void HttpUser::setPassword(const QString &passwd)
{
  user_password=passwd;
}


bool HttpUser::isValid(const QString &name,const QString &passwd)
{
  return (name==user_name)&&(passwd==user_password);
}
