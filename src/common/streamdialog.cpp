// streamdialog.cpp
//
// Configuration dialog for stream metadata settings
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

#include "streamdialog.h"

StreamDialog::StreamDialog(const QString &caption,QWidget *parent)
  : QDialog(parent)
{
  gui_caption=caption;

  //
  // Fonts
  //
  QFont label_font("helvetica",14,QFont::Bold);
  label_font.setPixelSize(14);

  setWindowTitle(gui_caption+" - "+tr("Stream Metadata Settings"));

  //
  // Stream Name
  //
  gui_stream_name_label=new QLabel(tr("Name")+":",this);
  gui_stream_name_label->setFont(label_font);
  gui_stream_name_label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
  gui_stream_name_edit=new QLineEdit(this);

  //
  // Stream Description
  //
  gui_stream_description_label=new QLabel(tr("Description")+":",this);
  gui_stream_description_label->setFont(label_font);
  gui_stream_description_label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
  gui_stream_description_edit=new QLineEdit(this);

  //
  // Stream URL
  //
  gui_stream_url_label=new QLabel(tr("URL")+":",this);
  gui_stream_url_label->setFont(label_font);
  gui_stream_url_label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
  gui_stream_url_edit=new QLineEdit(this);

  //
  // Stream Genre
  //
  gui_stream_genre_label=new QLabel(tr("Genre")+":",this);
  gui_stream_genre_label->setFont(label_font);
  gui_stream_genre_label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
  gui_stream_genre_edit=new QLineEdit(this);

  //
  // Stream Icq
  //
  gui_stream_icq_label=new QLabel(tr("ICQ ID")+":",this);
  gui_stream_icq_label->setFont(label_font);
  gui_stream_icq_label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
  gui_stream_icq_edit=new QLineEdit(this);

  //
  // Stream AOL Instant Messager ID
  //
  gui_stream_aim_label=new QLabel(tr("AOL IM ID")+":",this);
  gui_stream_aim_label->setFont(label_font);
  gui_stream_aim_label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
  gui_stream_aim_edit=new QLineEdit(this);

  //
  // Internet Relay Chat ID
  //
  gui_stream_irc_label=new QLabel(tr("IRC ID")+":",this);
  gui_stream_irc_label->setFont(label_font);
  gui_stream_irc_label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
  gui_stream_irc_edit=new QLineEdit(this);

  //
  // Timestamp Offset
  //
  gui_stream_timestamp_offset_label=new QLabel(tr("Timestamp Offset")+":",this);
  gui_stream_timestamp_offset_label->setFont(label_font);
  gui_stream_timestamp_offset_label->
    setAlignment(Qt::AlignRight|Qt::AlignVCenter);
  gui_stream_timestamp_offset_spin=new SpinBox(this);
  gui_stream_timestamp_offset_spin->setRange(-300,300);
  gui_stream_timestamp_offset_unit=new QLabel(tr("seconds"),this);
  gui_stream_timestamp_offset_unit->setFont(label_font);
  gui_stream_timestamp_offset_unit->
    setAlignment(Qt::AlignLeft|Qt::AlignVCenter);

  //
  // Close Button
  //
  gui_close_button=new QPushButton(tr("Close"),this);
  gui_close_button->setFont(label_font);
  connect(gui_close_button,SIGNAL(clicked()),this,SLOT(hide()));

  setMinimumHeight(sizeHint().height());
  setMaximumHeight(sizeHint().height());
  setMinimumSize(sizeHint());
}


QSize StreamDialog::sizeHint() const
{
  return QSize(400,276);
}


void StreamDialog::makeArgs(QStringList *args,bool escape_args)
{
  QString quote="";
  if(escape_args) {
    quote="\"";
  }
  if(!gui_stream_name_edit->text().isEmpty()) {
    args->push_back("--stream-name="+quote+gui_stream_name_edit->text()+quote);
  }
  if(!gui_stream_description_edit->text().isEmpty()) {
    args->push_back("--stream-description="+quote+
		    gui_stream_description_edit->text()+quote);
  }
  if(!gui_stream_url_edit->text().isEmpty()) {
    args->push_back("--stream-url="+quote+gui_stream_url_edit->text()+quote);
  }
  if(!gui_stream_genre_edit->text().isEmpty()) {
    args->push_back("--stream-genre="+quote+gui_stream_genre_edit->text()+
		    quote);
  }
  if(!gui_stream_icq_edit->text().isEmpty()) {
    args->push_back("--stream-icq="+quote+gui_stream_icq_edit->text()+quote);
  }
  if(!gui_stream_aim_edit->text().isEmpty()) {
    args->push_back("--stream-aim="+quote+gui_stream_aim_edit->text()+quote);
  }
  if(!gui_stream_irc_edit->text().isEmpty()) {
    args->push_back("--stream-irc="+quote+gui_stream_irc_edit->text()+quote);
  }
  if(gui_stream_timestamp_offset_spin->value()!=0) {
    args->push_back("--stream-timestamp-offset="+
	   QString().sprintf("%d",gui_stream_timestamp_offset_spin->value()));
  }
}


