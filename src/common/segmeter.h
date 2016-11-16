//   segmeter.h
//
//   An audio meter display widget.
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

#ifndef SEGMETER_H
#define SEGMETER_H

#include <QColor>
#include <QPainter>
#include <QPaintEvent>
#include <QSizePolicy>
#include <QTimer>
#include <QWidget>

/*
 * Default Colors
 */
#define DEFAULT_LOW_COLOR Qt::green
#define DEFAULT_DARK_LOW_COLOR 0,80,0
#define DEFAULT_HIGH_COLOR Qt::yellow
#define DEFAULT_DARK_HIGH_COLOR 75,75,0
#define DEFAULT_CLIP_COLOR Qt::red
#define DEFAULT_DARK_CLIP_COLOR 85,0,0

/*
 * Global Settings
 */
#define PEAK_HOLD_TIME 750

/**
 * @short An audio level meter
 * @author Fred Gleason <fredg@wava.com>
 *
 * This class implements an audio level meter, styled after the traditional
 * LED based meters found on many types of audio gear.  The meter can
 * be subdivided into up to three different color bands, calibrated to
 * correspond to various audio threshold levels.
 **/
class SegMeter : public QWidget
{
 Q_OBJECT
 public:
 /**
  * Meter modes.
  * Independent - Operation of the solid and floating bar segments are 
  *               completely independent of each other.
  *
  * Peak - The solid bar shows current level, while the floating bar shows 
  *        the peak level achieved over the previous half second.
  **/
  enum Mode {Independent=0,Peak=1};
  enum Orientation {Left=0,Right=1,Up=2,Down=3};
 /**
  * Instantiates the widget.
  * @param o The orientation of the meter, meaning the direction of 
  * increasing signal level on the meter.
  **/
  SegMeter(SegMeter::Orientation o,QWidget *parent=0);

 /**
  * Returns a suggested size for the meter.
  **/
  QSize sizeHint() const;

  /**
   * Returns the sizing policy of the meter.  This widget can be freely
   * resized to fit the requiements of the calling application.
   */
  QSizePolicy sizePolicy() const;

  /** 
   * Set the range of the audio meter.
   * @param min The minimum value which will register on the meter.
   * @param max The "full scale" value of the meter.
   **/
  void setRange(int min,int max);

  /**
   * Set the color of the "dark" (i.e. non-lit) segments of the low level
   * range on the meter.
   * @param color Color of the dark segments
   **/
  void setDarkLowColor(QColor color);

  /**
   * Set the color of the "dark" (i.e. non-lit) segments of the high level
   * range on the meter.
   * @param color Color of the dark segments
   **/
  void setDarkHighColor(QColor color);

  /**
   * Set the color of the "dark" (i.e. non-lit) segments of the clip level
   * range on the meter.
   * @param color Color of the dark segments
   **/
  void setDarkClipColor(QColor color);

  /**
   * Set the color of the first level range segments on the meter.
   * @param color Color of the first level range segments
   **/
  void setLowColor(QColor color);

  /**
   * Set the color of the second level range segments on the meter.
   * @param color Color of the second level range segments
   **/
  void setHighColor(QColor color);

  /**
   * Set the color of the third level range segments on the meter.
   * @param color Color of the third level range segments
   **/
  void setClipColor(QColor color);

  /**
   * Set the transition point between the first and second range segments.
   * @param level Transition point between first and second range segments.
   **/
  void setHighThreshold(int level);

  /**
   * Set the transition point between the second and third range segments.
   * @param level Transition point between second and third range segments.
   **/
  void setClipThreshold(int level);

  /**
   * Set the thickness of the segments in the meter.
   * @param size Thickness of segments in meter.
   **/
  void setSegmentSize(int size);

  /**
   * Set the size of the gap between segments in the meter.
   * @param size Thickness of gap in meter.
   **/
  void setSegmentGap(int gap);

  /**
   * Get the current meter mode
   **/
  SegMeter::Mode mode() const;

  /**
   * Set the meter mode.
   * @param mode = The mode to set
   **/
  void setMode(SegMeter::Mode mode);

 public slots:
   /**
    * Set the level of the "solid" display on the meter.
    * @param level Level of "solid" display on the meter.
    **/
  void setSolidBar(int level);

   /**
    * Set the level of the "floating" display on the meter.
    * @param level Level of "floating" display on the meter.
    **/
  void setFloatingBar(int level);

   /**
    * Set the level of the peak display on the meter.  This is only valid
    * when the meter is in "peak" mode.
    * @param level Level of "floating" display on the meter.
    **/
  void setPeakBar(int level);

 protected:
  void paintEvent(QPaintEvent *);

 private slots:
  void peakData();

 private:
  SegMeter::Orientation orient;
  SegMeter::Mode seg_mode;
  QTimer *peak_timer;
  int range_min,range_max;
  QColor dark_low_color;
  QColor dark_high_color;
  QColor dark_clip_color;
  QColor low_color;
  QColor high_color;
  QColor clip_color;
  int high_threshold,clip_threshold;
  int solid_bar,floating_bar;
  int seg_size,seg_gap;
};


#endif  // SEGMETER_H
