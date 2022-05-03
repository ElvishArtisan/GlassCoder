// glasscommander.h
//
// glasscommander(1) Audio Encoder front end
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

#ifndef GLASSCOMMANDER_H
#define GLASSCOMMANDER_H

#include <QDir>
#include <QList>
#include <QProcess>
#include <QTimer>
#include <QToolBar>

#include "deletedialog.h"
#include "glasswidget.h"
#include "guiapplication.h"
#include "instancedialog.h"

#define GLASSCOMMANDER_USAGE "[options]\n"
#define GLASSCOMMANDER_SETTINGS_FILE QString("glasscommanderrc")
#define GLASSCOMMANDER_TEMP_KEEPALIVE_FILENAME QString("keepalive")
#define GLASSCOMMANDER_TEMP_KEEPALIVE_INTERVAL 60000
#define GLASSCOMMANDER_TERMINATE_TIMEOUT 5000

class MainWidget : public GuiApplication
{
 Q_OBJECT;
 public:
  MainWidget(QWidget *parent=0);
  QSize sizeHint() const;

 private slots:
  void addInstanceData();
  void removeInstanceData();
  void abandonInstanceData();
  void topInsertClickedData();
  void insertClickedData(const QString &instance_name);
  void removeClickedData(const QString &instance_name);
  void codecFinishedData(int exit_code,QProcess::ExitStatus exit_status);
  void deviceFinishedData(int exit_code,QProcess::ExitStatus exit_status);
  void processErrorData(QProcess::ProcessError err);
  void configurationChangedData(GlassWidget *encoder);
  void autostartData();
  void startAllData();
  void stopAllData();
  void encoderStoppedData();
  void stopTimeoutData();
  void updateKeepaliveData();

 protected:
  void closeEvent(QCloseEvent *e);
  void resizeEvent(QResizeEvent *e);

 private:
  void ConnectEncoder(GlassWidget *encoder);
  void LoadEncoders();
  void SaveEncoders();
  void LoadEncoderConfig(GlassWidget *encoder);
  int GetEncoderPosition(const QString &instance_name) const;
  void ProcessError(int exit_code,QProcess::ExitStatus exit_status);
  void ExitProgram(int exit_code) const;
  InstanceDialog *gui_instance_dialog;
  DeleteDialog *gui_delete_dialog;
  QString gui_new_instance_name;
  QToolBar *gui_toolbar;
  QAction *gui_remove_action;
  QAction *gui_abandon_action;
  QList<GlassWidget *> gui_encoders;
  QString gui_codec_types;
  QString gui_source_types;
  QProcess *gui_process;
  QPushButton *gui_insert_button;
  QPushButton *gui_startall_button;
  QPushButton *gui_stopall_button;
  QTimer *gui_stop_timer;
  int gui_stop_count;
  QTimer *gui_autostart_timer;
  int gui_autostart_index;
  bool gui_starting_all;
  QDir *gui_temp_dir;
  QTimer *gui_temp_keepalive_timer;
  QString gui_temp_keepalive_pathname;
  int gui_temp_keepalive_fd;
};


#endif  // GLASSCOMMANDER_H
