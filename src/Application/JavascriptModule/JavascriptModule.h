/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   JavascriptModule.h
 *  @brief  Enables Javascript execution and scripting by using QtScript.
 */

#pragma once

#include "IModule.h"

#include "AttributeChangeType.h"

#include "AssetFwd.h"
#include "SceneFwd.h"
#include "JavascriptFwd.h"

#include <QObject>
#include <QString>
#include <QVariantMap>

class JavascriptInstance;

/// Enables Javascript execution and scripting by using QtScript.
class JavascriptModule : public IModule
{
    Q_OBJECT

public:
    JavascriptModule();
    ~JavascriptModule();

    void Load();
    void Initialize();
    void PostInitialize();
    void Uninitialize();
    void Update(f64 frametime);

    void RunScript(const QString &scriptname);
    void RunString(const QString &codestr, const QVariantMap &context = QVariantMap());

    /// Prepares script instance by registering all needed services to it.
    /** If script is part of the scene, i.e. EC_Script component is present, we add some special services.
        @param instance Script istance.
        @param comp Script component, null by default. */
    void PrepareScriptInstance(JavascriptInstance* instance, EC_Script *comp = 0);

public slots:
    /// New scene has been added to foundation.
    void SceneAdded(const QString &name);

    /// New component has been added to scene.
    void ComponentAdded(Entity* entity, IComponent* comp, AttributeChange::Type change);

    /// Component has been removed from scene.
    void ComponentRemoved(Entity* entity, IComponent* comp, AttributeChange::Type change);

signals:
    /// A script engine has been created
    /** The purpose of this is to allow dynamic service objects (registered with FrameWork->RegisterDynamicObject())
        to perform further scriptengine initialization, such as registration of new datatypes. The slot
        OnScriptEngineCreated() will be invoked on the dynamic service object, if it exists. */
    void ScriptEngineCreated(QScriptEngine* engine);

private:
    /// Parses the plugin startup configuration file to detect which startup scripts should be run.
    QStringList ParseStartupScriptConfig();

    /// Load & execute startup scripts
    /** Destroys old scripts if they exist */
    void LoadStartupScripts();

    /// Stop & delete startup scripts
    void UnloadStartupScripts();

    /// Parse the appname and classname from an EC_Script
    void ParseAppAndClassName(EC_Script* instance, QString& appName, QString& className);

    /// Find a named script application
    EC_Script* FindScriptApplication(EC_Script* instance, const QString& appName);

    /// Create a script class instance into a script application
    void CreateScriptObject(EC_Script* app, EC_Script* instance, const QString& className);

    /// Remove a script class instance from an EC_Script
    void RemoveScriptObject(EC_Script* instance);
    
    /// Create script class instances for all EC_Scripts depending on this script application
    void CreateScriptObjects(EC_Script* app);

    /// Remove script class instances for all EC_Scripts depending on this script application
    void RemoveScriptObjects(JavascriptInstance* jsInstance);

    /// Default engine for console & commandline script execution
    QScriptEngine *engine;

    /// Engines for executing startup (possibly persistent) scripts
    std::vector<JavascriptInstance *> startupScripts_;

    /// Additional startupscript defined from command line
    std::string commandLineStartupScript_;

private slots:
    void ConsoleRunString(const QStringList &params);
    void ConsoleRunFile(const QStringList &params);
    void ConsoleReloadScripts();
    
    void ScriptAssetsChanged(const std::vector<ScriptAssetPtr>& newScripts);
    void ScriptAppNameChanged(const QString& newAppName);
    void ScriptClassNameChanged(const QString& newClassName);
    void ScriptEvaluated();
    void ScriptUnloading();
};

// API things
QScriptValue Print(QScriptContext *context, QScriptEngine *engine);
QScriptValue ScriptRunFile(QScriptContext *context, QScriptEngine *engine);

