// stereotool.cpp
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

#include <stdio.h>

#include "stereotool.h"

//
// StereoTool Callback
//
#ifdef STEREOTOOL
void __StereoToolCallback(void *priv,gStereoTool *st,void *c)
{
  StereoTool *stereotool=(StereoTool *)priv;
  stereotool->hide();
}
#endif  // STEREOTOOL

StereoTool::StereoTool()
{
  st_stereotool=NULL;
#ifdef STEREOTOOL
  st_stereotool=NULL;
  st_stereotool_gui=NULL;
#endif  // STEREOTOOL
}


StereoTool::~StereoTool()
{
#ifdef STEREOTOOL
  if(st_stereotool!=NULL) {
    stereoTool_Delete(st_stereotool);
  }
  if(st_stereotool_gui!=NULL) {
    stereoTool_GUI_Delete(st_stereotool_gui);
  }
#endif  // STEREOTOOL
}


bool StereoTool::start(const char *key)
{
  if(st_stereotool==NULL) {
#ifdef STEREOTOOL
    st_stereotool=stereoTool_Create(key);
    stereoTool_SetCallback(st_stereotool,__StereoToolCallback,this);
    return true;
#endif  // STEREOTOOL
  }
  return false;
}


bool StereoTool::isActive() const
{
  return st_stereotool!=NULL;
}


void StereoTool::show()
{
#ifdef STEREOTOOL
  if(st_stereotool_gui==NULL) {
    st_stereotool_gui=stereoTool_GUI_Create(st_stereotool);
    stereoTool_GUI_Show(st_stereotool_gui,NULL);
  }
#endif  // STEREOTOOL
}


void StereoTool::hide()
{
#ifdef STEREOTOOL
  if(st_stereotool_gui!=NULL) {
    stereoTool_GUI_Hide(st_stereotool_gui);
    stereoTool_GUI_Delete(st_stereotool_gui);
    st_stereotool_gui=NULL;
  }
#endif  // STEREOTOOL
}


void StereoTool::process(float *pcm,int32_t frames,int32_t chans,
			 int32_t samprate)
{
#ifdef STEREOTOOL
  stereoTool_Process(st_stereotool,pcm,frames,chans,samprate);
#endif  // STEREOTOOL
}


int StereoTool::latency() const
{
  if(st_stereotool!=NULL) {
#ifdef STEREOTOOL
    return stereoTool_GetLatency(st_stereotool);
#endif  // STEREOTOOL
  }
  return 0;
}


int StereoTool::latency(int32_t samprate,bool silence) const
{
  if(st_stereotool!=NULL) {
#ifdef STEREOTOOL
    return stereoTool_GetLatency2(st_stereotool,samprate,silence);
#endif  // STEREOTOOL
  }
  return 0;
}


bool StereoTool::loadPreset(const QString &preset,int type)
{
  if(st_stereotool!=NULL) {
#ifdef STEREOTOOL
    return stereoTool_LoadPreset(st_stereotool,preset.toUtf8(),type);
#endif  // STEREOTOOL
  }
  return false;
}


void StereoTool::reset(int type)
{
#ifdef STEREOTOOL
  stereoTool_Reset(st_stereotool,type);
#endif  // STEREOTOOL
}


bool StereoTool::licenseValid() const
{
#ifdef STEREOTOOL
  return stereoTool_CheckLicenseValid(st_stereotool);
#endif  // STEREOTOOL
  return false;
}


int StereoTool::softwareVersion() const
{
  if(st_stereotool!=NULL) {
#ifdef STEREOTOOL
    return stereoTool_GetSoftwareVersion();
#endif  // STEREOTOOL
  }
  return 0;
}


int StereoTool::apiVersion() const
{
  if(st_stereotool!=NULL) {
#ifdef STEREOTOOL
    return stereoTool_GetApiVersion();
#endif  // STEREOTOOL
  }
  return 0; 
}
