// config.h
//
// Configuration Class for glasscoder(1)
//
// (C) Copyright 2016 Fred Gleason <fredg@paravelsystems.com>
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

#define GLASSCODER_USAGE "[options]\n"

class Config
{
 public:
  Config();
  bool audioAtomicFrames() const;
  unsigned audioBitrateQuantity() const;
  unsigned audioBitrate(int n=-1) const;
  std::vector<unsigned> *audioBitrates() const;
  unsigned audioChannels() const;
  AudioDevice::DeviceType audioDevice() const;
  Codec::Type audioFormat() const;
  unsigned audioQuality() const;
  unsigned audioSamplerate() const;
  bool serverExitOnLast() const;
  int serverMaxConnections() const;
  QString serverPassword() const;
  QString serverScriptDown() const;
  QString serverScriptUp() const;
  int serverStartConnections() const;
  Connector::ServerType serverType() const;
  QUrl serverUrl() const;
  QString serverUserAgent() const;
  QString serverUsername() const;
  QString serverPipe() const;
  bool stereoToolEnabled() const;
  QString stereoToolKey() const;
  QString stereoToolBuiltInPreset() const;
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
  bool listPresets() const;
  unsigned metadataPort() const;
  bool meterData() const;

 private:
  void ListCodecs() const;
  void ListDevices() const;
  void ListPresets() const;

  //
  // Audio Arguments
  //
  bool audio_atomic_frames;
  std::vector<unsigned> *audio_bitrate;
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
  QString server_script_down;
  QString server_script_up;
  int server_start_connections;
  Connector::ServerType server_type;
  QUrl server_url;
  QString server_user_agent;
  QString server_username;
  QString server_pipe;

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
  // Stereotool Arguments
  //
  bool stereotool_enable;
  QString stereotool_key;
  QString stereotool_builtin_preset;

  //
  // Miscellaneous Arguments
  //
  bool list_codecs;
  bool list_devices;
  bool list_presets;
  unsigned metadata_port;
  bool meter_data;
};


#endif  // CONFIG_H
