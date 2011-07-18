# =============================================================================
# per-dependency configuration macros
#
# All per-dependency configuration (or hacks) should go here. All per-module
# build instructions should go in <Module>/CMakeLists.txt. The rest should
# remain generic.

macro (configure_boost)
    if (MSVC)
        set(Boost_USE_MULTITHREADED TRUE)
        set(Boost_USE_STATIC_LIBS TRUE)
    else ()
        set(Boost_USE_STATIC_LIBS FALSE)
    endif ()

    if (APPLE)
            set (BOOST_COMPONENTS boost_date_time boost_filesystem boost_system boost_thread boost_regex boost_program_options)
    else ()
            set (BOOST_COMPONENTS date_time filesystem system thread regex program_options)
    endif ()
 
    sagase_configure_package (BOOST 
        NAMES Boost boost
        COMPONENTS ${BOOST_COMPONENTS}
        PREFIXES ${ENV_TUNDRA_DEP_PATH})

    if (APPLE)
        set (BOOST_LIBRARY_DIRS ${ENV_TUNDRA_DEP_PATH}/lib)
        set (BOOST_INCLUDE_DIRS ${ENV_TUNDRA_DEP_PATH}/include)
    endif()

    # boost library naming is complex, and FindBoost.cmake is preferred to 
    # find the correct names. however on windows it appears to not find the
    # library directories correctly. find_path cannot be counted on to find
    # the libraries as component thread -> libboost_thread_vc90-mt.lib (etc.)

    if (MSVC)
        set (BOOST_INCLUDE_DIRS ${BOOST_INCLUDE_DIRS} ${ENV_TUNDRA_DEP_PATH}/Boost/include)
        set (BOOST_LIBRARY_DIRS ${BOOST_LIBRARY_DIRS} ${ENV_TUNDRA_DEP_PATH}/Boost/lib)
    endif ()

    sagase_configure_report (BOOST)
endmacro (configure_boost)

macro (configure_poco)
    sagase_configure_package (POCO 
        NAMES Poco PoCo poco
        COMPONENTS Poco PocoFoundation PocoNet PocoUtil PocoXML
        PREFIXES ${ENV_TUNDRA_DEP_PATH})

    # POCO autolinks on MSVC
    if (MSVC)
        set (POCO_LIBRARIES "")
        set (POCO_DEBUG_LIBRARIES "")
    endif ()

    sagase_configure_report (POCO)
endmacro (configure_poco)

macro (configure_qt4)
    sagase_configure_package (QT4 
        NAMES Qt4 4.6.1
        COMPONENTS QtCore QtGui QtWebkit QtScript QtScriptTools QtXml QtNetwork QtUiTools QtDeclarative
        PREFIXES ${ENV_TUNDRA_DEP_PATH} ${ENV_QT_DIR})

    # FindQt4.cmake
    if (QT4_FOUND AND QT_USE_FILE)
    
        include (${QT_USE_FILE})
        
        set (QT4_INCLUDE_DIRS 
            ${QT_INCLUDE_DIR}
            ${QT_QTCORE_INCLUDE_DIR}
            ${QT_QTGUI_INCLUDE_DIR}
            ${QT_QTUITOOLS_INCLUDE_DIR}
            ${QT_QTNETWORK_INCLUDE_DIR}
            ${QT_QTXML_INCLUDE_DIR}
            ${QT_QTSCRIPT_INCLUDE_DIR}
            ${QT_DECLARATIVE_INCLUDE_DIR}
            ${QT_QTWEBKIT_INCLUDE_DIR})
            
#            ${QT_QTSCRIPTTOOLS_INCLUDE_DIR}
#            ${QT_PHONON_INCLUDE_DIR}

        
        set (QT4_LIBRARY_DIR  
            ${QT_LIBRARY_DIR})
        
        set (QT4_LIBRARIES 
            ${QT_LIBRARIES}
            ${QT_QTCORE_LIBRARY}
            ${QT_QTGUI_LIBRARY}
            ${QT_QTUITOOLS_LIBRARY}
            ${QT_QTNETWORK_LIBRARY}
            ${QT_QTXML_LIBRARY}
            ${QT_QTSCRIPT_LIBRARY}
            ${QT_DECLARATIVE_LIBRARY}
            ${QT_QTWEBKIT_LIBRARY})
            
