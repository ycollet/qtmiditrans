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
#include <vector>

class Gui_Midi : public QMainWindow
{
  Q_OBJECT

 public:
  Gui_Midi(QMainWindow * parent = 0);
  void buildDialog(QMainWindow * parent);
  
  bool load_config_file(QString fileName);
  bool save_config_file(QString fileName) const;
  
  int getJTransStartType() const;
  int getJTransStartChan() const;
  int getJTransStartPitch() const;
  int getJTransStopType() const;
  int getJTransStopChan() const;
  int getJTransStopPitch() const;
  
  void setJTransStartType(int val);
  void setJTransStartChan(int val);
  void setJTransStartPitch(int val);
  void setJTransStopType(int val);
  void setJTransStopChan(int val);
  void setJTransStopPitch(int val);

  bool isStartLearnMode() const;
  bool isStopLearnMode() const;
  void setLearnType(int);
  void setLearnChan(int);
  void setLearnPitch(int);
  int getLearnType() const;
  int getLearnChan() const;
  int getLearnPitch() const;
  
 private slots:
  void open_File();
  void save_File();
  int  about_dialog();
  
  void startTypeChanged();
  void startChanChanged();
  void startPitchChanged();
  void stopTypeChanged();
  void stopChanChanged();
  void stopPitchChanged();
  
  void startLearnChanged();
  void stopLearnChanged();
  
 private:
  QMenu   *fileMenu;
  QMenu   *aboutMenu;
  QAction *fileOpenAction;
  QAction *fileSaveAsAction;
  QAction *exitAction;
  QAction *aboutAction;
  
  QLineEdit* startTypeLE;
  QLineEdit* startChanLE;
  QLineEdit* startPitchLE;
  QLineEdit* stopTypeLE;
  QLineEdit* stopChanLE;
  QLineEdit* stopPitchLE;
  
  int jtrans_start_type;
  int jtrans_start_chan;
  int jtrans_start_pitch;
  int jtrans_stop_type;
  int jtrans_stop_chan;
  int jtrans_stop_pitch;
  
  bool start_learn_mode;
  bool stop_learn_mode;
  int learn_type;
  int learn_chan;
  int learn_pitch;
};

#endif
