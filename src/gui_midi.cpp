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
#include <QMap>

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
  jtrans_type[Play]      = 0;
  jtrans_chan[Play]      = 0;
  jtrans_pitch[Play]     = -1;
  jtrans_type[Stop]      = 0;
  jtrans_chan[Stop]      = 0;
  jtrans_pitch[Stop]     = -1;
  jtrans_type[Rewind]    = 0;
  jtrans_chan[Rewind]    = 0;
  jtrans_pitch[Rewind]   = -1;
  jtrans_type[Forward]   = 0;
  jtrans_chan[Forward]   = 0;
  jtrans_pitch[Forward]  = -1;
  jtrans_type[Backward]  = 0;
  jtrans_chan[Backward]  = 0;
  jtrans_pitch[Backward] = -1;
  
  learn_mode[Play]     = false;
  learn_mode[Stop]     = false;
  learn_mode[Rewind]   = false;
  learn_mode[Forward]  = false;
  learn_mode[Backward] = false;

  learn_type  = -1;
  learn_chan  = -1;
  learn_pitch = -1;

  skipAccel = 1.1;
  
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
    
  QLabel* playTypeLabel      = new QLabel(tr("Midi Type:"), parent);
  QLabel* playChanLabel      = new QLabel(tr("Chan Type:"), parent);
  QLabel* playPitchLabel     = new QLabel(tr("Pitch Type:"), parent);
  QLabel* stopTypeLabel      = new QLabel(tr("Midi Type:"), parent);
  QLabel* stopChanLabel      = new QLabel(tr("Chan Type:"), parent);
  QLabel* stopPitchLabel     = new QLabel(tr("Pitch Type:"), parent);
  QLabel* rewindTypeLabel    = new QLabel(tr("Midi Type:"), parent);
  QLabel* rewindChanLabel    = new QLabel(tr("Chan Type:"), parent);
  QLabel* rewindPitchLabel   = new QLabel(tr("Pitch Type:"), parent);
  QLabel* forwardTypeLabel   = new QLabel(tr("Midi Type:"), parent);
  QLabel* forwardChanLabel   = new QLabel(tr("Chan Type:"), parent);
  QLabel* forwardPitchLabel  = new QLabel(tr("Pitch Type:"), parent);
  QLabel* backwardTypeLabel  = new QLabel(tr("Midi Type:"), parent);
  QLabel* backwardChanLabel  = new QLabel(tr("Chan Type:"), parent);
  QLabel* backwardPitchLabel = new QLabel(tr("Pitch Type:"), parent);
  QLabel* skipAccelLabel     = new QLabel(tr("Forward / Backward speed:"), parent);

  TypeLE[Play]      = new QLineEdit(parent); TypeLE[Play]->setText(QString::number(jtrans_type[Play]));
  ChanLE[Play]      = new QLineEdit(parent); ChanLE[Play]->setText(QString::number(jtrans_chan[Play]));
  PitchLE[Play]     = new QLineEdit(parent); PitchLE[Play]->setText(QString::number(jtrans_pitch[Play]));
  TypeLE[Stop]      = new QLineEdit(parent); TypeLE[Stop]->setText(QString::number(jtrans_type[Stop]));
  ChanLE[Stop]      = new QLineEdit(parent); ChanLE[Stop]->setText(QString::number(jtrans_chan[Stop]));
  PitchLE[Stop]     = new QLineEdit(parent); PitchLE[Stop]->setText(QString::number(jtrans_pitch[Stop]));
  TypeLE[Rewind]    = new QLineEdit(parent); TypeLE[Rewind]->setText(QString::number(jtrans_type[Rewind]));
  ChanLE[Rewind]    = new QLineEdit(parent); ChanLE[Rewind]->setText(QString::number(jtrans_chan[Rewind]));
  PitchLE[Rewind]   = new QLineEdit(parent); PitchLE[Rewind]->setText(QString::number(jtrans_pitch[Rewind]));
  TypeLE[Forward]   = new QLineEdit(parent); TypeLE[Forward]->setText(QString::number(jtrans_type[Forward]));
  ChanLE[Forward]   = new QLineEdit(parent); ChanLE[Forward]->setText(QString::number(jtrans_chan[Forward]));
  PitchLE[Forward]  = new QLineEdit(parent); PitchLE[Forward]->setText(QString::number(jtrans_pitch[Forward]));
  TypeLE[Backward]  = new QLineEdit(parent); TypeLE[Backward]->setText(QString::number(jtrans_type[Backward]));
  ChanLE[Backward]  = new QLineEdit(parent); ChanLE[Backward]->setText(QString::number(jtrans_chan[Backward]));
  PitchLE[Backward] = new QLineEdit(parent); PitchLE[Backward]->setText(QString::number(jtrans_pitch[Backward]));
  SkipAccelLE       = new QLineEdit(parent); SkipAccelLE->setText(QString::number(skipAccel));
  
  QPushButton* playTypeApply      = new QPushButton(tr("Apply"), parent);
  QPushButton* playChanApply      = new QPushButton(tr("Apply"), parent);
  QPushButton* playPitchApply     = new QPushButton(tr("Apply"), parent);
  QPushButton* stopTypeApply      = new QPushButton(tr("Apply"), parent);
  QPushButton* stopChanApply      = new QPushButton(tr("Apply"), parent);
  QPushButton* stopPitchApply     = new QPushButton(tr("Apply"), parent);
  QPushButton* rewindTypeApply    = new QPushButton(tr("Apply"), parent);
  QPushButton* rewindChanApply    = new QPushButton(tr("Apply"), parent);
  QPushButton* rewindPitchApply   = new QPushButton(tr("Apply"), parent);
  QPushButton* forwardTypeApply   = new QPushButton(tr("Apply"), parent);
  QPushButton* forwardChanApply   = new QPushButton(tr("Apply"), parent);
  QPushButton* forwardPitchApply  = new QPushButton(tr("Apply"), parent);
  QPushButton* backwardTypeApply  = new QPushButton(tr("Apply"), parent);
  QPushButton* backwardChanApply  = new QPushButton(tr("Apply"), parent);
  QPushButton* backwardPitchApply = new QPushButton(tr("Apply"), parent);
  QPushButton* skipAccelApply     = new QPushButton(tr("Apply"), parent);

  connect(playTypeApply,      SIGNAL(clicked()), this, SLOT(playTypeChanged()));
  connect(playChanApply,      SIGNAL(clicked()), this, SLOT(playChanChanged()));
  connect(playPitchApply,     SIGNAL(clicked()), this, SLOT(playPitchChanged()));
  connect(stopTypeApply,      SIGNAL(clicked()), this, SLOT(stopTypeChanged()));
  connect(stopChanApply,      SIGNAL(clicked()), this, SLOT(stopChanChanged()));
  connect(stopPitchApply,     SIGNAL(clicked()), this, SLOT(stopPitchChanged()));
  connect(rewindTypeApply,    SIGNAL(clicked()), this, SLOT(rewindTypeChanged()));
  connect(rewindChanApply,    SIGNAL(clicked()), this, SLOT(rewindChanChanged()));
  connect(rewindPitchApply,   SIGNAL(clicked()), this, SLOT(rewindPitchChanged()));
  connect(forwardTypeApply,   SIGNAL(clicked()), this, SLOT(forwardTypeChanged()));
  connect(forwardChanApply,   SIGNAL(clicked()), this, SLOT(forwardChanChanged()));
  connect(forwardPitchApply,  SIGNAL(clicked()), this, SLOT(forwardPitchChanged()));
  connect(backwardTypeApply,  SIGNAL(clicked()), this, SLOT(backwardTypeChanged()));
  connect(backwardChanApply,  SIGNAL(clicked()), this, SLOT(backwardChanChanged()));
  connect(backwardPitchApply, SIGNAL(clicked()), this, SLOT(backwardPitchChanged()));
  connect(skipAccelApply,     SIGNAL(clicked()), this, SLOT(skipAccelChanged()));
  
  QPushButton* playLearnButton      = new QPushButton(tr("Learn"), parent);
  QPushButton* stopLearnButton      = new QPushButton(tr("Learn"), parent);
  QPushButton* rewindLearnButton    = new QPushButton(tr("Learn"), parent);
  QPushButton* forwardLearnButton   = new QPushButton(tr("Learn"), parent);
  QPushButton* backwardLearnButton  = new QPushButton(tr("Learn"), parent);
  
  connect(playLearnButton,     SIGNAL(clicked()), this, SLOT(playLearnChanged()));
  connect(stopLearnButton,     SIGNAL(clicked()), this, SLOT(stopLearnChanged()));
  connect(rewindLearnButton,   SIGNAL(clicked()), this, SLOT(rewindLearnChanged()));
  connect(forwardLearnButton,  SIGNAL(clicked()), this, SLOT(forwardLearnChanged()));
  connect(backwardLearnButton, SIGNAL(clicked()), this, SLOT(backwardLearnChanged()));
  
  // Play part
  
  QHBoxLayout* playTypeLayout = new QHBoxLayout(parent);
  QWidget* playTypeWidget = new QWidget(parent);
  playTypeLayout->addWidget(playTypeLabel);
  playTypeLayout->addWidget(TypeLE[Play]);
  playTypeLayout->addWidget(playTypeApply);
  playTypeWidget->setLayout(playTypeLayout);

  QHBoxLayout* playChanLayout = new QHBoxLayout(parent);
  QWidget* playChanWidget = new QWidget(parent);
  playChanLayout->addWidget(playChanLabel);
  playChanLayout->addWidget(ChanLE[Play]);
  playChanLayout->addWidget(playChanApply);
  playChanWidget->setLayout(playChanLayout);

  QHBoxLayout* playPitchLayout = new QHBoxLayout(parent);
  QWidget* playPitchWidget = new QWidget(parent);
  playPitchLayout->addWidget(playPitchLabel);
  playPitchLayout->addWidget(PitchLE[Play]);
  playPitchLayout->addWidget(playPitchApply);
  playPitchWidget->setLayout(playPitchLayout);
  
  // Stop part
  
  QHBoxLayout* stopTypeLayout = new QHBoxLayout(parent);
  QWidget* stopTypeWidget = new QWidget(parent);
  stopTypeLayout->addWidget(stopTypeLabel);
  stopTypeLayout->addWidget(TypeLE[Stop]);
  stopTypeLayout->addWidget(stopTypeApply);
  stopTypeWidget->setLayout(stopTypeLayout);

  QHBoxLayout* stopChanLayout = new QHBoxLayout(parent);
  QWidget* stopChanWidget = new QWidget(parent);
  stopChanLayout->addWidget(stopChanLabel);
  stopChanLayout->addWidget(ChanLE[Stop]);
  stopChanLayout->addWidget(stopChanApply);
  stopChanWidget->setLayout(stopChanLayout);

  QHBoxLayout* stopPitchLayout = new QHBoxLayout(parent);
  QWidget* stopPitchWidget = new QWidget(parent);
  stopPitchLayout->addWidget(stopPitchLabel);
  stopPitchLayout->addWidget(PitchLE[Stop]);
  stopPitchLayout->addWidget(stopPitchApply);
  stopPitchWidget->setLayout(stopPitchLayout);

  // Rewind part

  QHBoxLayout* rewindTypeLayout = new QHBoxLayout(parent);
  QWidget* rewindTypeWidget = new QWidget(parent);
  rewindTypeLayout->addWidget(rewindTypeLabel);
  rewindTypeLayout->addWidget(TypeLE[Rewind]);
  rewindTypeLayout->addWidget(rewindTypeApply);
  rewindTypeWidget->setLayout(rewindTypeLayout);

  QHBoxLayout* rewindChanLayout = new QHBoxLayout(parent);
  QWidget* rewindChanWidget = new QWidget(parent);
  rewindChanLayout->addWidget(rewindChanLabel);
  rewindChanLayout->addWidget(ChanLE[Rewind]);
  rewindChanLayout->addWidget(rewindChanApply);
  rewindChanWidget->setLayout(rewindChanLayout);

  QHBoxLayout* rewindPitchLayout = new QHBoxLayout(parent);
  QWidget* rewindPitchWidget = new QWidget(parent);
  rewindPitchLayout->addWidget(rewindPitchLabel);
  rewindPitchLayout->addWidget(PitchLE[Rewind]);
  rewindPitchLayout->addWidget(rewindPitchApply);
  rewindPitchWidget->setLayout(rewindPitchLayout);
  
  // Forward part
  
  QHBoxLayout* forwardTypeLayout = new QHBoxLayout(parent);
  QWidget* forwardTypeWidget = new QWidget(parent);
  forwardTypeLayout->addWidget(forwardTypeLabel);
  forwardTypeLayout->addWidget(TypeLE[Forward]);
  forwardTypeLayout->addWidget(forwardTypeApply);
  forwardTypeWidget->setLayout(forwardTypeLayout);

  QHBoxLayout* forwardChanLayout = new QHBoxLayout(parent);
  QWidget* forwardChanWidget = new QWidget(parent);
  forwardChanLayout->addWidget(forwardChanLabel);
  forwardChanLayout->addWidget(ChanLE[Forward]);
  forwardChanLayout->addWidget(forwardChanApply);
  forwardChanWidget->setLayout(forwardChanLayout);

  QHBoxLayout* forwardPitchLayout = new QHBoxLayout(parent);
  QWidget* forwardPitchWidget = new QWidget(parent);
  forwardPitchLayout->addWidget(forwardPitchLabel);
  forwardPitchLayout->addWidget(PitchLE[Forward]);
  forwardPitchLayout->addWidget(forwardPitchApply);
  forwardPitchWidget->setLayout(forwardPitchLayout);
  
  // Backward part
  
  QHBoxLayout* backwardTypeLayout = new QHBoxLayout(parent);
  QWidget* backwardTypeWidget = new QWidget(parent);
  backwardTypeLayout->addWidget(backwardTypeLabel);
  backwardTypeLayout->addWidget(TypeLE[Backward]);
  backwardTypeLayout->addWidget(backwardTypeApply);
  backwardTypeWidget->setLayout(backwardTypeLayout);

  QHBoxLayout* backwardChanLayout = new QHBoxLayout(parent);
  QWidget* backwardChanWidget = new QWidget(parent);
  backwardChanLayout->addWidget(backwardChanLabel);
  backwardChanLayout->addWidget(ChanLE[Backward]);
  backwardChanLayout->addWidget(backwardChanApply);
  backwardChanWidget->setLayout(backwardChanLayout);

  QHBoxLayout* backwardPitchLayout = new QHBoxLayout(parent);
  QWidget* backwardPitchWidget = new QWidget(parent);
  backwardPitchLayout->addWidget(backwardPitchLabel);
  backwardPitchLayout->addWidget(PitchLE[Backward]);
  backwardPitchLayout->addWidget(backwardPitchApply);
  backwardPitchWidget->setLayout(backwardPitchLayout);

  // Paremeters part
  
  QHBoxLayout* skipAccelLayout = new QHBoxLayout(parent);
  QWidget* skipAccelWidget = new QWidget(parent);
  skipAccelLayout->addWidget(skipAccelLabel);
  skipAccelLayout->addWidget(SkipAccelLE);
  skipAccelLayout->addWidget(skipAccelApply);
  skipAccelWidget->setLayout(skipAccelLayout);
  
  // Horizontal pack part
  
  QVBoxLayout *playLayout = new QVBoxLayout(parent);
  QWidget *playWidget = new QWidget(parent);
  playLayout->addWidget(playTypeWidget);
  playLayout->addWidget(playChanWidget);
  playLayout->addWidget(playPitchWidget);
  playLayout->addWidget(playLearnButton);
  playWidget->setLayout(playLayout);
  
  QVBoxLayout *stopLayout = new QVBoxLayout(parent);
  QWidget *stopWidget = new QWidget(parent);
  stopLayout->addWidget(stopTypeWidget);
  stopLayout->addWidget(stopChanWidget);
  stopLayout->addWidget(stopPitchWidget);
  stopLayout->addWidget(stopLearnButton);
  stopWidget->setLayout(stopLayout);

  QVBoxLayout *rewindLayout = new QVBoxLayout(parent);
  QWidget *rewindWidget = new QWidget(parent);
  rewindLayout->addWidget(rewindTypeWidget);
  rewindLayout->addWidget(rewindChanWidget);
  rewindLayout->addWidget(rewindPitchWidget);
  rewindLayout->addWidget(rewindLearnButton);
  rewindWidget->setLayout(rewindLayout);
  
  QVBoxLayout *forwardLayout = new QVBoxLayout(parent);
  QWidget *forwardWidget = new QWidget(parent);
  forwardLayout->addWidget(forwardTypeWidget);
  forwardLayout->addWidget(forwardChanWidget);
  forwardLayout->addWidget(forwardPitchWidget);
  forwardLayout->addWidget(forwardLearnButton);
  forwardWidget->setLayout(forwardLayout);
  
  QVBoxLayout *backwardLayout = new QVBoxLayout(parent);
  QWidget *backwardWidget = new QWidget(parent);
  backwardLayout->addWidget(backwardTypeWidget);
  backwardLayout->addWidget(backwardChanWidget);
  backwardLayout->addWidget(backwardPitchWidget);
  backwardLayout->addWidget(backwardLearnButton);
  backwardWidget->setLayout(backwardLayout);

  QVBoxLayout *parametersLayout = new QVBoxLayout(parent);
  QWidget *parametersWidget = new QWidget(parent);
  parametersLayout->addWidget(skipAccelWidget);
  parametersWidget->setLayout(parametersLayout);
  
  QPushButton *exitButton = new QPushButton(tr("Exit"), parent);
  connect(exitButton, SIGNAL(clicked()), this, SLOT(close()));
  
  QTabWidget *tabWidget = new QTabWidget(parent);
  tabWidget->addTab(playWidget,       QIcon(":/images/Play"),     tr("Play"));
  tabWidget->addTab(stopWidget,       QIcon(":/images/Stop"),     tr("Stop"));
  tabWidget->addTab(rewindWidget,     QIcon(":/images/Rewind"),   tr("Rewind"));
  tabWidget->addTab(forwardWidget,    QIcon(":/images/Forward"),  tr("Forward"));
  tabWidget->addTab(backwardWidget,   QIcon(":/images/Backward"), tr("Backward"));
  tabWidget->addTab(parametersWidget, tr("Parameters"));

  QVBoxLayout* dialogLayout = new QVBoxLayout(parent);
  QWidget* dialogWidget = new QWidget(parent);
  dialogLayout->addWidget(tabWidget);
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

