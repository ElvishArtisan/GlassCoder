// logging.h
//
// Logging routines for glasscoder(1).
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

#include <syslog.h>

#include <QString>

#define LOG_TO_STDERR 0
#define LOG_TO_SYSLOG 1
#define LOG_TO_STDOUT 2

#define CONNECTION_IDLE 0
#define CONNECTION_PENDING 1
#define CONNECTION_OK 2
#define CONNECTION_FAILED 3

extern int global_log_to;
extern bool global_log_verbose;

void Log(int prio,const QString &msg);
