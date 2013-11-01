#=============================================================================
#  QtMidiTranslate
#  Qt Midi Translater
#  $Id:$
#
#  Copyright (C) 2013 by Yann Collette and others
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License version 2.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#=============================================================================

option(UPDATE_TRANSLATIONS "Update source translation translations/*.ts files (WARNING: make clean will delete the source .ts files! Danger!)" OFF)

set(QTMIDI_SRCS main.cpp)

set(QTMIDI_UI_SRCS ./gui_midi.cpp
		   ./help_dialog.cpp)

set(QTMIDI_UI_HEADERS ./gui_midi.h
		      ./help_dialog.h)

set(QTMIDI_TRANS ./translations/qtmidi_en.ts)

# Manage translation files
set(FILES_TO_TRANSLATE)

file(GLOB FTT_HEADER_FILES *.h)
file(GLOB FTT_CPP_FILES    *.cpp)
file(GLOB FTT_UI_FILES     *.ui)

set(FILES_TO_TRANSLATE ${FTT_HEADER_FILES} ${FTT_CPP_FILES} ${FTT_UI_FILES})

file(GLOB TRANSLATIONS_FILES translations/*.ts)

if (UPDATE_TRANSLATIONS)
  qt4_create_translation(QTMIDI_QM ${FILES_TO_TRANSLATE} ${TRANSLATIONS_FILES})
else (UPDATE_TRANSLATIONS)
  qt4_add_translation(QTMIDI_QM ${TRANSLATIONS_FILES})
endif (UPDATE_TRANSLATIONS) 

add_custom_target(translations_target DEPENDS ${QTMIDI_QM})

qt4_wrap_cpp(QTMIDI_UI_HEADERS_MOC ${QTMIDI_UI_HEADERS})

qt4_add_resources(QTMIDI_RSRCS ./qtmidi.qrc)

#qt4_add_translation(QTMIDI_QM ${QTMIDI_TRANS})

add_executable(qtmidi ${QTMIDI_SRCS} ${QTMIDI_RSRCS} ${QTMIDI_QM} ${QTMIDI_UI_SRCS} ${QTMIDI_UI_HEADERS_MOC})
target_link_libraries(qtmidi ${QT_LIBRARIES} ${JACK_LIBRARIES})

install(TARGETS qtmidi
        DESTINATION bin)

install(FILES ${QM_FILES} 
        DESTINATION ${CMAKE_INSTALL_PREFIX}/translations)