void Gui_Midi::playTypeChanged()
{
  jtrans_type[Play] = TypeLE[Play]->text().toInt();
  qDebug() << "DEBUG: jtrans_type[Play] = " << TypeLE[Play]->text();
}

void Gui_Midi::playChanChanged()
{
  jtrans_chan[Play] = ChanLE[Play]->text().toInt();
  qDebug() << "DEBUG: jtrans_chan[Play] = " << ChanLE[Play]->text();
}

void Gui_Midi::playPitchChanged()
{
  jtrans_pitch[Play] = PitchLE[Play]->text().toInt();
  qDebug() << "DEBUG: jtrans_pitch[Play] = " << PitchLE[Play]->text();
}

void Gui_Midi::stopTypeChanged()
{
  jtrans_type[Stop] = TypeLE[Stop]->text().toInt();
  qDebug() << "DEBUG: jtrans_type[Stop] = " << TypeLE[Stop]->text();
}

void Gui_Midi::stopChanChanged()
{
  jtrans_chan[Stop] = ChanLE[Stop]->text().toInt();
  qDebug() << "DEBUG: jtrans_chan[Stop] = " << ChanLE[Stop]->text();
}

void Gui_Midi::stopPitchChanged()
{
  jtrans_pitch[Stop] = PitchLE[Stop]->text().toInt();
  qDebug() << "DEBUG: jtrans_pitch[Stop] = " << PitchLE[Stop]->text();
}

