//   stereometer.h
//
//   A Stereo Audio Meter Widget
//
//   (C) Copyright 2002-2015 Fred Gleason <fredg@paravelsystems.com>
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
//

#ifndef STEREOMETER_H
#define STEREOMETER_H

#include <QtGui/QtGui>

#include "segmeter.h"

#define CLIP_LIGHT_COLOR Qt::red

class StereoMeter : public QWidget
{
 Q_OBJECT
 public:
  StereoMeter(QWidget *parent=0);
  QSize sizeHint() const;
  QSizePolicy sizePolicy() const;
  void setReference(int level);
  void setClipLight(int level);
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
  void setLabel(QString label);
  SegMeter::Mode mode() const;
  void setMode(SegMeter::Mode mode);

 public slots:
  void setLeftSolidBar(int level);
  void setRightSolidBar(int level);
  void setLeftFloatingBar(int level);
  void setRightFloatingBar(int level);
  void setLeftPeakBar(int level);
  void setRightPeakBar(int level);
  void resetClipLight();

 signals:
  void clip();

 protected:
  void paintEvent(QPaintEvent *);

 private:
  SegMeter *left_meter,*right_meter;
  int ref_level;
  int clip_light_level;
  bool clip_light_on;
  int label_x;
  QString meter_label;
  QFont meter_label_font;
  QFont meter_scale_font;
};


#endif  // STEREOMETER_H
