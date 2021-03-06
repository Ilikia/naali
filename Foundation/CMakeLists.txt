# Define target name
init_target (Foundation)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB MOC_FILES Framework.h Application.h QtInputService.h InputContext.h KeyEventSignal.h
    QtInputKeyEvent.h QtInputMouseEvent.h FrameAPI.h ConsoleAPI.h DebugAPI.h ConfigAPI.h VersionInfo.h)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})
set (FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${H_FILES} ${CPP_FILES} PARENT_SCOPE)

# Build parameters required by Qt - define files for moc prepass, process resource and ui files.
QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})
QT4_WRAP_UI(UI_SRCS ${UI_FILES})
QT4_ADD_RESOURCES(RESOURCE_SRCS ${RESOURCE_FILES})

use_modules (Foundation Core Interfaces Scene Ui Input Asset Audio Console Devices)

build_library (${TARGET_NAME} STATIC ${SOURCE_FILES} ${MOC_SRCS})

link_modules (Core Interfaces Scene Ui Input Asset Audio Console Devices)

SetupCompileFlagsWithPCH()

final_target ()