#            ${QT_QTSCRIPTTOOLS_LIBRARY}
#            ${QT_PHONON_LIBRARY}
        
    endif ()
    
    sagase_configure_report (QT4)
endmacro (configure_qt4)

macro (configure_python)
    sagase_configure_package (PYTHON
        NAMES PythonLibs Python python Python26 python26 Python2.6 python2.6
        COMPONENTS Python python Python26 python Python2.6 python2.6
        PREFIXES ${ENV_TUNDRA_DEP_PATH})

    # FindPythonLibs.cmake
    if (PYTHONLIBS_FOUND)
        set (PYTHON_LIBRARIES ${PYTHON_LIBRARY})
        set (PYTHON_INCLUDE_DIRS ${PYTHON_INCLUDE_PATH})
        #unset (PYTHON_DEBUG_LIBRARIES ${PYTHON_DEBUG_LIBRARY})
    endif ()
    
    # FindPythonLibs.cmake prefers the system-wide Python, which does not
    # include debug libraries, so we force to NAALI_DEP_PATH.

    if (MSVC)
        set (PYTHON_LIBRARY_DIRS ${ENV_TUNDRA_DEP_PATH}/Python/lib)
        set (PYTHON_INCLUDE_DIRS ${ENV_TUNDRA_DEP_PATH}/Python/include)
        set (PYTHON_LIBRARIES python26)
        set (PYTHON_DEBUG_LIBRARIES python26_d)
    endif()
    
    sagase_configure_report (PYTHON)
endmacro (configure_python)

macro (configure_python_qt)
    sagase_configure_package (PYTHON_QT
        NAMES PythonQt
        COMPONENTS PythonQt PythonQt_QtAll
        PREFIXES ${ENV_TUNDRA_DEP_PATH})

    sagase_configure_report (PYTHON_QT)
endmacro (configure_python_qt)

macro (configure_ogre)
    
    if (NOT WIN32)
        # Mac
        if (APPLE)
    	  FIND_LIBRARY (OGRE_LIBRARY NAMES Ogre)
    	  set (OGRE_INCLUDE_DIRS ${OGRE_LIBRARY}/Headers)
    	  set (OGRE_LIBRARIES ${OGRE_LIBRARY})
        # Linux
        else ()
            sagase_configure_package (OGRE 
                NAMES Ogre OgreSDK ogre OGRE
                COMPONENTS Ogre ogre OGRE OgreMain 
                PREFIXES ${ENV_OGRE_HOME} ${ENV_NAALI_DEP_PATH})
        endif ()
        
        sagase_configure_report (OGRE)
        
    else ()
        # Find directx
        include(FindDirectX)
        
        # Find ogre
        if (DirectX_FOUND)
            set (TUNDRA_OGRE_NEEDED_COMPONENTS Ogre ogre OGRE OgreMain RenderSystem_Direct3D9)
        else ()
            set (TUNDRA_OGRE_NEEDED_COMPONENTS Ogre ogre OGRE OgreMain)
        endif()
        
        sagase_configure_package (OGRE 
            NAMES Ogre OgreSDK ogre OGRE
            COMPONENTS ${TUNDRA_OGRE_NEEDED_COMPONENTS}
            PREFIXES ${ENV_OGRE_HOME} ${ENV_TUNDRA_DEP_PATH})

        # Report ogre then search check directx
        sagase_configure_report (OGRE)
        
        # DirectX SDK found, use DX9 surface blitting
        message ("** Configuring DirectX")
        if (DirectX_FOUND)
            message (STATUS "-- Include Directories:")
            message (STATUS "       " ${DirectX_INCLUDE_DIR})
            message (STATUS "-- Library Directories:")
            message (STATUS "       " ${DirectX_LIBRARY_DIR})
            message (STATUS "-- Defines:")
            message (STATUS "        USE_D3D9_SUBSURFACE_BLIT")
            
            add_definitions (-DUSE_D3D9_SUBSURFACE_BLIT)
            include_directories (${DirectX_INCLUDE_DIR})
            link_directories (${DirectX_LIBRARY_DIR})
        else ()
            message (STATUS "DirectX not found!")
            message (STATUS "-- Install DirectX SDK to enable additional features. If you already have the DirectX SDK installed")
            message (STATUS "   please set DIRECTX_ROOT env variable as your installation directory.")
        endif()
        message (STATUS "")
    endif ()
