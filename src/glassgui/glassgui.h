// glassgui.h
//
// glassgui(1) Audio Encoder front end
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

#ifndef GLASSGUI_H
#define GLASSGUI_H

#include <QDir>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QMainWindow>
#include <QProcess>
#include <QPushButton>
#include <QSpinBox>
#include <QStringList>
#include <QTimer>

#include "codecdialog.h"
#include "codeviewer.h"
#include "combobox.h"
#include "guiapplication.h"
#include "hpiinputlistview.h"
#include "messagewidget.h"
#include "serverdialog.h"
#include "sourcedialog.h"
#include "statuswidget.h"
#include "stereometer.h"
#include "streamdialog.h"

#define GLASSGUI_USAGE "[options]\n"
#define GLASSGUI_TERMINATE_TIMEOUT 5000

class MainWidget : public GuiApplication
{
 Q_OBJECT;
 public:
  MainWidget(QWidget *parent=0);
  QSize sizeHint() const;

 protected:
  void closeEvent(QCloseEvent *e);
  void resizeEvent(QResizeEvent *e);

 private slots:
  void startEncodingData();
  void stopEncodingData();
  void showCodeData();
  void metadataData();
  void serverTypeChangedData(Connector::ServerType type,bool multirate);
  void serverData();
  void codecData();
  void streamData();
  void sourceData();
  void codecFinishedData(int exit_code,QProcess::ExitStatus exit_status);
  void checkArgs();
  void checkArgs(const QString &str);
  void deviceFinishedData(int exit_code,QProcess::ExitStatus exit_status);
  void processReadyReadStandardOutputData();
  void processFinishedData(int exit_code,QProcess::ExitStatus exit_status);
  void processErrorData(QProcess::ProcessError err);
  void processCollectGarbageData();
  void processKillData();

 private:
  QString instance_name;
  void LockControls(bool state);
  void ProcessFeedback(const QString &str);
  void ProcessError(int exit_code,QProcess::ExitStatus exit_status);
  void LoadSettings();
  bool SaveSettings();
  void ListInstances();
  void ExitProgram(int exit_code) const;
  StereoMeter *gui_meter;
  QPushButton *gui_start_button;
  QLabel *gui_metadata_label;
  QLineEdit *gui_metadata_edit;
  QPushButton *gui_metadata_button;
  QPushButton *gui_code_button;
  ServerDialog *gui_server_dialog;
  QPushButton *gui_server_button;
  CodecDialog *gui_codec_dialog;
  QPushButton *gui_codec_button;
  StreamDialog *gui_stream_dialog;
  QPushButton *gui_stream_button;
  SourceDialog *gui_source_dialog;
  QPushButton *gui_source_button;
  CodeViewer *gui_codeviewer_dialog;
  QProcess *gui_process;
  QTimer *gui_process_cleanup_timer;
  QTimer *gui_process_kill_timer;
  QString gui_process_accum;
  MessageWidget *gui_message_widget;
  StatusWidget *gui_status_widget;
  QLabel *gui_status_frame_widget;
  bool gui_autostart;
  QTimer *gui_autostart_timer;
  QDir *gui_temp_dir;
};


#endif  // GLASSGUI_H