void Gui_Midi::rewindTypeChanged()
{
  jtrans_type[Rewind] = TypeLE[Rewind]->text().toInt();
  qDebug() << "DEBUG: jtrans_type[Rewind] = " << TypeLE[Rewind]->text();
}

void Gui_Midi::rewindChanChanged()
{
  jtrans_chan[Rewind] = ChanLE[Rewind]->text().toInt();
  qDebug() << "DEBUG: jtrans_chan[Rewind] = " << ChanLE[Rewind]->text();
}

void Gui_Midi::rewindPitchChanged()
{
  jtrans_pitch[Rewind] = PitchLE[Rewind]->text().toInt();
  qDebug() << "DEBUG: jtrans_pitch[Rewind] = " << PitchLE[Rewind]->text();
}

void Gui_Midi::forwardTypeChanged()
{
  jtrans_type[Forward] = TypeLE[Forward]->text().toInt();
  qDebug() << "DEBUG: jtrans_type[Forward] = " << TypeLE[Forward]->text();
}

void Gui_Midi::forwardChanChanged()
{
  jtrans_chan[Forward] = ChanLE[Forward]->text().toInt();
  qDebug() << "DEBUG: jtrans_chan[Forward] = " << ChanLE[Forward]->text();
}

void Gui_Midi::forwardPitchChanged()
{
  jtrans_pitch[Forward] = PitchLE[Forward]->text().toInt();
  qDebug() << "DEBUG: jtrans_pitch[Forward] = " << PitchLE[Forward]->text();
}