endmacro (configure_ogre)

macro (link_ogre)
    use_package (OGRE)
    link_package (OGRE)
endmacro ()

macro (configure_skyx)
    sagase_configure_package (SKYX
        NAMES SkyX SKYX skyx
        COMPONENTS SkyX SKYX skyx
        PREFIXES ${ENV_TUNDRA_DEP_PATH})
    
    sagase_configure_report (SKYX)
endmacro (configure_skyx)

macro (configure_hydrax)
    sagase_configure_package (HYDRAX
        NAMES Hydrax HYDRAX hydrax
        COMPONENTS Hydrax HYDRAX hydrax
        PREFIXES ${ENV_TUNDRA_DEP_PATH})
    
    sagase_configure_report (HYDRAX)
endmacro (configure_hydrax)

macro (configure_qtpropertybrowser)
    sagase_configure_package (QT_PROPERTY_BROWSER
        NAMES QtPropertyBrowser QtSolutions_PropertyBrowser-2.5
        COMPONENTS QtPropertyBrowser QtSolutions_PropertyBrowser-2.5
        PREFIXES ${ENV_TUNDRA_DEP_PATH})
    
    sagase_configure_report (QT_PROPERTY_BROWSER)
endmacro (configure_qtpropertybrowser)

macro (configure_xmlrpc)
    if (APPLE)
        sagase_configure_package (XMLRPC 
            NAMES xmlrpc xmlrpcepi xmlrpc-epi
            COMPONENTS xmlrpc xmlrpcepi xmlrpc-epi xmlrpcepid iconv
            PREFIXES ${ENV_TUNDRA_DEP_PATH}
                ${ENV_TUNDRA_DEP_PATH}/xmlrpc-epi/src
                ${ENV_TUNDRA_DEP_PATH}/xmlrpc-epi/Debug
                ${ENV_TUNDRA_DEP_PATH}/xmlrpc-epi/Release)
    else()
        sagase_configure_package (XMLRPC 
            NAMES xmlrpc xmlrpcepi xmlrpc-epi
            COMPONENTS xmlrpc xmlrpcepi xmlrpc-epi xmlrpcepid
            PREFIXES ${ENV_TUNDRA_DEP_PATH}
                ${ENV_TUNDRA_DEP_PATH}/xmlrpc-epi/src
                ${ENV_TUNDRA_DEP_PATH}/xmlrpc-epi/Debug
        ${ENV_TUNDRA_DEP_PATH}/xmlrpc-epi/Release)
    endif()
   
    if (MSVC)
        set (XMLRPC_LIBRARIES xmlrpcepi)
        set (XMLRPC_DEBUG_LIBRARIES xmlrpcepid)
    endif()
    
    sagase_configure_report (XMLRPC)
endmacro (configure_xmlrpc)

macro (configure_curl)
    sagase_configure_package (CURL 
        NAMES Curl curl libcurl
        COMPONENTS curl libcurl_imp libcurld_imp
        PREFIXES ${ENV_TUNDRA_DEP_PATH}
        ${ENV_TUNDRA_DEP_PATH}/libcurl/lib/DLL-Debug
        ${ENV_TUNDRA_DEP_PATH}/libcurl/lib/DLL-Release)
    
    if (MSVC)
        set (CURL_LIBRARIES libcurl_imp)
        set (CURL_DEBUG_LIBRARIES libcurld_imp)
    endif()

    if (APPLE)
        set (CURL_LIBRARIES curl)
    endif()
    
    sagase_configure_report (CURL)
