// socketmessage.h
//
// Abstract a WebSockets message
//
// (C) Copyright 2016-2019 Fred Gleason <fredg@paravelsystems.com>
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

#ifndef SOCKETMESSAGE_H
#define SOCKETMESSAGE_H

#include <QByteArray>

class SocketMessage
{
 public:
  enum OpCode {Continuation=0,Text=1,Binary=2,AppReserv3=3,
	       AppReserv4=4,AppReserv5=5,AppReserv6=6,AppReserv7=7,
	       Close=8,Ping=9,Pong=10,CntlReserv11=11,
	       CntlReserv12=12,CntlReserv13=13,CntlReserv14=14,CntlReserv15=15};
  SocketMessage();
  OpCode opCode() const;
  void setOpCode(OpCode opcode);
  QByteArray payload() const;
  void appendPayload(const char c);
  void clearPayload();
  static bool isControlMessage(OpCode opcode);

 private:
  OpCode sock_op_code;
  QByteArray sock_payload;
};


#endif  // SOCKETMESSAGE_H
