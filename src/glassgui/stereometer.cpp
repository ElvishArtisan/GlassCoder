// stereometer.cpp
//
// This implements a widget that represents a stereo audio level meter,
// complete with labels and scale.
//
// (C) Copyright 2002-2015 Fred Gleason <fredg@paravelsystems.com>
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

#include <stdio.h>

#include "segmeter.h"
#include "stereometer.h"

StereoMeter::StereoMeter(QWidget *parent)
  : QWidget(parent)
{
  ref_level=0;
  clip_light_level=1600;
  clip_light_on=false;
  label_x=0;
  meter_label=QString("");
  left_meter=new SegMeter(SegMeter::Right,this);
  left_meter->setGeometry(25,10,300,10);
  left_meter->setRange(-4600,-800);
  left_meter->setHighThreshold(-1600);
  left_meter->setClipThreshold(-1100);
  left_meter->setSegmentSize(5);
  left_meter->setSegmentGap(1);
  left_meter->setSolidBar(-10000);
  left_meter->setFloatingBar(-10000);
  right_meter=new SegMeter(SegMeter::Right,this);
  right_meter->setGeometry(25,40,300,10);
  right_meter->setRange(-4600,-800);
  right_meter->setHighThreshold(-1600);
  right_meter->setClipThreshold(-1100);
  right_meter->setSegmentSize(5);
  right_meter->setSegmentGap(1);
  right_meter->setSolidBar(-10000);
  right_meter->setFloatingBar(-10000);
  setFixedSize(335,60);

  //
  // Generate Fonts
  //
  meter_label_font=QFont("System",18,QFont::Bold);
  meter_label_font.setPixelSize(18);
  meter_scale_font=QFont("System",12,QFont::Bold);
  meter_scale_font.setPixelSize(12);

  //
  // Set Colors
  // 
  QPalette p=palette();
  p.setColor(QPalette::Window,Qt::black);
  setPalette(p);
}


QSize StereoMeter::sizeHint() const
{
  if(meter_label==QString("")) {
    return QSize(335,60);
  }
  else {
    return QSize(335,80);
  }
}


QSizePolicy StereoMeter::sizePolicy() const
{
  return QSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
}


void StereoMeter::setReference(int level)
{
  ref_level=level;
}


void StereoMeter::setClipLight(int level)
{
  clip_light_level=level-1600+ref_level;
}


void StereoMeter::setDarkLowColor(QColor color)
{
  left_meter->setDarkLowColor(color);
  right_meter->setDarkLowColor(color);
}


void StereoMeter::setDarkHighColor(QColor color)
{
  left_meter->setDarkHighColor(color);
  right_meter->setDarkHighColor(color);
}


void StereoMeter::setDarkClipColor(QColor color)
{
  left_meter->setDarkClipColor(color);
  right_meter->setDarkClipColor(color);
}


void StereoMeter::setLowColor(QColor color)
{
  left_meter->setLowColor(color);
  right_meter->setLowColor(color);
}


void StereoMeter::setHighColor(QColor color)
{
  left_meter->setHighColor(color);
  right_meter->setHighColor(color);
}


void StereoMeter::setClipColor(QColor color)
{
  left_meter->setClipColor(color);
  right_meter->setClipColor(color);
}


void StereoMeter::setHighThreshold(int level)
{
  left_meter->setHighThreshold(level);
  right_meter->setHighThreshold(level);
}


void StereoMeter::setClipThreshold(int level)
{
  left_meter->setClipThreshold(level);
  right_meter->setClipThreshold(level);
}


void StereoMeter::setLabel(QString label)
{
  meter_label=label;
  if(meter_label!=QString("")) {
    QFont meter_font=QFont("System",18,QFont::Normal);
    meter_font.setPixelSize(18);
    QFontMetrics meter_metrics=QFontMetrics(meter_font);
    label_x=(335-meter_metrics.width(meter_label))/2;
    setFixedSize(335,80);
  }
  else {
    setFixedSize(335,60);
  }
}


SegMeter::Mode StereoMeter::mode() const
{
  return left_meter->mode();
}


void StereoMeter::setMode(SegMeter::Mode mode)
{
  left_meter->setMode(mode);
  right_meter->setMode(mode);
}


void StereoMeter::setLeftSolidBar(int level)
{
  left_meter->setSolidBar(level-ref_level);
  if((level>=0)&&(!clip_light_on)) {
    clip_light_on=true;
    emit clip();
    update();
  }
}


void StereoMeter::setRightSolidBar(int level)
{
  right_meter->setSolidBar(level-ref_level);
  if((level>=clip_light_level)&&(!clip_light_on)) {
    clip_light_on=true;
    emit clip();
    update();
  }
}


void StereoMeter::setLeftFloatingBar(int level)
{
  left_meter->setFloatingBar(level-ref_level);
  if((level>=clip_light_level)&&(!clip_light_on)) {
    clip_light_on=true;
    emit clip();
    update();
  }
}


void StereoMeter::setRightFloatingBar(int level)
{
  right_meter->setFloatingBar(level-ref_level);
  if((level>=clip_light_level)&&(!clip_light_on)) {
    clip_light_on=true;
    emit clip();
    update();
  }
}


void StereoMeter::setLeftPeakBar(int level)
{
  left_meter->setPeakBar(level-ref_level);
  if((level>=clip_light_level)&&(!clip_light_on)) {
    clip_light_on=true;
    emit clip();
    update();
  }
}


void StereoMeter::setRightPeakBar(int level)
{
  right_meter->setPeakBar(level-ref_level);
  if((level>=clip_light_level)&&(!clip_light_on)) {
    clip_light_on=true;
    emit clip();
    update();
  }
}


void StereoMeter::resetClipLight()
{
  clip_light_on=false;
  update();
}


void StereoMeter::setSegmentSize(int size)
{
  left_meter->setSegmentSize(size);
  right_meter->setSegmentSize(size);
}


void StereoMeter::setSegmentGap(int gap)
{
  left_meter->setSegmentGap(gap);
  right_meter->setSegmentGap(gap);
}


void StereoMeter::paintEvent(QPaintEvent *paintEvent)
{
  //
  // Setup
  //
  QPixmap pix(this->size());
  pix.fill(this,0,0);
  QPainter *p=new QPainter(&pix);
  p->setBrush(QColor(Qt::white));
  p->setPen(QColor(Qt::white));
  p->setFont(meter_scale_font);
  p->drawText(10,20,tr("L"));
  p->drawText(10,50,tr("R"));
  p->drawText(12,35,"-30");
  p->drawText(48,35,"-25");
  p->drawText(88,35,"-20");
  p->drawText(126,35,"-15");
  p->drawText(167,35,"-10");
  p->drawText(207,35,"-5");
  p->drawText(255,35,"0");
  p->drawText(314,35,"+8");
  if(meter_label!=QString("")) {
    p->setFont(meter_label_font);
    p->drawText(label_x,72,meter_label);
  }
  if(clip_light_on) {
    p->setFont(meter_scale_font);
    p->setPen(QColor(CLIP_LIGHT_COLOR));
    p->drawText(274,35,tr("CLIP"));
  }
  p->end();
  p->begin(this);
  p->drawPixmap(0,0,pix);
  p->end();
  delete p;
}
