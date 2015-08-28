// asihpi.cpp
//
// Source Mappings for ASIHPI devices
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

#ifdef ASIHPI
#include <asihpi/hpi.h>
#endif  // ASIHPI

#include "asihpi.h"

//
// Name Table
//
QString asihpi_common_source_names[]={"NONE",
				      "OSTREAM",
				      "LINEIN",
				      "AESEBU",
				      "TUNER",
				      "RF",
				      "CLOCK",
				      "BITSTREAM",
				      "MIC",
				      "COBRANET",
				      "ANALOG",
				      "ADAPTER",
				      "RTP",
				      "INTERNAL",
				      "AVB",
				      "BLULINK"};

uint16_t AsihpiSourceNode(const QString &str)
{
  uint16_t ret=0;

#ifdef ASIHPI
  for(uint16_t i=HPI_SOURCENODE_NONE;i<HPI_SOURCENODE_LAST_INDEX;i++) {
    if(AsihpiSourceName(i).toLower()==str.toLower()) {
      ret=i;
    }
  }
#endif  // ASIHPI

  return ret;
}


QString AsihpiSourceName(uint16_t src)
{
  QString ret;

#ifdef ASIHPI
  if((src>=HPI_SOURCENODE_NONE)&&(src<HPI_SOURCENODE_LAST_INDEX)) {
    ret=asihpi_common_source_names[src-HPI_SOURCENODE_NONE];
  }
#endif  // ASIHPI
  return ret;
}
