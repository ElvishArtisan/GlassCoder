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
#include <QListView>
#include <QMainWindow>
#include <QProcess>
#include <QPushButton>
#include <QSpinBox>
#include <QStringList>
#include <QTimer>

#include "codeviewer.h"
#include "combobox.h"
#include "hpiinputlistview.h"
#include "statuswidget.h"
#include "stereometer.h"

#define GLASSGUI_USAGE ""
#define GLASSGUI_SETTINGS_DIR ".glassgui"
#define GLASSGUI_SETTINGS_FILE "glassguirc"
#define GLASSGUI_TERMINATE_TIMEOUT 5000
#define GLASSGUI_MAX_SUBSTREAMS 3

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
  void processReadyReadStandardOutputData();
  void processFinishedData(int exit_code,QProcess::ExitStatus exit_status);
  void processErrorData(QProcess::ProcessError err);
  void processCollectGarbageData();
  void processKillData();
  void messageTimeoutData();
  void fileSelectName();

 private:
  void LockControls(bool state);
  void ProcessFeedback(const QString &str);
  bool MakeServerArgs(QStringList *args);
  void MakeCodecArgs(QStringList *args);
  void MakeStreamArgs(QStringList *args,bool escape_args);
  bool MakeSourceArgs(QStringList *args,bool escape_args);
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
  ComboBox *gui_codec_bitrate_box[GLASSGUI_MAX_SUBSTREAMS];

  QLabel *gui_stream_label;
  QLabel *gui_stream_name_label;
  QLineEdit *gui_stream_name_edit;
  QLabel *gui_stream_description_label;
  QLineEdit *gui_stream_description_edit;
  QLabel *gui_stream_url_label;
  QLineEdit *gui_stream_url_edit;
  QLabel *gui_stream_genre_label;
  QLineEdit *gui_stream_genre_edit;
  QLabel *gui_stream_icq_label;
  QLineEdit *gui_stream_icq_edit;
  QLabel *gui_stream_aim_label;
  QLineEdit *gui_stream_aim_edit;
  QLabel *gui_stream_irc_label;
  QLineEdit *gui_stream_irc_edit;

  QLabel *gui_source_label;
  QLabel *gui_source_type_label;
  ComboBox *gui_source_type_box;

  QLabel *gui_alsa_device_label;
  QLineEdit *gui_alsa_device_edit;

  QLabel *gui_file_name_label;
  QLineEdit *gui_file_name_edit;
  QPushButton *gui_file_select_button;

  HpiInputListView *gui_asihpi_view;

  QLabel *gui_jack_server_name_label;
  QLineEdit *gui_jack_server_name_edit;
  QLabel *gui_jack_client_name_label;
  QLineEdit *gui_jack_client_name_edit;

  CodeViewer *gui_codeviewer_dialog;
  QProcess *gui_process;
  QTimer *gui_process_cleanup_timer;
  QTimer *gui_process_kill_timer;
  QString gui_process_accum;

  QDir *gui_settings_dir;
  QLabel *gui_message_label;
  QTimer *gui_message_timer;
  StatusWidget *gui_status_widget;
  QLabel *gui_status_frame_widget;
};


#endif  // GLASSGUI_H
