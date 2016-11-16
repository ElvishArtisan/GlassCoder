// spinbox.cpp
//
// SpinBox widget for GlassGui
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

#include "spinbox.h"

SpinBox::SpinBox(QWidget *parent)
  : QSpinBox(parent)
{
  spin_read_only=false;
}


bool SpinBox::isReadOnly() const
{
  return spin_read_only;
}


void SpinBox::setReadOnly(bool state)
{
  if(state!=spin_read_only) {
    if(state) {
      spin_maximum=maximum();
      spin_minimum=minimum();
      setRange(value(),value());
    }
    else {
      setRange(spin_minimum,spin_maximum);
    }
    spin_read_only=state;
  }
}