endmacro (configure_curl)

macro (configure_openjpeg)
    sagase_configure_package (OPENJPEG
        NAMES OpenJpeg OpenJPEG openjpeg
        COMPONENTS OpenJpeg OpenJPEG openjpeg
        PREFIXES ${ENV_TUNDRA_DEP_PATH}
        ${ENV_TUNDRA_DEP_PATH}/OpenJpeg/libopenjpeg
        ${ENV_TUNDRA_DEP_PATH}/OpenJpeg/Debug
        ${ENV_TUNDRA_DEP_PATH}/OpenJpeg/Release)
    
    sagase_configure_report (OPENJPEG)
endmacro (configure_openjpeg)

macro (configure_telepathy_qt4)
    sagase_configure_package (TELEPATHY_QT4 
        NAMES QtTelepathy Telepathy-QT4 telepathy-qt4 TelepathyQt4 telepathy-1.0
        COMPONENTS QtTelepathyCore QtTelepathyCommon QtTelepathyClient telepathy-qt4 telepathy-qt4-farsight connection # connection added here to help sagase to find include folder with connection.h file
        PREFIXES ${ENV_TUNDRA_DEP_PATH} )
    sagase_configure_report (TELEPATHY_QT4)
endmacro (configure_telepathy_qt4)

macro (configure_gstreamer)
    sagase_configure_package (GSTREAMER
        NAMES gstreamer gst gstreamer-0.10
        COMPONENTS gstreamer gstfarsight gstinterfaces gst # gst added to help sagese to find include folder with gst.h file
        PREFIXES ${ENV_TUNDRA_DEP_PATH})
    sagase_configure_report (GSTREAMER)
endmacro (configure_gstreamer)

macro (configure_dbus)
    sagase_configure_package (DBUS
        NAMES dbus dbus-1
        COMPONENTS dbus-1 dbus # dbus added to help sagese to find include folder with dbus.h file
        PREFIXES ${ENV_TUNDRA_DEP_PATH})
    sagase_configure_report (DBUS)
endmacro (configure_dbus)

macro (configure_glib)
    sagase_configure_package (GLIB
        NAMES glib-2.0 glib Glib
        COMPONENTS glib-2.0 gobject-2.0 glib # glib added to help sagese to find include folder with glib.h file
        PREFIXES ${ENV_TUNDRA_DEP_PATH})
    sagase_configure_report (GLIB)
endmacro (configure_glib)

macro (configure_telepathy_glib)
    sagase_configure_package (TELEPATHY_GLIB
        NAMES telepathy-glib Telepathy-Glib
        COMPONENTS telepathy-glib connection # connection added to help sagese to find include folder with connection.h file
        PREFIXES ${ENV_TUNDRA_DEP_PATH})
    sagase_configure_report (TELEPATHY_GLIB)
endmacro (configure_telepathy_glib)

macro (configure_telepathy_farsight)
    sagase_configure_package (TELEPATHY_FARSIGHT 
        NAMES telepathy-farsight Telepathy-Farsight
        COMPONENTS telepathy-farsight stream # stream added to help sagese to find include folder with stream.h file
        PREFIXES ${ENV_TUNDRA_DEP_PATH})
    sagase_configure_report (TELEPATHY_FARSIGHT)
endmacro (configure_telepathy_farsight)

macro (configure_farsight2)
    sagase_configure_package (FARSIGHT2
        NAMES farsight2 farsight2-0.10
        COMPONENTS gstfarsight fs-interfaces # fs-interfaces added to help sagese to find include folder with fs-interfaces.h file
        PREFIXES ${ENV_TUNDRA_DEP_PATH}/farsight2)
    sagase_configure_report (FARSIGHT2)