void StreamDialog::setServerType(Connector::ServerType type)
{
  switch(type) {
  case Connector::HlsServer:
    gui_stream_name_label->setEnabled(false);
    gui_stream_name_edit->setEnabled(false);
    gui_stream_description_label->setEnabled(false);
    gui_stream_description_edit->setEnabled(false);
    gui_stream_url_label->setEnabled(false);
    gui_stream_url_edit->setEnabled(false);
    gui_stream_genre_label->setEnabled(false);
    gui_stream_genre_edit->setEnabled(false);
    gui_stream_icq_label->setEnabled(false);
    gui_stream_icq_edit->setEnabled(false);
    gui_stream_aim_label->setEnabled(false);
    gui_stream_aim_edit->setEnabled(false);
    gui_stream_irc_label->setEnabled(false);
    gui_stream_irc_edit->setEnabled(false);
    gui_stream_timestamp_offset_label->setEnabled(true);
    gui_stream_timestamp_offset_spin->setEnabled(true);
    gui_stream_timestamp_offset_unit->setEnabled(true);
    break;

  case Connector::FileServer:
  case Connector::FileArchiveServer:
    gui_stream_name_label->setEnabled(false);
    gui_stream_name_edit->setEnabled(false);
    gui_stream_description_label->setEnabled(false);
    gui_stream_description_edit->setEnabled(false);
    gui_stream_url_label->setEnabled(false);
    gui_stream_url_edit->setEnabled(false);
    gui_stream_genre_label->setEnabled(false);
    gui_stream_genre_edit->setEnabled(false);
    gui_stream_icq_label->setEnabled(false);
    gui_stream_icq_edit->setEnabled(false);
    gui_stream_aim_label->setEnabled(false);
    gui_stream_aim_edit->setEnabled(false);
    gui_stream_irc_label->setEnabled(false);
    gui_stream_irc_edit->setEnabled(false);
    gui_stream_timestamp_offset_label->setEnabled(false);
    gui_stream_timestamp_offset_spin->setEnabled(false);
    gui_stream_timestamp_offset_unit->setEnabled(false);
    break;

  case Connector::Shoutcast1Server:
    gui_stream_name_label->setEnabled(true);
    gui_stream_name_edit->setEnabled(true);
    gui_stream_description_label->setEnabled(false);
    gui_stream_description_edit->setEnabled(false);
    gui_stream_url_label->setEnabled(true);
    gui_stream_url_edit->setEnabled(true);
    gui_stream_genre_label->setEnabled(true);
    gui_stream_genre_edit->setEnabled(true);
    gui_stream_icq_label->setEnabled(true);
    gui_stream_icq_edit->setEnabled(true);
    gui_stream_aim_label->setEnabled(true);
    gui_stream_aim_edit->setEnabled(true);
    gui_stream_irc_label->setEnabled(true);
    gui_stream_irc_edit->setEnabled(true);
    gui_stream_timestamp_offset_label->setEnabled(false);
    gui_stream_timestamp_offset_spin->setEnabled(false);
    gui_stream_timestamp_offset_unit->setEnabled(false);
    break;

  case Connector::Shoutcast2Server:
    gui_stream_name_label->setEnabled(true);
    gui_stream_name_edit->setEnabled(true);
    gui_stream_description_label->setEnabled(false);
    gui_stream_description_edit->setEnabled(false);
    gui_stream_url_label->setEnabled(false);
    gui_stream_url_edit->setEnabled(false);
    gui_stream_genre_label->setEnabled(true);
    gui_stream_genre_edit->setEnabled(true);
    gui_stream_icq_label->setEnabled(true);
    gui_stream_icq_edit->setEnabled(true);
    gui_stream_aim_label->setEnabled(true);
    gui_stream_aim_edit->setEnabled(true);
    gui_stream_irc_label->setEnabled(true);
    gui_stream_irc_edit->setEnabled(true);
    gui_stream_timestamp_offset_label->setEnabled(false);
    gui_stream_timestamp_offset_spin->setEnabled(false);
    gui_stream_timestamp_offset_unit->setEnabled(false);
    break;

  case Connector::Icecast2Server:
  case Connector::IcecastOutServer:
    gui_stream_name_label->setEnabled(true);
    gui_stream_name_edit->setEnabled(true);
    gui_stream_description_label->setEnabled(true);
    gui_stream_description_edit->setEnabled(true);
    gui_stream_url_label->setEnabled(true);
    gui_stream_url_edit->setEnabled(true);
    gui_stream_genre_label->setEnabled(true);
    gui_stream_genre_edit->setEnabled(true);
    gui_stream_icq_label->setEnabled(false);
    gui_stream_icq_edit->setEnabled(false);
    gui_stream_aim_label->setEnabled(false);
    gui_stream_aim_edit->setEnabled(false);
    gui_stream_irc_label->setEnabled(false);
    gui_stream_irc_edit->setEnabled(false);
    gui_stream_timestamp_offset_label->setEnabled(false);
    gui_stream_timestamp_offset_spin->setEnabled(false);
    gui_stream_timestamp_offset_unit->setEnabled(false);
    break;

  case Connector::IcecastStreamerServer:
    gui_stream_name_label->setEnabled(true);
    gui_stream_name_edit->setEnabled(true);
    gui_stream_description_label->setEnabled(true);
    gui_stream_description_edit->setEnabled(true);
    gui_stream_url_label->setEnabled(true);
    gui_stream_url_edit->setEnabled(true);
    gui_stream_genre_label->setEnabled(true);
    gui_stream_genre_edit->setEnabled(true);
    gui_stream_icq_label->setEnabled(false);
    gui_stream_icq_edit->setEnabled(false);
    gui_stream_aim_label->setEnabled(false);
    gui_stream_aim_edit->setEnabled(false);
    gui_stream_irc_label->setEnabled(false);
    gui_stream_irc_edit->setEnabled(false);
    gui_stream_timestamp_offset_label->setEnabled(false);
    gui_stream_timestamp_offset_spin->setEnabled(false);
    gui_stream_timestamp_offset_unit->setEnabled(false);
    break;

  case Connector::LastServer:
    break;
  }
}


