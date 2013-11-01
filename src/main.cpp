//=============================================================================
//  QtMidiTrans
//  Qt Midi Translater
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

#include <QApplication>
#include <QPlastiqueStyle>
#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>
#include <QDebug>

#include <iostream>
#include <sstream>
#include <string>

#include <signal.h>
#include <stdio.h>
#include <assert.h>

#include <jack/jack.h>
#include <jack/types.h>
#include <jack/session.h>
#include <jack/transport.h>
#include <jack/midiport.h>

#include "gui_midi.h"

jack_client_t *client = NULL;
int qtmidi_quit = 0;
static jack_port_t* port = NULL;

static void signal_handler(int sig)
{
  jack_client_close(client);
  std::cerr << QT_TRANSLATE_NOOP("main","signal received, exiting ...") << std::endl;
  exit(0);
}

int qtmidi_process(jack_nframes_t nframes, void *arg)
{
  void* buffer;
  jack_nframes_t N, i;
  Gui_Midi * ptrGuiMidi = (Gui_Midi *)arg;
  int jtrans_stop_type  = 0,  jtrans_stop_chan  = 0,  jtrans_stop_pitch  = -1;
  int jtrans_start_type = 0,  jtrans_start_chan = 0,  jtrans_start_pitch = -1;
  int midi_event_type   = 0,  midi_event_chan   = 0,  midi_event_pitch   = -1;
  bool learn_mode = false;
  
  jtrans_start_type  = ptrGuiMidi->getJTransStartType();
  jtrans_start_chan  = ptrGuiMidi->getJTransStartChan();
  jtrans_start_pitch = ptrGuiMidi->getJTransStartPitch();
  jtrans_stop_type   = ptrGuiMidi->getJTransStopType();
  jtrans_stop_chan   = ptrGuiMidi->getJTransStopChan();
  jtrans_stop_pitch  = ptrGuiMidi->getJTransStopPitch();
  
  learn_mode = ptrGuiMidi->isStartLearnMode() || ptrGuiMidi->isStopLearnMode();
  
  buffer = jack_port_get_buffer(port, nframes);
  assert(buffer);
  
  N = jack_midi_get_event_count(buffer);

  for(i=0; i<N; ++i) 
    {
      jack_midi_event_t event;
      int r;
      
      r = jack_midi_event_get(&event, buffer, i);
      
      if (r==0) 
	{
	  size_t j;
	  
	  if (event.size==0) return 0;
	  
	  assert(event.size==3);
	  
	  midi_event_type  = event.buffer[0] & 0xf0;
	  midi_event_chan  = event.buffer[0] & 0xf;
	  midi_event_pitch = event.buffer[1];

	  if (learn_mode)
	    {
	      qDebug() << "DEBUG: midi_event_type  = " << midi_event_type;
	      qDebug() << "DEBUG: midi_event_chan  = " << midi_event_chan;
	      qDebug() << "DEBUG: midi_event_pitch = " << midi_event_pitch;
	      
	      ptrGuiMidi->setLearnType(midi_event_type);
	      ptrGuiMidi->setLearnChan(midi_event_chan);
	      ptrGuiMidi->setLearnPitch(midi_event_pitch);
	    }
	  
	  if ((midi_event_type==jtrans_start_type) &&
	      (midi_event_chan==jtrans_start_chan) &&
	      (midi_event_pitch==jtrans_start_pitch))
	    {
	      jack_transport_start(client);
	    }
	  else if ((midi_event_type==jtrans_stop_type) &&
		   (midi_event_chan==jtrans_stop_chan) &&
		   (midi_event_pitch==jtrans_stop_pitch))
	    {
	      jack_transport_stop(client);
	    }
	}
    }
  
  return 0;
}

