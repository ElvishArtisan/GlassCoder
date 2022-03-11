// glasswidget.h
//
// Encoder widget for GlassCommander(1)
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

#ifndef GLASSWIDGET_H
#define GLASSWIDGET_H

#include <stdio.h>

#include <QDir>
#include <QFrame>
#include <QLabel>
#include <QProcess>
#include <QPushButton>
#include <QTimer>

#include "codecdialog.h"
#include "codeviewer.h"
#include "configdialog.h"
#include "messagewidget.h"
#include "playmeter.h"
#include "profile.h"
#include "serverdialog.h"
#include "sourcedialog.h"
#include "statuswidget.h"
#include "streamdialog.h"

class GlassWidget : public QFrame
{
 Q_OBJECT;
 public:
  enum Mode {NormalMode=0,InsertMode=1,RemoveMode=2};
  GlassWidget(const QString &instance_name,QDir *temp_dir,QWidget *parent=0);
  QSize sizeHint() const;
  bool autoStart() const;
  void setAutoStart(bool state);
  void setMode(Mode mode);
  QString instanceName() const;
  void addCodecTypes(const QString &codecs);
  void addSourceTypes(const QString &sources);
  bool isActive();
  void start();
  void terminate();
  void kill();
  void load(Profile *p);
  void save(FILE *f) const;

 signals:
  void configurationChanged(GlassWidget *encoder);
  void stopped();
  void insertClicked(const QString &instance_name);
  void removeClicked(const QString &instance_name);

 public slots:
  void setNormalMode();
  void setInsertMode();
  void setRemoveMode();

 private slots:
  void startEncodingData();
  void stopEncodingData();
  void processReadyReadStandardOutputData();
  void processFinishedData(int exit_code,QProcess::ExitStatus exit_status);
  void processErrorData(QProcess::ProcessError err);
  void insertData();
  void removeData();
  void configData();
  void checkArgs();
  void serverTypeChangedData(Connector::ServerType type);
  void killData();

 protected:
  void resizeEvent(QResizeEvent *e);

 private:
  void LockControls(bool state);
  void ProcessFeedback(const QString &str);
  void ProcessError(int exit_code,QProcess::ExitStatus exit_status);
  PlayMeter *gw_meters[2];
  QLabel *gw_name_label;
  QLabel *gw_status_frame_widget;
  StatusWidget *gw_status_widget;
  MessageWidget *gw_message_widget;
  QPushButton *gw_start_button;
  ConfigDialog *gw_config_dialog;
  QPushButton *gw_config_button;
  ServerDialog *gw_server_dialog;
  CodecDialog *gw_codec_dialog;
  StreamDialog *gw_stream_dialog;
  SourceDialog *gw_source_dialog;
  CodeViewer *gw_code_dialog;
  QProcess *gw_process;
  QString gw_process_accum;
  Mode gw_mode;
  QPushButton *gw_insert_button;
  QPushButton *gw_remove_button;
  bool gw_auto_start;
  QTimer *gw_kill_timer;
  QDir *gw_temp_dir;
};


#endif  // GLASSWIDGET_H
