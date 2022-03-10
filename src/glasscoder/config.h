// config.h
//
// Configuration Class for glasscoder(1)
//
// (C) Copyright 2016-2022 Fred Gleason <fredg@paravelsystems.com>
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

#ifndef CONFIG_H
#define CONFIG_H

#include <QUrl>

#include "audiodevice.h"
#include "cmdswitch.h"
#include "codec.h"
#include "connector.h"

#define GLASSCODER_CREDENTIALS "creds"
#define GLASSCODER_USAGE "[options]\n"

class Config
{
 public:
  enum ExitCode {ExitOk=0,ExitRetry=1,ExitFatal=2};
  Config();
  bool audioAtomicFrames() const;
  unsigned audioBitrate() const;
  unsigned audioChannels() const;
  AudioDevice::DeviceType audioDevice() const;
  Codec::Type audioFormat() const;
  unsigned audioQuality() const;
  unsigned audioSamplerate() const;
  bool serverExitOnLast() const;
  int serverMaxConnections() const;
  QString serverPassword() const;
  QString credentialsFile() const;
  bool deleteCredentials() const;
  QString sshIdentity() const;
  QString serverScriptDown() const;
  QString serverScriptUp() const;
  int serverStartConnections() const;
  Connector::ServerType serverType() const;
  QUrl serverUrl() const;
  QString serverBaseUrl() const;
  QString serverUserAgent() const;
  QString serverUsername() const;
  QString serverPipe() const;
  bool serverNoDeletes() const;
  QString streamAim() const;
  QString streamDescription() const;
  QString streamGenre() const;
  QString streamIcq() const;
  QString streamIrc() const;
  QString streamName() const;
  int streamTimestampOffset() const;
  QString streamUrl() const;
  QStringList deviceKeys() const;
  QStringList deviceValues() const;
  bool listCodecs() const;
  bool listDevices() const;
  unsigned metadataPort() const;
  bool meterData() const;
  bool dumpHeaders() const;
  bool verbose() const;

 private:
  void ListCodecs() const;
  void ListDevices() const;

  //
  // Audio Arguments
  //
  bool audio_atomic_frames;
  unsigned audio_bitrate;
  unsigned audio_channels;
  AudioDevice::DeviceType audio_device;
  Codec::Type audio_format;
  double audio_quality;
  unsigned audio_samplerate;

  //
  // Server Arguments
  //
  bool server_exit_on_last;
  int server_max_connections;
  QString server_password;
  QString credentials_file;
  bool delete_credentials;
  QString ssh_identity;
  QString server_script_down;
  QString server_script_up;
  int server_start_connections;
  Connector::ServerType server_type;
  QUrl server_url;
  QString server_base_url;
  QString server_user_agent;
  QString server_username;
  QString server_pipe;
  bool server_no_deletes;

  //
  // Stream Arguments
  //
  QString stream_aim;
  QString stream_description;
  QString stream_genre;
  QString stream_icq;
  QString stream_irc;
  QString stream_name;
  int stream_timestamp_offset;
  QString stream_url;

  //
  // Device Arguments
  //
  QStringList device_keys;
  QStringList device_values;

  //
  // Miscellaneous Arguments
  //
  bool list_codecs;
  bool list_devices;
  unsigned metadata_port;
  bool meter_data;
  bool dump_headers;
  bool show_verbose;
};


#endif  // CONFIG_H
