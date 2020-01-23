// guiapplication.h
//
// Abstract base class for GUI applications in GlassCoder
//
//   (C) Copyright 2015-2020 Fred Gleason <fredg@paravelsystems.com>
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

#ifndef GUIAPPLICATION_H
#define GUIAPPLICATION_H

#include <QDir>
#include <QMainWindow>
#include <QStringList>

#define GUI_SETTINGS_DIR ".glassgui"
#define GUI_SETTINGS_FILE "glassguirc"

class GuiApplication : public QMainWindow
{
 Q_OBJECT;
 public:
  GuiApplication(QWidget *parent=0);

 protected:
  QDir *settingsDirectory() const;
  bool setSettingsDirectory(const QString &dirname);
  bool checkSettingsDirectory();
  QString settingsFilename(const QString &instance_name);
  void deleteInstance(const QString &name);
  QStringList listInstances();

 private:
  QString gui_settings_path;
  QDir *gui_settings_dir;
};


#endif  // GUIAPPLICATION_H