void StreamDialog::setControlsLocked(bool state)
{
  gui_stream_name_edit->setReadOnly(state);
  gui_stream_description_edit->setReadOnly(state);
  gui_stream_url_edit->setReadOnly(state);
  gui_stream_genre_edit->setReadOnly(state);
  gui_stream_icq_edit->setReadOnly(state);
  gui_stream_aim_edit->setReadOnly(state);
  gui_stream_irc_edit->setReadOnly(state);
  gui_stream_timestamp_offset_spin->setReadOnly(state);
}


void StreamDialog::load(Profile *p)
{
  gui_stream_name_edit->setText(p->stringValue("GlassGui","StreamName"));
  gui_stream_description_edit->
    setText(p->stringValue("GlassGui","StreamDescription"));
  gui_stream_url_edit->setText(p->stringValue("GlassGui","StreamUrl"));
  gui_stream_genre_edit->setText(p->stringValue("GlassGui","StreamGenre"));
  gui_stream_icq_edit->setText(p->stringValue("GlassGui","StreamIcq"));
  gui_stream_aim_edit->setText(p->stringValue("GlassGui","StreamAim"));
  gui_stream_irc_edit->setText(p->stringValue("GlassGui","StreamIrc"));
  gui_stream_timestamp_offset_spin->
    setValue(p->intValue("GlassGui","StreamTimestampOffset"));
}


void StreamDialog::save(FILE *f)
{
  fprintf(f,"StreamName=%s\n",
	  (const char *)gui_stream_name_edit->text().toUtf8());
  fprintf(f,"StreamDescription=%s\n",
	  (const char *)gui_stream_description_edit->text().toUtf8());
  fprintf(f,"StreamUrl=%s\n",
	  (const char *)gui_stream_url_edit->text().toUtf8());
  fprintf(f,"StreamGenre=%s\n",
	  (const char *)gui_stream_genre_edit->text().toUtf8());
  fprintf(f,"StreamIcq=%s\n",
	  (const char *)gui_stream_icq_edit->text().toUtf8());
  fprintf(f,"StreamAim=%s\n",
	  (const char *)gui_stream_aim_edit->text().toUtf8());
  fprintf(f,"StreamIrc=%s\n",
	  (const char *)gui_stream_irc_edit->text().toUtf8());
  fprintf(f,"StreamTimestampOffset=%d\n",
	  gui_stream_timestamp_offset_spin->value());
}


void StreamDialog::resizeEvent(QResizeEvent *e)
{
  int ypos=10;

  gui_stream_name_label->setGeometry(10,ypos,110,24);
  gui_stream_name_edit->setGeometry(125,ypos,size().width()-145,24);
  ypos+=26;

  gui_stream_description_label->setGeometry(10,ypos,110,24);
  gui_stream_description_edit->setGeometry(125,ypos,size().width()-145,24);
  ypos+=26;

  gui_stream_url_label->setGeometry(10,ypos,110,24);
  gui_stream_url_edit->setGeometry(125,ypos,size().width()-145,24);
  ypos+=26;

  gui_stream_genre_label->setGeometry(10,ypos,110,24);
  gui_stream_genre_edit->setGeometry(125,ypos,size().width()-145,24);
  ypos+=26;

  gui_stream_icq_label->setGeometry(10,ypos,110,24);
  gui_stream_icq_edit->setGeometry(125,ypos,size().width()-145,24);
  ypos+=26;

  gui_stream_aim_label->setGeometry(10,ypos,110,24);
  gui_stream_aim_edit->setGeometry(125,ypos,size().width()-145,24);
  ypos+=26;

  gui_stream_irc_label->setGeometry(10,ypos,110,24);
  gui_stream_irc_edit->setGeometry(125,ypos,size().width()-145,24);
  ypos+=26;

  gui_stream_timestamp_offset_label->setGeometry(10,ypos,160,24);
  gui_stream_timestamp_offset_spin->setGeometry(175,ypos,60,24);
  gui_stream_timestamp_offset_unit->setGeometry(240,ypos,60,24);
  ypos+=35;

  gui_close_button->setGeometry(size().width()-80,size().height()-50,70,40);
}