void Gui_Midi::backwardTypeChanged()
{
  jtrans_type[Backward] = TypeLE[Backward]->text().toInt();
  qDebug() << "DEBUG: jtrans_type[Backward] = " << TypeLE[Backward]->text();
}

void Gui_Midi::backwardChanChanged()
{
  jtrans_chan[Backward] = ChanLE[Backward]->text().toInt();
  qDebug() << "DEBUG: jtrans_chan[Backward] = " << ChanLE[Backward]->text();
}

void Gui_Midi::backwardPitchChanged()
{
  jtrans_pitch[Backward] = PitchLE[Backward]->text().toInt();
  qDebug() << "DEBUG: jtrans_pitch[Backward] = " << PitchLE[Backward]->text();
}

void Gui_Midi::playLearnChanged()
{
  learn_mode[Play] = !learn_mode[Play]; // We toggle between modes
  TypeLE[Play]->setText(QString::number(learn_type));
  ChanLE[Play]->setText(QString::number(learn_chan));
  PitchLE[Play]->setText(QString::number(learn_pitch));
}

void Gui_Midi::stopLearnChanged()
{
  learn_mode[Stop] = !learn_mode[Stop]; // We toggle between modes
  TypeLE[Stop]->setText(QString::number(learn_type));
  ChanLE[Stop]->setText(QString::number(learn_chan));
  PitchLE[Stop]->setText(QString::number(learn_pitch));
}

