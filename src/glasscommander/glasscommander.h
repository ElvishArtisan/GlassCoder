// glasscommander.h
//
// glasscommander(1) Audio Encoder front end
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

#ifndef GLASSCOMMANDER_H
#define GLASSCOMMANDER_H

#include <QList>
#include <QProcess>

#include "glasswidget.h"
#include "guiapplication.h"

#define GLASSCOMMANDER_USAGE "[options]\n"
#define GLASSCOMMANDER_SETTINGS_FILE QString("glasscommanderrc")
#define GLASSCOMMANDER_TERMINATE_TIMEOUT 5000

class MainWidget : public GuiApplication
{
 Q_OBJECT;
 public:
  MainWidget(QWidget *parent=0);
  QSize sizeHint() const;

 private slots:
  void codecFinishedData(int exit_code,QProcess::ExitStatus exit_status);
  void deviceFinishedData(int exit_code,QProcess::ExitStatus exit_status);
  void processErrorData(QProcess::ProcessError err);
  void configurationChangedData(GlassWidget *encoder);

 protected:
  void closeEvent(QCloseEvent *e);
  void resizeEvent(QResizeEvent *e);

 private:
  void LoadEncoders();
  void ProcessError(int exit_code,QProcess::ExitStatus exit_status);
  QList<GlassWidget *> gui_encoders;
  QString gui_codec_types;
  QString gui_source_types;
  QProcess *gui_process;
};


#endif  // GLASSCOMMANDER_H
