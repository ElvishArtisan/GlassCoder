// glassgui.h
//
// glassgui(1) Audio Encoder front end
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

#ifndef GLASSGUI_H
#define GLASSGUI_H

#include <QDir>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QProcess>
#include <QPushButton>
#include <QSpinBox>
#include <QStringList>
#include <QTimer>

#include "codeviewer.h"
#include "combobox.h"
#include "stereometer.h"

#define GLASSGUI_USAGE ""
#define GLASSGUI_SETTINGS_DIR ".glassgui"
#define GLASSGUI_SETTINGS_FILE "glassguirc"

class MainWidget : public QMainWindow
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
  void serverTypeChanged(int n);
  void codecTypeChanged(int n);
  void codecSamplerateChanged(int n);
  void sourceTypeChanged(int n);
  void codecFinishedData(int exit_code,QProcess::ExitStatus exit_status);
  void checkArgs(const QString &str);
  void deviceFinishedData(int exit_code,QProcess::ExitStatus exit_status);
  void processFinishedData(int exit_code,QProcess::ExitStatus exit_status);
  void processErrorData(QProcess::ProcessError err);
  void processCollectGarbageData();
  void fileSelectName();

 private:
  bool MakeServerArgs(QStringList *args);
  void MakeCodecArgs(QStringList *args);
  bool MakeSourceArgs(QStringList *args);
  void ProcessError(int exit_code,QProcess::ExitStatus exit_status);
  void LoadSettings();
  bool SaveSettings();
  bool CheckSettingsDirectory();
  StereoMeter *gui_meter;
  QPushButton *gui_start_button;
  QPushButton *gui_code_button;

  QLabel *gui_server_label;
  QLabel *gui_server_type_label;
  ComboBox *gui_server_type_box;
  QLabel *gui_server_location_label;
  QLineEdit *gui_server_location_edit;
  QLabel *gui_server_username_label;
  QLineEdit *gui_server_username_edit;
  QLabel *gui_server_password_label;
  QLineEdit *gui_server_password_edit;

  QLabel *gui_codec_label;
  QLabel *gui_codec_type_label;
  ComboBox *gui_codec_type_box;
  QLabel *gui_codec_samplerate_label;
  ComboBox *gui_codec_samplerate_box;
  QLabel *gui_codec_channels_label;
  ComboBox *gui_codec_channels_box;
  QLabel *gui_codec_bitrate_label;
  ComboBox *gui_codec_bitrate_box;

  QLabel *gui_source_label;
  QLabel *gui_source_type_label;
  ComboBox *gui_source_type_box;

  QLabel *gui_file_name_label;
  QLineEdit *gui_file_name_edit;
  QPushButton *gui_file_select_button;

  QLabel *gui_jack_server_name_label;
  QLineEdit *gui_jack_server_name_edit;
  QLabel *gui_jack_client_name_label;
  QLineEdit *gui_jack_client_name_edit;

  CodeViewer *gui_codeviewer_dialog;
  QProcess *gui_process;
  QTimer *gui_process_cleanup_timer;

  QDir *gui_settings_dir;
};


#endif  // GLASSGUI_H