void qtmidi_session_callback(jack_session_event_t *event, void *arg)
{
  std::stringstream retval;

  qDebug() << "DEBUG: session notification";
  qDebug() << "path " << event->session_dir << ", uuid " << event->client_uuid << ", type: " << (event->type == JackSessionSave ? "save" : "quit");
  
  // Build the command line
  retval.str("");
  retval << "qtmidi " << event->client_uuid;
  event->command_line = strdup(retval.str().c_str());
  
  jack_session_reply(client, event);
  
  if (event->type==JackSessionSaveAndQuit) qtmidi_quit = 1;
  
  jack_session_event_free(event);
}

void jack_shutdown(void *arg)
{
  exit(1);
}

int main(int argc, char * argv[])
{
  int result = 0;
  const char *client_name = "QtMidiTrans";
  const char *server_name = NULL;
  jack_status_t status;

#ifdef ENABLE_JACK
  if (argc==1)        client = jack_client_open(client_name, JackNullOption, &status);
  else if (argc == 2) client = jack_client_open(client_name, JackSessionID, &status, argv[1]);
  
  if (client == NULL) 
    {
      std::cerr << QT_TRANSLATE_NOOP("main","jack_client_open() failed, status = ") << status << ")." << std::endl;
      if (status & JackServerFailed) 
	{
	  std::cerr << QT_TRANSLATE_NOOP("main","Unable to connect to JACK server.") << std::endl;
	}
      exit (1);
    }

  if (status & JackServerStarted) 
    {
      std::cerr << QT_TRANSLATE_NOOP("main","JACK server started.") << std::endl;
    }

  if (status & JackNameNotUnique) 
    {
      client_name = jack_get_client_name(client);
      std::cerr << QT_TRANSLATE_NOOP("main","unique name `") << client_name << QT_TRANSLATE_NOOP("main","' assigned") << std::endl;
    }

  jack_on_shutdown(client, jack_shutdown, 0);
#endif

  /* install a signal handler to properly quits jack client */
#ifdef WIN32
  signal(SIGINT,  signal_handler);
  signal(SIGABRT, signal_handler);
  signal(SIGTERM, signal_handler);
#else
  signal(SIGQUIT, signal_handler);
  signal(SIGTERM, signal_handler);
  signal(SIGHUP,  signal_handler);
  signal(SIGINT,  signal_handler);
#endif

  QApplication app(argc, argv);
  
  QTranslator qtTranslator;
  qtTranslator.load("qt_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
  app.installTranslator(&qtTranslator);

  QTranslator myappTranslator;
  if (myappTranslator.load("qtmidi_" + QLocale::system().name().toLower()))
    {
      app.installTranslator(&myappTranslator);
    }
  else
    {
      std::cerr << QT_TRANSLATE_NOOP("main","failed to load translation file ") << "qtmidi" << qPrintable(QLocale::system().name().toLower()) << "." << std::endl;
    }

  QApplication::setStyle(new  QPlastiqueStyle); // set plastique style

  Gui_Midi * mainMidi = new Gui_Midi(NULL);

  mainMidi->show();

#ifdef ENABLE_JACK
  /* Tell the JACK server that we are ready to roll.  Our process() callback will start running now. */
  jack_set_process_callback(client, qtmidi_process, (void *)mainMidi);

  /* tell the JACK server to call `session_callback()' if the session is saved. */
  jack_set_session_callback(client, qtmidi_session_callback, NULL);

  /* register the midi input port */
  port = jack_port_register(client, "input", JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);
  if (port==NULL) 
    {
      std::cerr << QT_TRANSLATE_NOOP("main","Could not register port.") << std::endl;
      exit (EXIT_FAILURE);
    }
  
  /* Start the jack client */
  if (jack_activate(client))
    {
      std::cerr << QT_TRANSLATE_NOOP("main","cannot activate client.") << std::endl;
      exit (1);
    }
#endif
  
  // if qtjmix_quit == 1, we should quit
  result = app.exec();

#ifdef ENABLE_JACK
  jack_client_close(client);
#endif

  return result;
}