endmacro (configure_farsight2)

macro (configure_dbus_glib)
    sagase_configure_package (DBUS_GLIB
        NAMES dbus-glib dbus
        COMPONENTS dbus-glib
        PREFIXES ${ENV_TUNDRA_DEP_PATH}/dbus-glib)
    sagase_configure_report (DBUS_GLIB)
endmacro (configure_dbus_glib)

macro (configure_openal)
    sagase_configure_package(OPENAL
        NAMES OpenAL openal
        COMPONENTS al OpenAL32
        PREFIXES ${ENV_TUNDRA_DEP_PATH}/OpenAL ${ENV_TUNDRA_DEP_PATH}/OpenAL/libs/Win32)

        if (OPENAL_FOUND)
            set (OPENAL_LIBRARIES ${OPENAL_LIBRARY})
            set (OPENAL_INCLUDE_DIRS ${OPENAL_INCLUDE_DIR})
        endif()

        # Force include dir on MSVC
        if (MSVC)
             set (OPENAL_INCLUDE_DIRS ${ENV_TUNDRA_DEP_PATH}/OpenAL/include)
        endif ()
    sagase_configure_report (OPENAL)
endmacro (configure_openal)

macro (configure_ogg)
    sagase_configure_package(OGG
        NAMES ogg libogg
        COMPONENTS ogg libogg
        PREFIXES ${ENV_TUNDRA_DEP_PATH}/libogg)
        
        # Force include dir on MSVC
        if (MSVC)
             set (OGG_INCLUDE_DIRS ${ENV_TUNDRA_DEP_PATH}/libogg/include)
        endif ()
    sagase_configure_report (OGG)
endmacro (configure_ogg)

macro (configure_vorbis)
if (APPLE)
    sagase_configure_package(VORBIS
        NAMES vorbisfile vorbis libvorbis libvorbisfile
        COMPONENTS vorbis libvorbis vorbisfile libvorbisfile
        PREFIXES ${ENV_TUNDRA_DEP_PATH}/libvorbis)
else()
    sagase_configure_package(VORBIS
        NAMES vorbisfile vorbis libvorbis
        COMPONENTS vorbis libvorbis libvorbisfile
        PREFIXES ${ENV_TUNDRA_DEP_PATH}/libvorbis)
endif()
        # Force include dir on MSVC
        if (MSVC)
             set (VORBIS_INCLUDE_DIRS ${ENV_TUNDRA_DEP_PATH}/libvorbis/include)
        endif ()
    sagase_configure_report (VORBIS)
endmacro (configure_vorbis)

macro (configure_theora)
    sagase_configure_package(THEORA
        NAMES theora libtheora
        COMPONENTS theora libtheora
        PREFIXES ${ENV_TUNDRA_DEP_PATH}/libtheora)
        
        # Force include dir on MSVC
        if (MSVC)
             set (THEORA_INCLUDE_DIRS ${ENV_TUNDRA_DEP_PATH}/libtheora/include)
        endif ()
    sagase_configure_report (THEORA)
endmacro (configure_theora)

macro (configure_mumbleclient)
    sagase_configure_package(MUMBLECLIENT
        NAMES mumbleclient
        COMPONENTS mumbleclient client
        PREFIXES ${ENV_TUNDRA_DEP_PATH}/libmumbleclient)
    sagase_configure_report (MUMBLECLIENT)
endmacro (configure_mumbleclient)

macro (configure_openssl)
    sagase_configure_package(OPENSSL
        NAMES openssl
        COMPONENTS libeay32 ssleay32 ssl
        PREFIXES ${ENV_TUNDRA_DEP_PATH}/OpenSSL)
    # remove 'NOTFOUND' entry which makes to linking impossible    
    if (MSVC)
        list(REMOVE_ITEM OPENSSL_LIBRARIES debug optimized SSL_EAY_RELEASE-NOTFOUND LIB_EAY_RELEASE-NOTFOUND SSL_EAY_DEBUG-NOTFOUND LIB_EAY_DEBUG-NOTFOUND NOTFOUND)
    message(----------)
    message(${OPENSSL_LIBRARIES})
    message(----------)
    endif ()        
    sagase_configure_report (OPENSSL)
