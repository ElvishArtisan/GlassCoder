// playmeter.h
//
//   A playback audio meter widget.
//
//   (C) Copyright 2002-2016 Fred Gleason <fredg@paravelsystems.com>
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Library General Public License 
//   version 2 as published by the Free Software Foundation.
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

#ifndef PLAYMETER_H
#define PLAYMETER_H

#include <QWidget>

#include <segmeter.h>


class PlayMeter : public QWidget
{
 Q_OBJECT
 public:
  PlayMeter(SegMeter::Orientation orient,QWidget *parent=0);
  QSize sizeHint() const;
  QSizePolicy sizePolicy() const;
  void setRange(int min,int max);
  void setDarkLowColor(QColor color);
  void setDarkHighColor(QColor color);
  void setDarkClipColor(QColor color);
  void setLowColor(QColor color);
  void setHighColor(QColor color);
  void setClipColor(QColor color);
  void setHighThreshold(int level);
  void setClipThreshold(int level);
  void setSegmentSize(int size);
  void setSegmentGap(int gap);
  SegMeter::Mode mode() const;
  void setMode(SegMeter::Mode mode);
  void setLabel(QString label);

 public slots:
  void setGeometry(int x,int y,int w,int h);
  void setGeometry(QRect &rect);
  void setSolidBar(int level);
  void setFloatingBar(int level);
  void setPeakBar(int level);

 protected:
  void paintEvent(QPaintEvent *);

 private:
  void makeFont();
  SegMeter *meter;
  QString meter_label;
  QFont label_font;
  SegMeter::Orientation orientation;
  int meter_label_x;
};


#endif  // PLAYMETER_H
