// stereotool.h
//
// Qt binding for the StereoTool Radio Processor
//
// (C) Copyright 2017 Fred Gleason <fredg@paravelsystems.com>
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser Public License version 2 as
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

#ifndef STEREOTOOL_H
#define STEREOTOOL_H

#include <stdint.h>

#include <QSize>
#include <QString>

#ifdef STEREOTOOL
#define _ST_LINUX
#define __int32 int32_t
#include <stereotool/Generic_StereoTool.h>
#include <stereotool/ParameterEnum.h>
#endif  // STEREOTOOL

class StereoTool
{
 public:
  StereoTool(bool activate,const QString &key);
  ~StereoTool();
  bool start();
  bool isActive() const;
  void show();
  void hide();
  void process(float *pcm,int32_t frames,int32_t chans,int32_t samprate);
  int latency() const;
  int latency(int32_t samprate,bool silence) const;
  bool loadPreset(const QString &preset,int type);
  void reset(int type);
  bool licenseValid() const;
  int softwareVersion() const;
  int apiVersion() const;

 private:
#ifdef STEREOTOOL
  bool st_activate;
  QString st_key;
  bool st_visibility;
  gStereoTool *st_stereotool;
  gStereoToolGUI *st_stereotool_gui;
  friend void __StereoToolCallback(void *priv,gStereoTool *st,void *c);
#endif  // STEREOTOOL
};


#endif  // STEREOTOOL_H
