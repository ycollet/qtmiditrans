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
#include <QIntValidator>
#include <QDoubleValidator>
#include <QTimer>

#include <sstream>
#include <iostream>

#include "gui_midi.h"
#include "help_dialog.h"

#define CONVERT_ERROR(x, message)					\
  if (!x)								\
    {									\
      std::cerr << qPrintable(QT_TRANSLATE_NOOP("gui_midi", "Error while reading the configuration file.")) << " " << message << std::endl; \
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
  midiTimer = new QTimer();
  
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
  
  learn_mode = false;

  learn_type  = -1;
  learn_chan  = -1;
  learn_pitch = -1;

  skipAccel = 1.1;
  timerPoll = 500;
  
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

Gui_Midi::~Gui_Midi()
{
  delete midiTimer;
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
  QLabel* timerPollLabel     = new QLabel(tr("Midi timer polling (ms):"), parent);

  QValidator *midiValidator  = new QIntValidator(0, 127, this);
  QValidator *timerValidator = new QIntValidator(0, 10000, this);
  
  // Get the size of a char
  QFontMetrics metrics(playTypeLabel->font());
  int pixelWidth = metrics.width(QString("0"));

  // Define the LineEdit
  TypeLE[Play] = new QLineEdit(parent); 
  TypeLE[Play]->setText(QString::number(jtrans_type[Play]));
  TypeLE[Play]->setMaxLength(4);
  TypeLE[Play]->setFixedWidth(6*pixelWidth);
  TypeLE[Play]->setValidator(midiValidator);
    
  ChanLE[Play] = new QLineEdit(parent); 
  ChanLE[Play]->setText(QString::number(jtrans_chan[Play]));
  ChanLE[Play]->setMaxLength(4);
  ChanLE[Play]->setFixedWidth(6*pixelWidth);
  ChanLE[Play]->setValidator(midiValidator);
  
  PitchLE[Play] = new QLineEdit(parent); 
  PitchLE[Play]->setText(QString::number(jtrans_pitch[Play]));
  PitchLE[Play]->setMaxLength(4);
  PitchLE[Play]->setFixedWidth(6*pixelWidth);
  PitchLE[Play]->setValidator(midiValidator);
  
  TypeLE[Stop] = new QLineEdit(parent); 
  TypeLE[Stop]->setText(QString::number(jtrans_type[Stop]));
  TypeLE[Stop]->setMaxLength(4);
  TypeLE[Stop]->setFixedWidth(6*pixelWidth);
  TypeLE[Stop]->setValidator(midiValidator);
  
  ChanLE[Stop] = new QLineEdit(parent); 
  ChanLE[Stop]->setText(QString::number(jtrans_chan[Stop]));
  ChanLE[Stop]->setMaxLength(4);
  ChanLE[Stop]->setFixedWidth(6*pixelWidth);
  ChanLE[Stop]->setValidator(midiValidator);
  
  PitchLE[Stop] = new QLineEdit(parent); 
  PitchLE[Stop]->setText(QString::number(jtrans_pitch[Stop]));
  PitchLE[Stop]->setMaxLength(4);
  PitchLE[Stop]->setFixedWidth(6*pixelWidth);
  PitchLE[Stop]->setValidator(midiValidator);
  
  TypeLE[Rewind] = new QLineEdit(parent); 
  TypeLE[Rewind]->setText(QString::number(jtrans_type[Rewind]));
  TypeLE[Rewind]->setMaxLength(4);
  TypeLE[Rewind]->setFixedWidth(6*pixelWidth);
  TypeLE[Rewind]->setValidator(midiValidator);
  
  ChanLE[Rewind] = new QLineEdit(parent); 
  ChanLE[Rewind]->setText(QString::number(jtrans_chan[Rewind]));
  ChanLE[Rewind]->setMaxLength(4);
  ChanLE[Rewind]->setFixedWidth(6*pixelWidth);
  ChanLE[Rewind]->setValidator(midiValidator);
  
  PitchLE[Rewind] = new QLineEdit(parent); 
  PitchLE[Rewind]->setText(QString::number(jtrans_pitch[Rewind]));
  PitchLE[Rewind]->setMaxLength(4);
  PitchLE[Rewind]->setFixedWidth(6*pixelWidth);
  PitchLE[Rewind]->setValidator(midiValidator);
  
  TypeLE[Forward] = new QLineEdit(parent); 
  TypeLE[Forward]->setText(QString::number(jtrans_type[Forward]));
  TypeLE[Forward]->setMaxLength(4);
  TypeLE[Forward]->setFixedWidth(6*pixelWidth);
  TypeLE[Forward]->setValidator(midiValidator);
  
  ChanLE[Forward] = new QLineEdit(parent); 
  ChanLE[Forward]->setText(QString::number(jtrans_chan[Forward]));
  ChanLE[Forward]->setMaxLength(4);
  ChanLE[Forward]->setFixedWidth(6*pixelWidth);
  ChanLE[Forward]->setValidator(midiValidator);
  
  PitchLE[Forward] = new QLineEdit(parent); 
  PitchLE[Forward]->setText(QString::number(jtrans_pitch[Forward]));
  PitchLE[Forward]->setMaxLength(4);
  PitchLE[Forward]->setFixedWidth(6*pixelWidth);
  PitchLE[Forward]->setValidator(midiValidator);
  
  TypeLE[Backward] = new QLineEdit(parent); 
  TypeLE[Backward]->setText(QString::number(jtrans_type[Backward]));
  TypeLE[Backward]->setMaxLength(4);
  TypeLE[Backward]->setFixedWidth(6*pixelWidth);
  TypeLE[Backward]->setValidator(midiValidator);
  
  ChanLE[Backward] = new QLineEdit(parent); 
  ChanLE[Backward]->setText(QString::number(jtrans_chan[Backward]));
  ChanLE[Backward]->setMaxLength(4);
  ChanLE[Backward]->setFixedWidth(6*pixelWidth);
  ChanLE[Backward]->setValidator(midiValidator);
  
  PitchLE[Backward] = new QLineEdit(parent); 
  PitchLE[Backward]->setText(QString::number(jtrans_pitch[Backward]));
  PitchLE[Backward]->setMaxLength(4);
  PitchLE[Backward]->setFixedWidth(6*pixelWidth);
  PitchLE[Backward]->setValidator(midiValidator);
  
  SkipAccelLE = new QLineEdit(parent); 
  QValidator *floatValidator = new QDoubleValidator(1.0, 60.0, 4, this);
  SkipAccelLE->setText(QString::number(skipAccel));
  SkipAccelLE->setMaxLength(6);
  SkipAccelLE->setFixedWidth(8*pixelWidth);
  SkipAccelLE->setValidator(floatValidator);

  TimerPollLE = new QLineEdit(parent); 
  TimerPollLE->setText(QString::number(timerPoll));
  TimerPollLE->setMaxLength(6);
  TimerPollLE->setFixedWidth(8*pixelWidth);
  TimerPollLE->setValidator(timerValidator);
  
  QPushButton* playApply       = new QPushButton(tr("Apply"), parent);
  QPushButton* stopApply       = new QPushButton(tr("Apply"), parent);
  QPushButton* rewindApply     = new QPushButton(tr("Apply"), parent);
  QPushButton* forwardApply    = new QPushButton(tr("Apply"), parent);
  QPushButton* backwardApply   = new QPushButton(tr("Apply"), parent);
  QPushButton* parametersApply = new QPushButton(tr("Apply"), parent);

  connect(playApply,       SIGNAL(clicked()), this, SLOT(playChanged()));
  connect(stopApply,       SIGNAL(clicked()), this, SLOT(stopChanged()));
  connect(rewindApply,     SIGNAL(clicked()), this, SLOT(rewindChanged()));
  connect(forwardApply,    SIGNAL(clicked()), this, SLOT(forwardChanged()));
  connect(backwardApply,   SIGNAL(clicked()), this, SLOT(backwardChanged()));
  connect(parametersApply, SIGNAL(clicked()), this, SLOT(parametersChanged()));
  
  // Play part
  
  QHBoxLayout* playTypeLayout = new QHBoxLayout(parent);
  QWidget* playTypeWidget = new QWidget(parent);
  playTypeLayout->addWidget(playTypeLabel);
  playTypeLayout->addWidget(TypeLE[Play]);
  playTypeWidget->setLayout(playTypeLayout);

  QHBoxLayout* playChanLayout = new QHBoxLayout(parent);
  QWidget* playChanWidget = new QWidget(parent);
  playChanLayout->addWidget(playChanLabel);
  playChanLayout->addWidget(ChanLE[Play]);
  playChanWidget->setLayout(playChanLayout);

  QHBoxLayout* playPitchLayout = new QHBoxLayout(parent);
  QWidget* playPitchWidget = new QWidget(parent);
  playPitchLayout->addWidget(playPitchLabel);
  playPitchLayout->addWidget(PitchLE[Play]);
  playPitchWidget->setLayout(playPitchLayout);
  
  QHBoxLayout* playApplyLayout = new QHBoxLayout(parent);
  QWidget* playApplyWidget = new QWidget(parent);
  playApplyLayout->addStretch(10);
  playApplyLayout->addWidget(playApply);
  playApplyLayout->addStretch(10);
  playApplyWidget->setLayout(playApplyLayout);

  // Stop part
  
  QHBoxLayout* stopTypeLayout = new QHBoxLayout(parent);
  QWidget* stopTypeWidget = new QWidget(parent);
  stopTypeLayout->addWidget(stopTypeLabel);
  stopTypeLayout->addWidget(TypeLE[Stop]);
  stopTypeWidget->setLayout(stopTypeLayout);

  QHBoxLayout* stopChanLayout = new QHBoxLayout(parent);
  QWidget* stopChanWidget = new QWidget(parent);
  stopChanLayout->addWidget(stopChanLabel);
  stopChanLayout->addWidget(ChanLE[Stop]);
  stopChanWidget->setLayout(stopChanLayout);

  QHBoxLayout* stopPitchLayout = new QHBoxLayout(parent);
  QWidget* stopPitchWidget = new QWidget(parent);
  stopPitchLayout->addWidget(stopPitchLabel);
  stopPitchLayout->addWidget(PitchLE[Stop]);
  stopPitchWidget->setLayout(stopPitchLayout);

  // Rewind part

  QHBoxLayout* rewindTypeLayout = new QHBoxLayout(parent);
  QWidget* rewindTypeWidget = new QWidget(parent);
  rewindTypeLayout->addWidget(rewindTypeLabel);
  rewindTypeLayout->addWidget(TypeLE[Rewind]);
  rewindTypeWidget->setLayout(rewindTypeLayout);

  QHBoxLayout* rewindChanLayout = new QHBoxLayout(parent);
  QWidget* rewindChanWidget = new QWidget(parent);
  rewindChanLayout->addWidget(rewindChanLabel);
  rewindChanLayout->addWidget(ChanLE[Rewind]);
  rewindChanWidget->setLayout(rewindChanLayout);

  QHBoxLayout* rewindPitchLayout = new QHBoxLayout(parent);
  QWidget* rewindPitchWidget = new QWidget(parent);
  rewindPitchLayout->addWidget(rewindPitchLabel);
  rewindPitchLayout->addWidget(PitchLE[Rewind]);
  rewindPitchWidget->setLayout(rewindPitchLayout);
  
  // Forward part
  
  QHBoxLayout* forwardTypeLayout = new QHBoxLayout(parent);
  QWidget* forwardTypeWidget = new QWidget(parent);
  forwardTypeLayout->addWidget(forwardTypeLabel);
  forwardTypeLayout->addWidget(TypeLE[Forward]);
  forwardTypeWidget->setLayout(forwardTypeLayout);

  QHBoxLayout* forwardChanLayout = new QHBoxLayout(parent);
  QWidget* forwardChanWidget = new QWidget(parent);
  forwardChanLayout->addWidget(forwardChanLabel);
  forwardChanLayout->addWidget(ChanLE[Forward]);
  forwardChanWidget->setLayout(forwardChanLayout);

  QHBoxLayout* forwardPitchLayout = new QHBoxLayout(parent);
  QWidget* forwardPitchWidget = new QWidget(parent);
  forwardPitchLayout->addWidget(forwardPitchLabel);
  forwardPitchLayout->addWidget(PitchLE[Forward]);
  forwardPitchWidget->setLayout(forwardPitchLayout);
  
  // Backward part
  
  QHBoxLayout* backwardTypeLayout = new QHBoxLayout(parent);
  QWidget* backwardTypeWidget = new QWidget(parent);
  backwardTypeLayout->addWidget(backwardTypeLabel);
  backwardTypeLayout->addWidget(TypeLE[Backward]);
  backwardTypeWidget->setLayout(backwardTypeLayout);

  QHBoxLayout* backwardChanLayout = new QHBoxLayout(parent);
  QWidget* backwardChanWidget = new QWidget(parent);
  backwardChanLayout->addWidget(backwardChanLabel);
  backwardChanLayout->addWidget(ChanLE[Backward]);
  backwardChanWidget->setLayout(backwardChanLayout);

  QHBoxLayout* backwardPitchLayout = new QHBoxLayout(parent);
  QWidget* backwardPitchWidget = new QWidget(parent);
  backwardPitchLayout->addWidget(backwardPitchLabel);
  backwardPitchLayout->addWidget(PitchLE[Backward]);
  backwardPitchWidget->setLayout(backwardPitchLayout);

  // Paremeters part
  
  QHBoxLayout* skipAccelLayout = new QHBoxLayout(parent);
  QWidget* skipAccelWidget = new QWidget(parent);
  skipAccelLayout->addWidget(skipAccelLabel);
  skipAccelLayout->addWidget(SkipAccelLE);
  skipAccelWidget->setLayout(skipAccelLayout);

  QHBoxLayout* timerPollLayout = new QHBoxLayout(parent);
  QWidget* timerPollWidget = new QWidget(parent);
  timerPollLayout->addWidget(timerPollLabel);
  timerPollLayout->addWidget(TimerPollLE);
  timerPollWidget->setLayout(timerPollLayout);
  
  // Horizontal pack part
  
  QVBoxLayout *playLayout = new QVBoxLayout(parent);
  QWidget *playWidget = new QWidget(parent);
  playLayout->addWidget(playTypeWidget,  0, Qt::AlignLeft);
  playLayout->addWidget(playChanWidget,  0, Qt::AlignLeft);
  playLayout->addWidget(playPitchWidget, 0, Qt::AlignLeft);
  playLayout->addWidget(playApply,       0, Qt::AlignHCenter);
  playWidget->setLayout(playLayout);
  
  QVBoxLayout *stopLayout = new QVBoxLayout(parent);
  QWidget *stopWidget = new QWidget(parent);
  stopLayout->addWidget(stopTypeWidget,  0, Qt::AlignLeft);
  stopLayout->addWidget(stopChanWidget,  0, Qt::AlignLeft);
  stopLayout->addWidget(stopPitchWidget, 0, Qt::AlignLeft);
  stopLayout->addWidget(stopApply,       0, Qt::AlignHCenter);
  stopWidget->setLayout(stopLayout);

  QVBoxLayout *rewindLayout = new QVBoxLayout(parent);
  QWidget *rewindWidget = new QWidget(parent);
  rewindLayout->addWidget(rewindTypeWidget,  0, Qt::AlignLeft);
  rewindLayout->addWidget(rewindChanWidget,  0, Qt::AlignLeft);
  rewindLayout->addWidget(rewindPitchWidget, 0, Qt::AlignLeft);
  rewindLayout->addWidget(rewindApply,       0, Qt::AlignHCenter);
  rewindWidget->setLayout(rewindLayout);
  
  QVBoxLayout *forwardLayout = new QVBoxLayout(parent);
  QWidget *forwardWidget = new QWidget(parent);
  forwardLayout->addWidget(forwardTypeWidget,  0, Qt::AlignLeft);
  forwardLayout->addWidget(forwardChanWidget,  0, Qt::AlignLeft);
  forwardLayout->addWidget(forwardPitchWidget, 0, Qt::AlignLeft);
  forwardLayout->addWidget(forwardApply,       0, Qt::AlignHCenter);
  forwardWidget->setLayout(forwardLayout);
  
  QVBoxLayout *backwardLayout = new QVBoxLayout(parent);
  QWidget *backwardWidget = new QWidget(parent);
  backwardLayout->addWidget(backwardTypeWidget,  0, Qt::AlignLeft);
  backwardLayout->addWidget(backwardChanWidget,  0, Qt::AlignLeft);
  backwardLayout->addWidget(backwardPitchWidget, 0, Qt::AlignLeft);
  backwardLayout->addWidget(backwardApply,       0, Qt::AlignHCenter);
  backwardWidget->setLayout(backwardLayout);

  QVBoxLayout *parametersLayout = new QVBoxLayout(parent);
  QWidget *parametersWidget = new QWidget(parent);
  parametersLayout->addWidget(skipAccelWidget, 0, Qt::AlignLeft);
  parametersLayout->addWidget(timerPollWidget, 0, Qt::AlignLeft);
  parametersLayout->addWidget(parametersApply, 0, Qt::AlignHCenter);
  parametersWidget->setLayout(parametersLayout);
  
  QPushButton *learnButton = new QPushButton(tr("Learn"), parent);
  ledLight = new QLed(parent);
  connect(learnButton, SIGNAL(clicked()), this,     SLOT(midiPoll()));
  connect(learnButton, SIGNAL(clicked()), ledLight, SLOT(switchLed()));
  
  QHBoxLayout* learnLayout = new QHBoxLayout(parent);
  QWidget* learnWidget = new QWidget(parent);
  learnLayout->addStretch(10);
  learnLayout->addWidget(learnButton);
  learnLayout->addStretch(10);
  learnLayout->addWidget(ledLight);
  learnLayout->addStretch(10);
  learnWidget->setLayout(learnLayout);

  QPushButton *exitButton = new QPushButton(tr("Exit"), parent);
  connect(exitButton, SIGNAL(clicked()), this, SLOT(close()));
  
  QHBoxLayout* exitLayout = new QHBoxLayout(parent);
  QWidget* exitWidget = new QWidget(parent);
  exitLayout->addStretch(10);
  exitLayout->addWidget(exitButton);
  exitLayout->addStretch(10);
  exitWidget->setLayout(exitLayout);

  QHBoxLayout* commonButtonsLayout = new QHBoxLayout(parent);
  QWidget* commonButtonsWidget = new QWidget(parent);
  commonButtonsLayout->addWidget(exitWidget);
  commonButtonsLayout->addWidget(learnWidget);
  commonButtonsWidget->setLayout(commonButtonsLayout);
  
  tabWidget = new QTabWidget(parent);
  tabWidget->addTab(playWidget,       QIcon(":/images/Play"),     tr("Play"));
  tabWidget->addTab(stopWidget,       QIcon(":/images/Stop"),     tr("Stop"));
  tabWidget->addTab(rewindWidget,     QIcon(":/images/Rewind"),   tr("Rewind"));
  tabWidget->addTab(forwardWidget,    QIcon(":/images/Forward"),  tr("Forward"));
  tabWidget->addTab(backwardWidget,   QIcon(":/images/Backward"), tr("Backward"));
  tabWidget->addTab(parametersWidget, tr("Parameters"));
  
  QVBoxLayout* dialogLayout = new QVBoxLayout(parent);
  QWidget* dialogWidget = new QWidget(parent);
  dialogLayout->addWidget(tabWidget);
  dialogLayout->addWidget(commonButtonsWidget);
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

void Gui_Midi::playChanged()
{
  jtrans_type[Play]  = TypeLE[Play]->text().toInt();
  jtrans_chan[Play]  = ChanLE[Play]->text().toInt();
  jtrans_pitch[Play] = PitchLE[Play]->text().toInt();
  qDebug() << "DEBUG: jtrans_type[Play]  = " << TypeLE[Play]->text();
  qDebug() << "DEBUG: jtrans_chan[Play]  = " << ChanLE[Play]->text();
  qDebug() << "DEBUG: jtrans_pitch[Play] = " << PitchLE[Play]->text();
}

void Gui_Midi::stopChanged()
{
  jtrans_type[Stop]  = TypeLE[Stop]->text().toInt();
  jtrans_chan[Stop]  = ChanLE[Stop]->text().toInt();
  jtrans_pitch[Stop] = PitchLE[Stop]->text().toInt();
  qDebug() << "DEBUG: jtrans_type[Stop]  = " << TypeLE[Stop]->text();
  qDebug() << "DEBUG: jtrans_chan[Stop]  = " << ChanLE[Stop]->text();
  qDebug() << "DEBUG: jtrans_pitch[Stop] = " << PitchLE[Stop]->text();
}

void Gui_Midi::rewindChanged()
{
  jtrans_type[Rewind]  = TypeLE[Rewind]->text().toInt();
  jtrans_chan[Rewind]  = ChanLE[Rewind]->text().toInt();
  jtrans_pitch[Rewind] = PitchLE[Rewind]->text().toInt();
  qDebug() << "DEBUG: jtrans_type[Rewind]  = " << TypeLE[Rewind]->text();
  qDebug() << "DEBUG: jtrans_chan[Rewind]  = " << ChanLE[Rewind]->text();
  qDebug() << "DEBUG: jtrans_pitch[Rewind] = " << PitchLE[Rewind]->text();
}

void Gui_Midi::forwardChanged()
{
  jtrans_type[Forward]  = TypeLE[Forward]->text().toInt();
  jtrans_chan[Forward]  = ChanLE[Forward]->text().toInt();
  jtrans_pitch[Forward] = PitchLE[Forward]->text().toInt();
  qDebug() << "DEBUG: jtrans_type[Forward]  = " << TypeLE[Forward]->text();
  qDebug() << "DEBUG: jtrans_chan[Forward]  = " << ChanLE[Forward]->text();
  qDebug() << "DEBUG: jtrans_pitch[Forward] = " << PitchLE[Forward]->text();
}

void Gui_Midi::backwardChanged()
{
  jtrans_type[Backward]  = TypeLE[Backward]->text().toInt();
  jtrans_chan[Backward]  = ChanLE[Backward]->text().toInt();
  jtrans_pitch[Backward] = PitchLE[Backward]->text().toInt();
  qDebug() << "DEBUG: jtrans_type[Backward]  = " << TypeLE[Backward]->text();
  qDebug() << "DEBUG: jtrans_chan[Backward]  = " << ChanLE[Backward]->text();
  qDebug() << "DEBUG: jtrans_pitch[Backward] = " << PitchLE[Backward]->text();
}

void Gui_Midi::playLearnChanged()
{
  if (tabWidget->currentIndex()==0) {
    TypeLE[Play]->setText(QString::number(learn_type));
    ChanLE[Play]->setText(QString::number(learn_chan));
    PitchLE[Play]->setText(QString::number(learn_pitch));
  }
}

void Gui_Midi::stopLearnChanged()
{
  if (tabWidget->currentIndex()==1) {
    TypeLE[Stop]->setText(QString::number(learn_type));
    ChanLE[Stop]->setText(QString::number(learn_chan));
    PitchLE[Stop]->setText(QString::number(learn_pitch));
  }
}

void Gui_Midi::rewindLearnChanged()
{
  if (tabWidget->currentIndex()==2) {
    TypeLE[Rewind]->setText(QString::number(learn_type));
    ChanLE[Rewind]->setText(QString::number(learn_chan));
    PitchLE[Rewind]->setText(QString::number(learn_pitch));
  }
}

void Gui_Midi::forwardLearnChanged()
{
  if (tabWidget->currentIndex()==3) {
    TypeLE[Forward]->setText(QString::number(learn_type));
    ChanLE[Forward]->setText(QString::number(learn_chan));
    PitchLE[Forward]->setText(QString::number(learn_pitch));
  }
}

void Gui_Midi::backwardLearnChanged()
{
  if (tabWidget->currentIndex()==4) {
    TypeLE[Backward]->setText(QString::number(learn_type));
    ChanLE[Backward]->setText(QString::number(learn_chan));
    PitchLE[Backward]->setText(QString::number(learn_pitch));
  }
}

void Gui_Midi::parametersChanged()
{
  skipAccel = SkipAccelLE->text().toInt();
  timerPoll = TimerPollLE->text().toFloat();
  qDebug() << "DEBUG: skipAccel = " << SkipAccelLE->text();
  qDebug() << "DEBUG: timerPoll = " << TimerPollLE->text();
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
  jtrans_type[Play] = fields.at(1).toInt(&convert_result); CONVERT_ERROR(convert_result, "play:type");
  TypeLE[Play]->setText(QString::number(jtrans_type[Play]));
    
  line = in.readLine(); fields = line.split(' ');
  jtrans_chan[Play] = fields.at(1).toInt(&convert_result); CONVERT_ERROR(convert_result, "play:chan");
  ChanLE[Play]->setText(QString::number(jtrans_chan[Play]));

  line = in.readLine(); fields = line.split(' ');
  jtrans_pitch[Play] = fields.at(1).toInt(&convert_result); CONVERT_ERROR(convert_result, "play:pitch");
  PitchLE[Play]->setText(QString::number(jtrans_pitch[Play]));

  line = in.readLine(); fields = line.split(' ');
  jtrans_type[Stop] = fields.at(1).toInt(&convert_result); CONVERT_ERROR(convert_result, "stop:type");
  TypeLE[Stop]->setText(QString::number(jtrans_type[Stop]));

  line = in.readLine(); fields = line.split(' ');
  jtrans_chan[Stop] = fields.at(1).toInt(&convert_result); CONVERT_ERROR(convert_result, "stop:chan");
  ChanLE[Stop]->setText(QString::number(jtrans_chan[Stop]));

  line = in.readLine(); fields = line.split(' ');
  jtrans_pitch[Stop] = fields.at(1).toInt(&convert_result); CONVERT_ERROR(convert_result, "stop:pitch");
  PitchLE[Stop]->setText(QString::number(jtrans_pitch[Stop]));
  
  line = in.readLine(); fields = line.split(' ');
  jtrans_type[Rewind] = fields.at(1).toInt(&convert_result); CONVERT_ERROR(convert_result, "rewind:type");
  TypeLE[Rewind]->setText(QString::number(jtrans_type[Rewind]));

  line = in.readLine(); fields = line.split(' ');
  jtrans_chan[Rewind] = fields.at(1).toInt(&convert_result); CONVERT_ERROR(convert_result, "rewind:chan");
  ChanLE[Rewind]->setText(QString::number(jtrans_chan[Rewind]));

  line = in.readLine(); fields = line.split(' ');
  jtrans_pitch[Rewind] = fields.at(1).toInt(&convert_result); CONVERT_ERROR(convert_result, "rewind:pitch");
  PitchLE[Rewind]->setText(QString::number(jtrans_pitch[Rewind]));
  
  line = in.readLine(); fields = line.split(' ');
  jtrans_type[Forward] = fields.at(1).toInt(&convert_result); CONVERT_ERROR(convert_result, "forward:type");
  TypeLE[Forward]->setText(QString::number(jtrans_type[Forward]));

  line = in.readLine(); fields = line.split(' ');
  jtrans_chan[Forward] = fields.at(1).toInt(&convert_result); CONVERT_ERROR(convert_result, "forward:chan");
  ChanLE[Forward]->setText(QString::number(jtrans_chan[Forward]));

  line = in.readLine(); fields = line.split(' ');
  jtrans_pitch[Forward] = fields.at(1).toInt(&convert_result); CONVERT_ERROR(convert_result, "forward:pitch");
  PitchLE[Forward]->setText(QString::number(jtrans_pitch[Forward]));
  
  line = in.readLine(); fields = line.split(' ');
  jtrans_type[Backward] = fields.at(1).toInt(&convert_result); CONVERT_ERROR(convert_result, "backward:type");
  TypeLE[Backward]->setText(QString::number(jtrans_type[Backward]));

  line = in.readLine(); fields = line.split(' ');
  jtrans_chan[Backward] = fields.at(1).toInt(&convert_result); CONVERT_ERROR(convert_result, "backward:chan");
  ChanLE[Backward]->setText(QString::number(jtrans_chan[Backward]));

  line = in.readLine(); fields = line.split(' ');
  jtrans_pitch[Backward] = fields.at(1).toInt(&convert_result); CONVERT_ERROR(convert_result, "backward:pitch");
  PitchLE[Backward]->setText(QString::number(jtrans_pitch[Backward]));
  
  line = in.readLine(); fields = line.split(' ');
  skipAccel = fields.at(1).toFloat(&convert_result); CONVERT_ERROR(convert_result, "skipAccel");
  
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

bool Gui_Midi::isLearnMode() const
{
  return learn_mode;
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

void Gui_Midi::midiPoll()
{
  learn_mode = !learn_mode;
  
  if (learn_mode) startMidiPoll();
  else            stopMidiPoll();
}

void Gui_Midi::startMidiPoll()
{
  // Connect our signal and slot
  connect(midiTimer, SIGNAL(timeout()), this, SLOT(playLearnChanged()));
  connect(midiTimer, SIGNAL(timeout()), this, SLOT(stopLearnChanged()));
  connect(midiTimer, SIGNAL(timeout()), this, SLOT(rewindLearnChanged()));
  connect(midiTimer, SIGNAL(timeout()), this, SLOT(forwardLearnChanged()));
  connect(midiTimer, SIGNAL(timeout()), this, SLOT(backwardLearnChanged()));
  
  midiTimer->setInterval(timerPoll);
  midiTimer->setSingleShot(false);
  midiTimer->start(timerPoll); // Start the timer to update the Gui
}

void Gui_Midi::stopMidiPoll()
{
  midiTimer->stop(); // Stop the timer
}
