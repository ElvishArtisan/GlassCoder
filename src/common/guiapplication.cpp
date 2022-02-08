// guiapplication.cpp
//
// Abstract base class for GUI applications in GlassCoder
//
//   (C) Copyright 2015-2022 Fred Gleason <fredg@paravelsystems.com>
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

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <QMessageBox>

#include "guiapplication.h"

GuiApplication::GuiApplication(QWidget *parent)
  : QMainWindow(parent)
{
  gui_settings_path="";
  gui_settings_dir=NULL;
  checkSettingsDirectory();
}


QDir *GuiApplication::settingsDirectory() const
{
  return gui_settings_dir;
}


bool GuiApplication::setSettingsDirectory(const QString &dirname)
{
  gui_settings_path=dirname;
  return checkSettingsDirectory();
}


bool GuiApplication::checkSettingsDirectory()
{
  QString path;

  if(gui_settings_path.isEmpty()) {
    path=QString("/")+GUI_SETTINGS_DIR;
    if(getenv("HOME")!=NULL) {
      path=QString(getenv("HOME"))+"/"+GUI_SETTINGS_DIR;
    }
  }
  else {
    path=gui_settings_path;
  }
  if((gui_settings_dir==NULL)||(gui_settings_dir->path()!=path)) {
    gui_settings_dir=new QDir(path);
  }
  if(!gui_settings_dir->exists()) {
    mkdir(path.toUtf8(),S_IRUSR|S_IWUSR|S_IXUSR);
    if(!gui_settings_dir->exists()) {
      return false;
    }
  }
  return true;
}


QString GuiApplication::settingsFilename(const QString &instance_name)
{
  if(!checkSettingsDirectory()) {
    QMessageBox::critical(this,"GlassGui - "+tr("Error"),
			  tr("Unable to create settings directory!"));
    exit(256);
  }
  QString ret=gui_settings_dir->path()+"/"+GUI_SETTINGS_FILE;
  if(!instance_name.isEmpty()) {
    ret+="-"+instance_name;
  }

  return ret;
}


void GuiApplication::deleteInstance(const QString &name)
{
  if(checkSettingsDirectory()) {
    unlink((gui_settings_dir->path()+"/"+GUI_SETTINGS_FILE+"-"+name).toUtf8());
  }
}


QStringList GuiApplication::listInstances()
{
  QStringList files;
  QStringList ret;
  if(checkSettingsDirectory()) {
    files=gui_settings_dir->
      entryList(QStringList(QString(GUI_SETTINGS_FILE)+"-*"),
		QDir::Files,QDir::Name);
    for(int i=0;i<files.size();i++) {
      files[i]=files[i].right(files[i].length()-11);
      ret.push_back(files[i]);
    }
  }
  return ret;
}
