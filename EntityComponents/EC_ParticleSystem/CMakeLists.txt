# Define target name and output directory
init_target (EC_ParticleSystem)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB XML_FILES *.xml)
file (GLOB MOC_FILES EC_ParticleSystem.h)

# Qt4 Moc files to subgroup "CMake Moc"
MocFolder ()

set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

# Qt4 Wrap
QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})

use_modules (Core Foundation Interfaces Scene RexCommon OgreRenderingModule)

build_library (${TARGET_NAME} STATIC ${SOURCE_FILES} ${MOC_SRCS} )

link_ogre()
link_modules (Core Foundation Interfaces Scene RexCommon OgreRenderingModule)

SetupCompileFlagsWithPCH()

final_target ()
