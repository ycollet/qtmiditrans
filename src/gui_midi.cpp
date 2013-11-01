//=============================================================================
//  QtMidiTrans
//  Qt Jack Midi Trans
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

#include <QtGui>
#include <QtWebKit/QWebView>

#include <sstream>
#include <iostream>

#include "gui_midi.h"
#include "help_dialog.h"

#define CONVERT_ERROR(x)						\
  if (x)								\
    {									\
      std::cerr << qPrintable(QT_TRANSLATE_NOOP("gui_midi", "Error while reading the configuration file.")) << std::endl; \
      return false;							\
    }

#ifdef WINDOWS
#  include <direct.h>
#  define GetCurrentDir _getcwd
#else
#  include <unistd.h>
#  define GetCurrentDir getcwd
#endif

Gui_Midi::Gui_Midi(QMainWindow *parent) : QMainWindow(parent)
{
  jtrans_start_type  = 0;
  jtrans_start_chan  = 0;
  jtrans_start_pitch = -1;
  jtrans_stop_type   = 0;
  jtrans_stop_chan   = 0;
  jtrans_stop_pitch  = -1;
  
  start_learn_mode = false;
  stop_learn_mode  = false;
  learn_type  = -1;
  learn_chan  = -1;
  learn_pitch = -1;
  
  // Define the GUI
  
  setWindowTitle("QtMidiTrans");
  setWindowIcon(QIcon(":/images/MainIcon"));

  // Initialize actions

  aboutAction = new QAction(tr("About &Qt"), parent);
  aboutAction->setStatusTip(tr("Show the Qt library's About box"));
  connect(aboutAction, SIGNAL(triggered()), this, SLOT(about_dialog()));

  exitAction = new QAction(tr("Exit"), parent);
  exitAction->setStatusTip(tr("Exit"));
  connect(exitAction, SIGNAL(triggered()), this, SLOT(close()));

  fileOpenAction = new QAction(tr("&Open"), parent);
  fileOpenAction->setStatusTip(tr("Open a configuration file"));
  connect(fileOpenAction, SIGNAL(triggered()), this, SLOT(open_File()));

  fileSaveAsAction = new QAction(tr("&Save as"), parent);
  fileSaveAsAction->setStatusTip(tr("Save a configuration file as"));
  connect(fileSaveAsAction, SIGNAL(triggered()), this, SLOT(save_File()));

  // Create Menus

  fileMenu = menuBar()->addMenu(tr("&File"));
  fileMenu->addAction(fileOpenAction);
  fileMenu->addAction(fileSaveAsAction);
  fileMenu->addAction(exitAction);

  menuBar()->addSeparator();

  aboutMenu = menuBar()->addMenu(tr("&About"));
  aboutMenu->addAction(aboutAction);

  setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

  buildDialog(parent);
}

