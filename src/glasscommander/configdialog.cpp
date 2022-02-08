// configdialog.cpp
//
// Show configration buttons for GlassCommander
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

#include "configdialog.h"

ConfigDialog::ConfigDialog(const QString &instance_name,
			   ServerDialog *server_dialog,
			   CodecDialog *codec_dialog,
			   StreamDialog *stream_dialog,
			   SourceDialog *source_dialog,
			   CodeViewer *code_dialog,QWidget *parent)
  : QDialog(parent)
{
  conf_server_dialog=server_dialog;
  conf_codec_dialog=codec_dialog;
  conf_stream_dialog=stream_dialog;
  conf_source_dialog=source_dialog;
  conf_code_dialog=code_dialog;

  setWindowTitle(tr("Instance")+": "+instance_name);

  QFont bold_font(font().family(),font().pointSize(),QFont::Bold);

  //
  // Server Settings
  //
  conf_server_button=new QPushButton(tr("Server")+"\n"+tr("Settings"),this);
  conf_server_button->setFont(bold_font);
  connect(conf_server_button,SIGNAL(clicked()),conf_server_dialog,SLOT(exec()));

  //
  // Codec Settings
  //
  conf_codec_button=new QPushButton(tr("Codec")+"\n"+tr("Settings"),this);
  conf_codec_button->setFont(bold_font);
  connect(conf_codec_button,SIGNAL(clicked()),conf_codec_dialog,SLOT(exec()));

  //
  // Stream Settings
  //
  conf_stream_button=new QPushButton(tr("Stream")+"\n"+tr("Settings"),this);
  conf_stream_button->setFont(bold_font);
  connect(conf_stream_button,SIGNAL(clicked()),conf_stream_dialog,SLOT(exec()));

  //
  // Source Settings
  //
  conf_source_button=new QPushButton(tr("Source")+"\n"+tr("Settings"),this);
  conf_source_button->setFont(bold_font);
  connect(conf_source_button,SIGNAL(clicked()),conf_source_dialog,SLOT(exec()));

  //
  // Code Viewer
  //
  conf_code_button=new QPushButton(tr("Show")+"\n"+tr("Code"),this);
  conf_code_button->setFont(bold_font);
  connect(conf_code_button,SIGNAL(clicked()),this,SLOT(showCodeDialog()));

  //
  // Autostart
  //
  conf_autostart_checkbox=new QCheckBox(this);
  conf_autostart_label=
    new QLabel(tr("Start this instance at application launch"),this);
  conf_autostart_label->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
  conf_autostart_label->setFont(bold_font);
}


QSize ConfigDialog::sizeHint() const
{
  return QSize(460,100);
}


int ConfigDialog::exec(bool *autostart)
{
  conf_autostart=autostart;
  conf_autostart_checkbox->setChecked(*autostart);

  return QDialog::exec();
}


void ConfigDialog::showCodeDialog()
{
  QStringList args;

  args.push_back("glasscoder");
  conf_server_dialog->makeArgs(&args,true);
  conf_codec_dialog->makeArgs(&args);
  conf_stream_dialog->makeArgs(&args,true);
  conf_source_dialog->makeArgs(&args,true);

  conf_code_dialog->exec(args);
}


void ConfigDialog::closeEvent(QCloseEvent *e)
{
  *conf_autostart=conf_autostart_checkbox->isChecked();
  done(0);
}


void ConfigDialog::resizeEvent(QResizeEvent *e)
{
  conf_server_button->setGeometry(10,10,80,50);

  conf_codec_button->setGeometry(100,10,80,50);

  conf_stream_button->setGeometry(190,10,80,50);

  conf_source_button->setGeometry(280,10,80,50);

  conf_code_button->setGeometry(370,10,80,50);

  conf_autostart_checkbox->setGeometry(20,65,20,20);
  conf_autostart_label->setGeometry(45,65,size().width()-45,20);
}