endmacro (configure_openssl)

macro (configure_protobuf)
    sagase_configure_package(PROTOBUF
        NAMES google protobuf
        COMPONENTS protobuf libprotobuf
        PREFIXES ${ENV_TUNDRA_DEP_PATH}/protobuf)
    # Force include dir and libraries on MSVC
    if (MSVC)
          set (PROTOBUF_INCLUDE_DIRS ${ENV_TUNDRA_DEP_PATH}/protobuf/include)
    endif ()
    sagase_configure_report (PROTOBUF)
    
endmacro (configure_protobuf)

macro (configure_celt)
    sagase_configure_package(CELT
        NAMES celt
        COMPONENTS celt0 celt celt # for celt.h
        PREFIXES ${ENV_TUNDRA_DEP_PATH}/celt)
    sagase_configure_report (CELT)
endmacro (configure_celt)

macro(use_package_knet)
    if ("$ENV{KNET_DIR_QT47}" STREQUAL "")
       set(KNET_DIR ${ENV_TUNDRA_DEP_PATH}/kNet)
    else()           
       set(KNET_DIR $ENV{KNET_DIR_QT47})
    endif()
    include_directories(${KNET_DIR}/include)
    link_directories(${KNET_DIR}/lib)
    if (UNIX)    
        add_definitions(-DUNIX)
    endif()
endmacro()

macro(link_package_knet)
    target_link_libraries(${TARGET_NAME} debug kNet)
    target_link_libraries(${TARGET_NAME} optimized kNet)
endmacro()

macro(use_package_bullet)
    if (WIN32)
        if ("$ENV{BULLET_DIR}" STREQUAL "")
            set(BULLET_DIR ${ENV_TUNDRA_DEP_PATH}/Bullet)
        endif()
        include_directories(${BULLET_DIR}/include)
        link_directories(${BULLET_DIR}/lib)
    else() # Linux, note: mac will also come here..
        if ("$ENV{BULLET_DIR}" STREQUAL "")
            set(BULLET_DIR ${ENV_TUNDRA_DEP_PATH})
        endif()
        include_directories(${BULLET_DIR}/include/bullet)
        link_directories(${BULLET_DIR}/lib)
    endif()
endmacro()

macro(link_package_bullet)
    target_link_libraries(${TARGET_NAME} optimized LinearMath optimized BulletDynamics optimized BulletCollision)
    if (WIN32)
        target_link_libraries(${TARGET_NAME} debug LinearMath_d debug BulletDynamics_d debug BulletCollision_d)
    endif()
endmacro()

macro(use_package_assimp)
    if (WIN32)
        if ("$ENV{ASSIMP_DIR}" STREQUAL "")
           set(ASSIMP_DIR ${ENV_TUNDRA_DEP_PATH}/assimp)
        endif()
        include_directories(${ASSIMP_DIR}/include)
        link_directories(${ASSIMP_DIR}/lib/assimp_debug_Win32)
        link_directories(${ASSIMP_DIR}/lib/assimp_release_Win32)
    else() # Linux, note: mac will also come here..
        if ("$ENV{ASSIMP_DIR}" STREQUAL "")
           set(ASSIMP_DIR ${ENV_TUNDRA_DEP_PATH})
        endif()
        include_directories(${ASSIMP_DIR}/include/assimp)
        link_directories(${ASSIMP_DIR}/lib)
    endif()
endmacro()

macro(link_package_assimp)
    target_link_libraries(${TARGET_NAME} optimized assimp)
    if (WIN32)
        target_link_libraries(${TARGET_NAME} debug assimpd)
    endif()
endmacro()
