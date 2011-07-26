#include "StableHeaders.h"
#include "PluginAPI.h"
#include "LoggingFunctions.h"
#include "Framework.h"
#include "Application.h"

#include <QtXml>
#include <iostream>
#include <vector>

std::string WStringToString(const std::wstring &str)
{
    std::vector<char> c((str.length()+1)*4);
    wcstombs(&c[0], str.c_str(), c.size()-1);
    return &c[0];
}

std::string GetErrorString(int error)
{
#ifdef WIN32
	void *lpMsgBuf = 0;

	HRESULT hresult = HRESULT_FROM_WIN32(error);
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		0, hresult, 0 /*Default language*/, (LPTSTR) &lpMsgBuf, 0, 0);

	// Copy message to C++ -style string, since the data need to be freed before return.
#ifdef UNICODE
	std::wstringstream ss;
#else
	std::stringstream ss;
#endif
	ss << (LPTSTR)lpMsgBuf << "(" << error << ")";
	LocalFree(lpMsgBuf);
#ifdef UNICODE
	return WStringToString(ss.str());
#else
	return ss.str();
#endif

#else
	std::stringstream ss;
	ss << strerror(error) << "(" << error << ")";
	return ss.str();
#endif
}


PluginAPI::PluginAPI(Framework *owner_)
:owner(owner_)
{
}

typedef void (*TundraPluginMainSignature)(Framework *owner);

void PluginAPI::LoadPlugin(const QString &filename)
{
#ifdef WIN32
  #ifdef _DEBUG
    const QString pluginSuffix = "d.dll";
  #else
    const QString pluginSuffix = ".dll";
  #endif
#elif defined(_POSIX_C_SOURCE)
    const QString pluginSuffix = ".so";
#elif defined(__APPLE__)
    const QString pluginSuffix = ".dylib";
#endif


    LogInfo("Loading plugin '" + filename + "'");
    owner->GetApplication()->SetSplashMessage("Loading plugin " + filename);
    QString path = Application::InstallationDirectory() + "plugins/" + filename.trimmed() + pluginSuffix;
    
    ///\todo Unicode support!
#ifdef WIN32
    path = path.replace("/", "\\");
    HMODULE module = LoadLibraryA(path.toStdString().c_str());
    if (module == NULL)
    {
        DWORD errorCode = GetLastError(); ///\todo ToString.
        LogError("Failed to load plugin from file \"" + path + "\": Error " + GetErrorString(errorCode).c_str() + "!");
        return;
    }
    TundraPluginMainSignature mainEntryPoint = (TundraPluginMainSignature)GetProcAddress(module, "TundraPluginMain");
    if (mainEntryPoint == NULL)
    {
        DWORD errorCode = GetLastError(); ///\todo ToString.
        LogError("Failed to find plugin startup function 'TundraPluginMain' from plugin file \"" + path + "\": Error " + GetErrorString(errorCode).c_str() + "!");
        return;
    }

#else
    char *dlerrstr;
    dlerror();
    PluginHandle module = dlopen(path.toStdString().c_str(), RTLD_GLOBAL|RTLD_LAZY);
    if (dlerrstr=dlerror())
    {
        LogError("Failed to load plugin from file \"" + path + "\": Error " + dlerrstr + "!");
        return;
    }

    dlerror();
    TundraPluginMainSignature mainEntryPoint = (TundraPluginMainSignature)dlsym(module, "TundraPluginMain");
    if (dlerrstr=dlerror())
    {
        LogError("Failed to find plugin startup function 'TundraPluginMain' from plugin file \"" + path + "\": Error " + dlerrstr + "!");
        return;
    }

#endif
    Plugin p = { module };
    plugins.push_back(p);
    mainEntryPoint(owner);
}

void PluginAPI::UnloadPlugins()
{
#ifdef WIN32
    for(std::list<Plugin>::reverse_iterator iter = plugins.rbegin(); iter != plugins.rend(); ++iter)
        FreeLibrary(iter->libraryHandle);
#endif
    plugins.clear();
}

QString LookupRelativePath(QString path)
{
    // If a relative path was specified, lookup from cwd first, then from application installation directory.
    if (QDir::isRelativePath(path))
    {
        QString cwdPath = Application::CurrentWorkingDirectory() + path;
        if (QFile::exists(cwdPath))
            return cwdPath;
        else
            return Application::InstallationDirectory() + path;
    }
    else
        return path;
}

QString PluginAPI::ConfigurationFile() const
{
    boost::program_options::variables_map &commandLineVariables = owner->ProgramOptions();
    QString configFilename = "plugins.xml";
    if (commandLineVariables.count("config") > 0)
        configFilename = commandLineVariables["config"].as<std::string>().c_str();
    
    return LookupRelativePath(configFilename);
}

void PluginAPI::LoadPluginsFromXML(QString pluginConfigurationFile)
{
    pluginConfigurationFile = LookupRelativePath(pluginConfigurationFile);

    QDomDocument doc("plugins");
    QFile file(pluginConfigurationFile);
    if (!file.open(QIODevice::ReadOnly))
    {
        LogError("PluginAPI::LoadPluginsFromXML: Failed to open file \"" + pluginConfigurationFile + "\"!");
        return;
    }
    if (!doc.setContent(&file))
    {
        LogError("PluginAPI::LoadPluginsFromXML: Failed to parse XML file \"" + pluginConfigurationFile + "\"!");
        file.close();
        return;
    }
    file.close();

    QDomElement docElem = doc.documentElement();

    QDomNode n = docElem.firstChild();
    while(!n.isNull())
    {
        QDomElement e = n.toElement(); // try to convert the node to an element.
        if (!e.isNull() && e.tagName() == "plugin" && e.hasAttribute("path"))
        {
            QString pluginPath = e.attribute("path");
            LoadPlugin(pluginPath);
        }
        n = n.nextSibling();
    }
}