void Gui_Midi::rewindLearnChanged()
{
  learn_mode[Rewind] = !learn_mode[Rewind]; // We toggle between modes
  TypeLE[Rewind]->setText(QString::number(learn_type));
  ChanLE[Rewind]->setText(QString::number(learn_chan));
  PitchLE[Rewind]->setText(QString::number(learn_pitch));
}

void Gui_Midi::forwardLearnChanged()
{
  learn_mode[Forward] = !learn_mode[Forward]; // We toggle between modes
  TypeLE[Forward]->setText(QString::number(learn_type));
  ChanLE[Forward]->setText(QString::number(learn_chan));
  PitchLE[Forward]->setText(QString::number(learn_pitch));
}

void Gui_Midi::backwardLearnChanged()
{
  learn_mode[Backward] = !learn_mode[Backward]; // We toggle between modes
  TypeLE[Backward]->setText(QString::number(learn_type));
  ChanLE[Backward]->setText(QString::number(learn_chan));
  PitchLE[Backward]->setText(QString::number(learn_pitch));
}

void Gui_Midi::skipAccelChanged()
{
  skipAccel = SkipAccelLE->text().toInt();
  qDebug() << "DEBUG: skipAccel = " << SkipAccelLE->text();
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
  jtrans_type[Play] = fields.at(1).toInt(&convert_result); CONVERT_ERROR(convert_result);

  line = in.readLine(); fields = line.split(' ');
  jtrans_chan[Play] = fields.at(1).toInt(&convert_result); CONVERT_ERROR(convert_result);

  line = in.readLine(); fields = line.split(' ');
  jtrans_pitch[Play] = fields.at(1).toInt(&convert_result); CONVERT_ERROR(convert_result);

  line = in.readLine(); fields = line.split(' ');
  jtrans_type[Stop] = fields.at(1).toInt(&convert_result); CONVERT_ERROR(convert_result);

  line = in.readLine(); fields = line.split(' ');
  jtrans_chan[Stop] = fields.at(1).toInt(&convert_result); CONVERT_ERROR(convert_result);

  line = in.readLine(); fields = line.split(' ');
  jtrans_pitch[Stop] = fields.at(1).toInt(&convert_result); CONVERT_ERROR(convert_result);
  
  line = in.readLine(); fields = line.split(' ');
  jtrans_type[Rewind] = fields.at(1).toInt(&convert_result); CONVERT_ERROR(convert_result);

  line = in.readLine(); fields = line.split(' ');
  jtrans_chan[Rewind] = fields.at(1).toInt(&convert_result); CONVERT_ERROR(convert_result);

  line = in.readLine(); fields = line.split(' ');
  jtrans_pitch[Rewind] = fields.at(1).toInt(&convert_result); CONVERT_ERROR(convert_result);
  
  line = in.readLine(); fields = line.split(' ');
  jtrans_type[Forward] = fields.at(1).toInt(&convert_result); CONVERT_ERROR(convert_result);

  line = in.readLine(); fields = line.split(' ');
  jtrans_chan[Forward] = fields.at(1).toInt(&convert_result); CONVERT_ERROR(convert_result);

  line = in.readLine(); fields = line.split(' ');
  jtrans_pitch[Forward] = fields.at(1).toInt(&convert_result); CONVERT_ERROR(convert_result);
  
  line = in.readLine(); fields = line.split(' ');
  jtrans_type[Backward] = fields.at(1).toInt(&convert_result); CONVERT_ERROR(convert_result);

  line = in.readLine(); fields = line.split(' ');
  jtrans_chan[Backward] = fields.at(1).toInt(&convert_result); CONVERT_ERROR(convert_result);

  line = in.readLine(); fields = line.split(' ');
  jtrans_pitch[Backward] = fields.at(1).toInt(&convert_result); CONVERT_ERROR(convert_result);
  
  line = in.readLine(); fields = line.split(' ');
  skipAccel = fields.at(1).toFloat(&convert_result); CONVERT_ERROR(convert_result);
  
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
  
  out << "play_type "  << jtrans_type[Play]  << endl;
  out << "play_chan "  << jtrans_chan[Play]  << endl;
  out << "play_pitch " << jtrans_pitch[Play] << endl;
  
  out << "stop_type "  << jtrans_type[Stop]  << endl;
  out << "stop_chan "  << jtrans_chan[Stop]  << endl;
  out << "stop_pitch " << jtrans_pitch[Stop] << endl;
  
  out << "rewind_type "  << jtrans_type[Rewind]  << endl;
  out << "rewind_chan "  << jtrans_chan[Rewind]  << endl;
  out << "rewind_pitch " << jtrans_pitch[Rewind] << endl;
  
  out << "forward_type "  << jtrans_type[Forward]  << endl;
  out << "forward_chan "  << jtrans_chan[Forward]  << endl;
  out << "forward_pitch " << jtrans_pitch[Forward] << endl;
  
  out << "backward_type "  << jtrans_type[Backward]  << endl;
  out << "backward_chan "  << jtrans_chan[Backward]  << endl;
  out << "backward_pitch " << jtrans_pitch[Backward] << endl;

  out << "skip_accel " << skipAccel << endl;
  
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

int Gui_Midi::getJTransType(button_type btype) const
{
  return jtrans_type[btype];
}

int Gui_Midi::getJTransChan(button_type btype) const
{
  return jtrans_chan[btype];
}

int Gui_Midi::getJTransPitch(button_type btype) const
{
  return jtrans_pitch[btype];
}

void Gui_Midi::setJTransType(button_type btype, int val)
{
  jtrans_type[btype] = val;
}

void Gui_Midi::setJTransChan(button_type btype, int val)
{
  jtrans_chan[btype] = val;
}

void Gui_Midi::setJTransPitch(button_type btype, int val)
{
  jtrans_pitch[btype] = val;
}

bool Gui_Midi::isLearnMode(button_type btype) const
{
  return learn_mode[btype];
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

float Gui_Midi::getSkipAccel() const
{
  return skipAccel;
}

void Gui_Midi::setSkipAccel(float val)
{
  skipAccel = val;
}