void Gui_Midi::buildDialog(QMainWindow *parent)
{  
  // Create the dialog
  QGroupBox* startGroupBox = new QGroupBox(tr("Start"), parent);
  QGroupBox* stopGroupBox  = new QGroupBox(tr("Stop"), parent);
    
  QLabel* startTypeLabel  = new QLabel(tr("Midi Type:"), parent);
  QLabel* startChanLabel  = new QLabel(tr("Chan Type:"), parent);
  QLabel* startPitchLabel = new QLabel(tr("Pitch Type:"), parent);
  QLabel* stopTypeLabel   = new QLabel(tr("Midi Type:"), parent);
  QLabel* stopChanLabel   = new QLabel(tr("Chan Type:"), parent);
  QLabel* stopPitchLabel  = new QLabel(tr("Pitch Type:"), parent);

  startTypeLE  = new QLineEdit(parent); startTypeLE->setText(QString::number(jtrans_start_type));
  startChanLE  = new QLineEdit(parent); startChanLE->setText(QString::number(jtrans_start_chan));
  startPitchLE = new QLineEdit(parent); startPitchLE->setText(QString::number(jtrans_start_pitch));
  stopTypeLE   = new QLineEdit(parent); stopTypeLE->setText(QString::number(jtrans_stop_type));
  stopChanLE   = new QLineEdit(parent); stopChanLE->setText(QString::number(jtrans_stop_chan));
  stopPitchLE  = new QLineEdit(parent); stopPitchLE->setText(QString::number(jtrans_stop_pitch));

  QPushButton* startTypeApply  = new QPushButton(tr("Apply"), parent);
  QPushButton* startChanApply  = new QPushButton(tr("Apply"), parent);
  QPushButton* startPitchApply = new QPushButton(tr("Apply"), parent);
  QPushButton* stopTypeApply   = new QPushButton(tr("Apply"), parent);
  QPushButton* stopChanApply   = new QPushButton(tr("Apply"), parent);
  QPushButton* stopPitchApply  = new QPushButton(tr("Apply"), parent);

  connect(startTypeApply,  SIGNAL(clicked()), this, SLOT(startTypeChanged()));
  connect(startChanApply,  SIGNAL(clicked()), this, SLOT(startChanChanged()));
  connect(startPitchApply, SIGNAL(clicked()), this, SLOT(startPitchChanged()));
  connect(stopTypeApply,   SIGNAL(clicked()), this, SLOT(stopTypeChanged()));
  connect(stopChanApply,   SIGNAL(clicked()), this, SLOT(stopChanChanged()));
  connect(stopPitchApply,  SIGNAL(clicked()), this, SLOT(stopPitchChanged()));

  QPushButton* startLearnButton = new QPushButton(tr("Learn"), parent);
  QPushButton* stopLearnButton  = new QPushButton(tr("Learn"), parent);
  
  connect(startLearnButton, SIGNAL(clicked()), this, SLOT(startLearnChanged()));
  connect(stopLearnButton,  SIGNAL(clicked()), this, SLOT(stopLearnChanged()));
  
  QHBoxLayout* startTypeLayout = new QHBoxLayout(parent);
  QWidget* startTypeWidget = new QWidget(parent);
  startTypeLayout->addWidget(startTypeLabel);
  startTypeLayout->addWidget(startTypeLE);
  startTypeLayout->addWidget(startTypeApply);
  startTypeWidget->setLayout(startTypeLayout);

  QHBoxLayout* startChanLayout = new QHBoxLayout(parent);
  QWidget* startChanWidget = new QWidget(parent);
  startChanLayout->addWidget(startChanLabel);
  startChanLayout->addWidget(startChanLE);
  startChanLayout->addWidget(startChanApply);
  startChanWidget->setLayout(startChanLayout);

  QHBoxLayout* startPitchLayout = new QHBoxLayout(parent);
  QWidget* startPitchWidget = new QWidget(parent);
  startPitchLayout->addWidget(startPitchLabel);
  startPitchLayout->addWidget(startPitchLE);
  startPitchLayout->addWidget(startPitchApply);
  startPitchWidget->setLayout(startPitchLayout);

  QHBoxLayout* stopTypeLayout = new QHBoxLayout(parent);
  QWidget* stopTypeWidget = new QWidget(parent);
  stopTypeLayout->addWidget(stopTypeLabel);
  stopTypeLayout->addWidget(stopTypeLE);
  stopTypeLayout->addWidget(stopTypeApply);
  stopTypeWidget->setLayout(stopTypeLayout);

  QHBoxLayout* stopChanLayout = new QHBoxLayout(parent);
  QWidget* stopChanWidget = new QWidget(parent);
  stopChanLayout->addWidget(stopChanLabel);
  stopChanLayout->addWidget(stopChanLE);
  stopChanLayout->addWidget(stopChanApply);
  stopChanWidget->setLayout(stopChanLayout);

  QHBoxLayout* stopPitchLayout = new QHBoxLayout(parent);
  QWidget* stopPitchWidget = new QWidget(parent);
  stopPitchLayout->addWidget(stopPitchLabel);
  stopPitchLayout->addWidget(stopPitchLE);
  stopPitchLayout->addWidget(stopPitchApply);
  stopPitchWidget->setLayout(stopPitchLayout);

  QVBoxLayout *startLayout = new QVBoxLayout(parent);
  startLayout->addWidget(startTypeWidget);
  startLayout->addWidget(startChanWidget);
  startLayout->addWidget(startPitchWidget);
  startLayout->addWidget(startLearnButton);
  startGroupBox->setLayout(startLayout);
  
  QVBoxLayout *stopLayout = new QVBoxLayout(parent);
  stopLayout->addWidget(stopTypeWidget);
  stopLayout->addWidget(stopChanWidget);
  stopLayout->addWidget(stopPitchWidget);
  stopLayout->addWidget(stopLearnButton);
  stopGroupBox->setLayout(stopLayout);

  QPushButton *exitButton = new QPushButton(tr("Exit"), parent);
  connect(exitButton, SIGNAL(clicked()), this, SLOT(close()));
  
  QVBoxLayout *dialogLayout = new QVBoxLayout(parent);
  QWidget* dialogWidget = new QWidget(parent);
  dialogLayout->addWidget(startGroupBox);
  dialogLayout->addWidget(stopGroupBox);
  dialogLayout->addWidget(exitButton);
  
  dialogWidget->setLayout(dialogLayout);
  
  dialogWidget->resize(dialogWidget->minimumWidth(),dialogWidget->height());
  
  this->setCentralWidget(dialogWidget);
  this->adjustSize();
  this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
  this->setFocusPolicy(Qt::ClickFocus);
  this->show();
}

