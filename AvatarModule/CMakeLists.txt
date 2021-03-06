# Define target name and output directory
init_target (AvatarModule OUTPUT modules/core)

GetEntityComponents()

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB XML_FILES *.xml)
file (GLOB UI_FILES ui/*.ui)

# SubFolders to project with filtering
AddSourceFolder (Avatar)
AddSourceFolder (AvatarEditing)
AddSourceFolder (EntityComponent)

# Qt4 Moc files to "CMake Moc" subgroup
# and ui_*.h generated .h files to "Generated UI" subgroup
MocFolder ()
UiFolder ()

set (FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${H_FILES} ${CPP_FILES} ${UI_FILES} PARENT_SCOPE)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

# Qt4 Wrap
QT4_WRAP_CPP(MOC_SRCS ${H_FILES})
QT4_WRAP_UI(UI_SRCS ${UI_FILES})

add_definitions (-DAVATAR_MODULE_EXPORTS)

# Includes
use_modules(AvatarModule Core Foundation Interfaces Scene RexCommon ProtocolUtilities
    HttpUtilities RpcUtilities AssetModule OgreRenderingModule Input Asset Console)

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS} ${UI_SRCS} ${RESOURCE_SRCS})

# Linking
link_ogre()

if (APPLE)
    link_modules(Core Foundation Interfaces Scene RexCommon ProtocolUtilities ProtocolModuleOpenSim ProtocolModuleTaiga
        HttpUtilities RpcUtilities AssetModule OgreRenderingModule Input Asset Console)
else()
    link_modules(Core Foundation Interfaces Scene RexCommon ProtocolUtilities
        HttpUtilities RpcUtilities AssetModule OgreRenderingModule Input Asset Console)
endif()

LinkEntityComponent(EntityComponents/EC_HoveringText EC_HoveringText)
LinkEntityComponent(EntityComponents/EC_HoveringWidget EC_HoveringWidget)
LinkEntityComponent(EntityComponents/EC_OpenSimPresence EC_OpenSimPresence)
LinkEntityComponent(EntityComponents/EC_SoundListener EC_SoundListener)
LinkEntityComponent(EntityComponents/EC_NetworkPosition EC_NetworkPosition)

SetupCompileFlagsWithPCH()
CopyModuleXMLFile()

final_target ()

