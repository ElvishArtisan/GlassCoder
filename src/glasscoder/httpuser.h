// httpuser.h
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

#ifndef HTTPUSER_H
#define HTTPUSER_H

#include <QString>

class HttpUser
{
 public:
  HttpUser(const QString &name,const QString &passwd);
  QString name() const;
  QString password() const;
  void setPassword(const QString &passwd);
  bool isValid(const QString &name,const QString &passwd);

 private:
  QString user_name;
  QString user_password;
};


#endif  // HTTPUSER_H