int Gui_Midi::about_dialog()
{
  HelpDialog * helpDialog = new HelpDialog(this);
  helpDialog->show();
}

void Gui_Midi::startTypeChanged()
{
  jtrans_start_type = startTypeLE->text().toInt();
  qDebug() << "DEBUG: jtrans_start_type = " << startTypeLE->text();
}

void Gui_Midi::startChanChanged()
{
  jtrans_start_chan = startChanLE->text().toInt();
  qDebug() << "DEBUG: jtrans_start_chan = " << startChanLE->text();
}

void Gui_Midi::startPitchChanged()
{
  jtrans_start_pitch = startPitchLE->text().toInt();
  qDebug() << "DEBUG: jtrans_start_pitch = " << startPitchLE->text();
}

void Gui_Midi::stopTypeChanged()
{
  jtrans_stop_type = stopTypeLE->text().toInt();
  qDebug() << "DEBUG: jtrans_stop_type = " << stopTypeLE->text();
}

void Gui_Midi::stopChanChanged()
{
  jtrans_stop_chan = stopChanLE->text().toInt();
  qDebug() << "DEBUG: jtrans_stop_chan = " << stopChanLE->text();
}

void Gui_Midi::stopPitchChanged()
{
  jtrans_stop_pitch = stopPitchLE->text().toInt();
  qDebug() << "DEBUG: jtrans_stop_pitch = " << stopPitchLE->text();
}

void Gui_Midi::startLearnChanged()
{
  start_learn_mode = !start_learn_mode; // We toggle between modes
  startTypeLE->setText(QString::number(learn_type));
  startChanLE->setText(QString::number(learn_chan));
  startPitchLE->setText(QString::number(learn_pitch));
}

void Gui_Midi::stopLearnChanged()
{
  stop_learn_mode = !stop_learn_mode; // We toggle between modes
  stopTypeLE->setText(QString::number(learn_type));
  stopChanLE->setText(QString::number(learn_chan));
  stopPitchLE->setText(QString::number(learn_pitch));
}

bool Gui_Midi::load_config_file(QString fileName)
{
  bool convert_result = false;
  QString line;
  QStringList fields;
  
  QFile file(fileName); 
  if (!file.open(QIODevice::ReadOnly))
    {
      std::cerr << qPrintable(tr("Cannot open file for reading: ")) << qPrintable(file.errorString()) << std::endl;
      return false;
    }

  QTextStream in(&file);
  
  line = in.readLine(); fields = line.split(' ');
  jtrans_start_type = fields.at(1).toInt(&convert_result); CONVERT_ERROR(convert_result);

  line = in.readLine(); fields = line.split(' ');
  jtrans_start_chan = fields.at(1).toInt(&convert_result); CONVERT_ERROR(convert_result);

  line = in.readLine(); fields = line.split(' ');
  jtrans_start_pitch = fields.at(1).toInt(&convert_result); CONVERT_ERROR(convert_result);

  line = in.readLine(); fields = line.split(' ');
  jtrans_stop_type = fields.at(1).toInt(&convert_result); CONVERT_ERROR(convert_result);

  line = in.readLine(); fields = line.split(' ');
  jtrans_stop_chan = fields.at(1).toInt(&convert_result); CONVERT_ERROR(convert_result);

  line = in.readLine(); fields = line.split(' ');
  jtrans_stop_pitch = fields.at(1).toInt(&convert_result); CONVERT_ERROR(convert_result);

  file.close();
}

