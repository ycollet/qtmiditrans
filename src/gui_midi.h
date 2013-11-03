//=============================================================================
//  QtMidiTrans
//  Qt Jack Midi Translater
//  $Id:$
//
//  Copyright (C) 2013 by Yann Collette and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#ifndef GUI_MIDI_H
#define GUI_MIDI_H

#include <QtGui>
#include <QMap>

class Gui_Midi : public QMainWindow
{
  Q_OBJECT

 public:
  enum button_type {Play, Stop, Rewind, Forward, Backward};
  
  Gui_Midi(QMainWindow * parent = 0);
  void buildDialog(QMainWindow * parent);
  
  bool load_config_file(QString fileName);
  bool save_config_file(QString fileName) const;
  
  int getJTransType(button_type btype) const;
  int getJTransChan(button_type btype) const;
  int getJTransPitch(button_type btype) const;
  
  void setJTransType(button_type btype, int val);
  void setJTransChan(button_type btype, int val);
  void setJTransPitch(button_type btype, int val);

  bool isLearnMode(button_type btype) const;
  
  void setLearnType(int);
  void setLearnChan(int);
  void setLearnPitch(int);
  int getLearnType() const;
  int getLearnChan() const;
  int getLearnPitch() const;
  
  float getSkipAccel() const;
  void setSkipAccel(float val);
  
 private slots:
  void open_File();
  void save_File();
  int  about_dialog();
  
  void playChanged();
  void stopChanged();
  void rewindChanged();
  void forwardChanged();
  void backwardChanged();
  void skipAccelChanged();
  
  void playLearnChanged();
  void stopLearnChanged();
  void rewindLearnChanged();
  void forwardLearnChanged();
  void backwardLearnChanged();
  
 private:
  QMenu   *fileMenu;
  QMenu   *aboutMenu;
  QAction *fileOpenAction;
  QAction *fileSaveAsAction;
  QAction *exitAction;
  QAction *aboutAction;
  
  QMap<button_type, QLineEdit*> TypeLE;
  QMap<button_type, QLineEdit*> ChanLE;
  QMap<button_type, QLineEdit*> PitchLE;
  QLineEdit* SkipAccelLE;
  
  QMap<button_type, int> jtrans_type;
  QMap<button_type, int> jtrans_chan;
  QMap<button_type, int> jtrans_pitch;
  
  QMap<button_type, bool> learn_mode;
  
  int learn_type;
  int learn_chan;
  int learn_pitch;

  float skipAccel;
};

#endif
