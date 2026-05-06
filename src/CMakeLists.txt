#=============================================================================
#  QtMidiTranslate
#  Qt Midi Translater
#  $Id:$
#
#  Copyright (C) 2013-2015 by Yann Collette and others
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

set(QTMIDI_SRCS main.cpp)

set(QTMIDI_UI_SRCS
    ./gui_midi.cpp
    ./QLed.cpp
    ./help_dialog.cpp)

set(QTMIDI_UI_HEADERS
    ./gui_midi.h
    ./QLed.h
    ./help_dialog.h)

qt_add_resources(QTMIDI_RSRCS ./qtmidi.qrc)

add_executable(qtmiditrans ${QTMIDI_SRCS} ${QTMIDI_RSRCS} ${QTMIDI_UI_SRCS})
target_link_libraries(qtmiditrans Qt6::Widgets Qt6::WebEngineWidgets ${JACK_LIBRARIES})
target_include_directories(qtmiditrans PRIVATE ${JACK_INCLUDE_DIRS})
target_link_directories(qtmiditrans PRIVATE ${JACK_LIBRARY_DIRS})

# qt_add_translations (Qt >= 6.2):
#   - creates an "update_translations" target that runs lupdate on the listed sources
#   - compiles .ts -> .qm and makes them available for installation
qt_add_translations(qtmiditrans
    TS_FILES translations/qtmidi_en.ts translations/qtmidi_fr.ts
    SOURCES ${QTMIDI_SRCS} ${QTMIDI_UI_SRCS} ${QTMIDI_UI_HEADERS}
    QM_FILES_OUTPUT_VARIABLE QTMIDI_QM
)

install(TARGETS qtmiditrans
        DESTINATION bin)

install(FILES ${QTMIDI_QM}
        DESTINATION ${CMAKE_INSTALL_PREFIX}/translations)
