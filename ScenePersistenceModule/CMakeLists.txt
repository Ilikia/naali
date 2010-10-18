# Define target name and output directory
init_target(ScenePersistenceModule OUTPUT modules/core)

GetEntityComponents()

# Define source files
file(GLOB CPP_FILES *.cpp *.c)
file(GLOB H_FILES *.h)
file(GLOB XML_FILES *.xml)
file(GLOB MOC_FILES ScenePersistenceModule.h)
set(SOURCE_FILES ${CPP_FILES} ${H_FILES})

# Qt4 Wrap
QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})
QT4_WRAP_UI(UI_SRCS ${UI_FILES})
QT4_ADD_RESOURCES(RESOURCE_SRCS ${RESOURCE_FILES})

add_definitions(-DSCENEPERSISTENCE_MODULE_EXPORTS) 

use_package(BOOST)
use_package(POCO)
use_package(OGRE)
use_package(QT4)
use_package(CURL)
use_package(XMLRPC)

use_modules(Core Foundation Interfaces SceneManager RexCommon ProtocolUtilities)

build_library(${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS})

link_package(OGRE)
link_package(BOOST)
link_package(POCO)
link_package(QT4)
link_package(CURL)
link_package(XMLRPC)

link_modules(Core Foundation Interfaces SceneManager RexCommon ProtocolUtilities)

SetupCompileFlags()
CopyModuleXMLFile()

final_target()