bool Gui_Midi::save_config_file(QString fileName) const
{
  QFile file(fileName); 
  if (!file.open(QIODevice::WriteOnly))
    {
      std::cerr << qPrintable(tr("Cannot open file for writing: ")) << qPrintable(file.errorString()) << std::endl;
      return false;
    }

  QTextStream out(&file);

  out << "start_type "  << jtrans_start_type  << endl;
  out << "start_chan "  << jtrans_start_chan  << endl;
  out << "start_pitch " << jtrans_start_pitch << endl;

  out << "stop_type "  << jtrans_stop_type  << endl;
  out << "stop_chan "  << jtrans_stop_chan  << endl;
  out << "stop_pitch " << jtrans_stop_pitch << endl;

  file.close();
}

void Gui_Midi::open_File()
{
  char cCurrentPath[FILENAME_MAX];

  GetCurrentDir(cCurrentPath, sizeof(cCurrentPath));

  QString fileName = QFileDialog::getOpenFileName(this, tr("Open QtMidi configuration file"), 
						  cCurrentPath,
						  tr("qtmidi config files (*.qtmidi)"));

  if (!fileName.isEmpty()) load_config_file(fileName);
}

void Gui_Midi::save_File()
{
  char cCurrentPath[FILENAME_MAX];

  GetCurrentDir(cCurrentPath, sizeof(cCurrentPath));

  QString fileName = QFileDialog::getSaveFileName(this, tr("Save QtMidi configuration file"), 
						  cCurrentPath,
						  tr("qtmidi config files (*.qtmidi)"));

  if (!fileName.isEmpty()) save_config_file(fileName);
}

int Gui_Midi::getJTransStartType() const
{
  return jtrans_start_type;
}

int Gui_Midi::getJTransStartChan() const
{
  return jtrans_start_chan;
}

int Gui_Midi::getJTransStartPitch() const
{
  return jtrans_start_pitch;
}

int Gui_Midi::getJTransStopType() const
{
  return jtrans_stop_type;
}

int Gui_Midi::getJTransStopChan() const
{
  return jtrans_stop_chan;
}

int Gui_Midi::getJTransStopPitch() const
{
  return jtrans_stop_pitch;
}


void Gui_Midi::setJTransStartType(int val)
{
  jtrans_start_type = val;
}

void Gui_Midi::setJTransStartChan(int val)
{
  jtrans_start_chan = val;
}

void Gui_Midi::setJTransStartPitch(int val)
{
  jtrans_start_pitch = val;
}

void Gui_Midi::setJTransStopType(int val)
{
  jtrans_stop_type = val;
}

void Gui_Midi::setJTransStopChan(int val)
{
  jtrans_stop_chan = val;
}

void Gui_Midi::setJTransStopPitch(int val)
{
  jtrans_stop_pitch = val;
}

bool Gui_Midi::isStartLearnMode() const
{
  return start_learn_mode;
}

bool Gui_Midi::isStopLearnMode() const
{
  return stop_learn_mode;
}

void Gui_Midi::setLearnType(int val)
{
  learn_type = val;
}

void Gui_Midi::setLearnChan(int val)
{
  learn_chan = val;
}

void Gui_Midi::setLearnPitch(int val)
{
  learn_pitch = val;
}

int Gui_Midi::getLearnType() const
{
  return learn_type;
}

int Gui_Midi::getLearnChan() const
{
  return learn_chan;
}

int Gui_Midi::getLearnPitch() const
{
  return learn_pitch;
}

