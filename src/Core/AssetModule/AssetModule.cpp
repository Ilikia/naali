// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "AssetModule.h"
#include "LocalAssetProvider.h"
#include "HttpAssetProvider.h"
#include "HttpAssetStorage.h"
#include "Framework.h"
#include "Profiler.h"
#include "CoreException.h"
#include "AssetAPI.h"
#include "LocalAssetStorage.h"
#include "ConsoleAPI.h"
#include "Application.h"

#include "KristalliProtocolModule.h"
#include "TundraLogicModule.h"
#include "TundraMessages.h"
#include "Client.h"
#include "Server.h"
#include "UserConnectedResponseData.h"
#include "UserConnection.h"
#include "MsgAssetDeleted.h"
#include "MsgAssetDiscovery.h"

#include "kNetBuildConfig.h"
#include "kNet/MessageConnection.h"

#include <QDir>
#include "MemoryLeakCheck.h"

namespace Asset
{
    AssetModule::AssetModule()
    :IModule("Asset")
    {
    }

    AssetModule::~AssetModule()
    {
    }

    void AssetModule::Initialize()
    {
        boost::shared_ptr<HttpAssetProvider> http = boost::shared_ptr<HttpAssetProvider>(new HttpAssetProvider(framework_));
        framework_->Asset()->RegisterAssetProvider(boost::dynamic_pointer_cast<IAssetProvider>(http));
        
        boost::shared_ptr<LocalAssetProvider> local = boost::shared_ptr<LocalAssetProvider>(new LocalAssetProvider(framework_));
        framework_->Asset()->RegisterAssetProvider(boost::dynamic_pointer_cast<IAssetProvider>(local));
        
        QString systemAssetDir = Application::InstallationDirectory() + "data/assets";
        local->AddStorageDirectory(systemAssetDir, "System", true);
        // Set asset dir as also as AssetAPI property
        framework_->Asset()->setProperty("assetdir", systemAssetDir);
        framework_->Asset()->setProperty("inbuiltassetdir", systemAssetDir);
        
        QString jsAssetDir = Application::InstallationDirectory() + "jsmodules";
        local->AddStorageDirectory(jsAssetDir, "Javascript", true);

        QString ogreAssetDir = Application::InstallationDirectory() + "media";
        local->AddStorageDirectory(ogreAssetDir, "Ogre Media", true);

        framework_->RegisterDynamicObject("assetModule", this);
    }

    void AssetModule::PostInitialize()
    {
        framework_->Console()->RegisterCommand(
            "RequestAsset", "Request asset from server. Usage: RequestAsset(uuid,assettype)", 
            this, SLOT(ConsoleRequestAsset(const QString &, const QString &)));

        framework_->Console()->RegisterCommand(
            "AddAssetStorage", "Usage: AddAssetStorage(storage string). For example: AddAssetStorage(name=MyAssets;type=HttpAssetStorage;src=http://www.myserver.com/;default;)", 
            this, SLOT(AddAssetStorage(const QString &)));

        framework_->Console()->RegisterCommand(
            "ListAssetStorages", "Serializes all currently registered asset storages to the console output log.", 
            this, SLOT(ListAssetStorages()));

        framework_->Console()->RegisterCommand(
            "RefreshHttpStorages", "Refreshes known assetrefs for all http asset storages", 
            this, SLOT(ConsoleRefreshHttpStorages()));

        ProcessCommandLineOptions();

        TundraLogic::Server *server = framework_->GetModule<TundraLogic::TundraLogicModule>()->GetServer().get();
        connect(server, SIGNAL(UserConnected(int, UserConnection *, UserConnectedResponseData *)), this, 
            SLOT(ServerNewUserConnected(int, UserConnection *, UserConnectedResponseData *)));

        TundraLogic::Client *client = framework_->GetModule<TundraLogic::TundraLogicModule>()->GetClient().get();
        connect(client, SIGNAL(Connected(UserConnectedResponseData *)), this, SLOT(ClientConnectedToServer(UserConnectedResponseData *)));
        connect(client, SIGNAL(Disconnected()), this, SLOT(ClientDisconnectedFromServer()));

        KristalliProtocol::KristalliProtocolModule *kristalli = framework_->GetModule<KristalliProtocol::KristalliProtocolModule>();
        connect(kristalli, SIGNAL(NetworkMessageReceived(kNet::MessageConnection *, kNet::message_id_t, const char *, size_t)), 
            this, SLOT(HandleKristalliMessage(kNet::MessageConnection*, kNet::message_id_t, const char*, size_t)), Qt::UniqueConnection);

        // Connect to asset uploads & deletions from storage to be able to broadcast asset discovery & deletion messages
        connect(framework_->Asset(), SIGNAL(AssetUploaded(const QString &)), this, SLOT(OnAssetUploaded(const QString &)));
        connect(framework_->Asset(), SIGNAL(AssetDeletedFromStorage(const QString&)), this, SLOT(OnAssetDeleted(const QString&)));
    }

    void AssetModule::ProcessCommandLineOptions()
    {
        assert(framework_);

        bool hasFile = framework_->HasCommandLineParameter("--file");
        bool hasStorage = framework_->HasCommandLineParameter("--storage");
        QStringList files = framework_->CommandLineParameters("--file");
        QStringList storages = framework_->CommandLineParameters("--storage");
        if (hasFile && files.isEmpty())
            LogError("AssetModule: --file specified without a value.");
        if (hasStorage && storages.isEmpty())
            LogError("AssetModule: --storage specified without a value.");
        foreach(const QString &file, files)
        {
            AssetStoragePtr storage = framework_->Asset()->DeserializeAssetStorageFromString(file.trimmed());
            framework_->Asset()->SetDefaultAssetStorage(storage);
        }
        foreach(const QString &storageName, storages)
        {
            AssetStoragePtr storage = framework_->Asset()->DeserializeAssetStorageFromString(storageName.trimmed());
            if (files.isEmpty()) // If "--file" was not specified, then use "--storage" as the default. (If both are specified, "--file" takes precedence over "--storage").
                framework_->Asset()->SetDefaultAssetStorage(storage);
        }
    }

    void AssetModule::ConsoleRefreshHttpStorages()
    {
        RefreshHttpStorages();
    }
    
    void AssetModule::ConsoleRequestAsset(const QString &assetRef, const QString &assetType)
    {
        AssetTransferPtr transfer = framework_->Asset()->RequestAsset(assetRef, assetType);
    }

    void AssetModule::AddAssetStorage(const QString &storageString)
    {
        AssetStoragePtr storage = framework_->Asset()->DeserializeAssetStorageFromString(storageString);
    }

    void AssetModule::ListAssetStorages()
    {
        LogInfo("Registered storages: ");
        foreach(const AssetStoragePtr &storage, framework_->Asset()->GetAssetStorages())
        {
            QString storageString = storage->SerializeToString();
            if (framework_->Asset()->GetDefaultAssetStorage() == storage)
                storageString += ";default";
            LogInfo(storageString.toStdString());
        }
    }

    void AssetModule::LoadAllLocalAssetsWithSuffix(const QString &suffix, const QString &assetType)
    {
        foreach(const AssetStoragePtr &s, framework_->Asset()->GetAssetStorages())
        {
            LocalAssetStorage *storage = dynamic_cast<LocalAssetStorage*>(s.get());
            if (storage)
                storage->LoadAllAssetsOfType(framework_->Asset(), suffix, assetType);
        }
    }
    
    void AssetModule::RefreshHttpStorages()
    {
        foreach(const AssetStoragePtr &s, framework_->Asset()->GetAssetStorages())
        {
            HttpAssetStorage *storage = dynamic_cast<HttpAssetStorage*>(s.get());
            if (storage)
                storage->RefreshAssetRefs();
        }
    }

    void AssetModule::ServerNewUserConnected(int connectionID, UserConnection *connection, UserConnectedResponseData *responseData)
    {
        QDomDocument &doc = responseData->responseData;
        QDomElement assetRoot = doc.createElement("asset");
        doc.appendChild(assetRoot);
        
        bool isLocalhostConnection = (connection->connection->RemoteEndPoint().IPToString() == "127.0.0.1" || 
            connection->connection->LocalEndPoint().IPToString() == connection->connection->RemoteEndPoint().IPToString());

        std::vector<AssetStoragePtr> storages = framework_->Asset()->GetAssetStorages();
        for(size_t i = 0; i < storages.size(); ++i)
        {
            bool isLocalStorage = (dynamic_cast<LocalAssetStorage*>(storages[i].get()) != 0);
            if (!isLocalStorage || isLocalhostConnection)
            {
                QDomElement storage = doc.createElement("storage");
                storage.setAttribute("data", storages[i]->SerializeToString());
                assetRoot.appendChild(storage);
            }
        }
        AssetStoragePtr defaultStorage = framework_->Asset()->GetDefaultAssetStorage();
        bool defaultStorageIsLocal = (dynamic_cast<LocalAssetStorage*>(defaultStorage.get()) != 0);
        if (defaultStorage && (!defaultStorageIsLocal || isLocalhostConnection))
        {
            QDomElement storage = doc.createElement("defaultStorage");
            storage.setAttribute("name", defaultStorage->Name());
            assetRoot.appendChild(storage);
        }
    }

    void AssetModule::ClientConnectedToServer(UserConnectedResponseData *responseData)
    {
        QDomDocument &doc = responseData->responseData;
        QDomElement assetRoot = doc.firstChildElement("asset");
        if (!assetRoot.isNull())
        {
            for (QDomElement storage = assetRoot.firstChildElement("storage"); !storage.isNull(); 
                storage = storage.nextSiblingElement("storage"))
            {
                QString storageData = storage.attribute("data");
                AssetStoragePtr assetStorage = framework_->Asset()->DeserializeAssetStorageFromString(storageData);

                // Remember that this storage was received from the server, so we can later stop using it when we disconnect (and possibly reconnect to another server).
                if (assetStorage)
                    storagesReceivedFromServer.push_back(assetStorage);
            }

            QDomElement defaultStorage = assetRoot.firstChildElement("defaultStorage");
            if (!defaultStorage.isNull())
            {
                QString defaultStorageName = defaultStorage.attribute("name");
                AssetStoragePtr defaultStoragePtr = framework_->Asset()->GetAssetStorageByName(defaultStorageName);
                if (defaultStoragePtr)
                    framework_->Asset()->SetDefaultAssetStorage(defaultStoragePtr);
            }
        }
    }

    void AssetModule::ClientDisconnectedFromServer()
    {
        for(size_t i = 0; i < storagesReceivedFromServer.size(); ++i)
        {
            AssetStoragePtr storage = storagesReceivedFromServer[i].lock();
            if (storage)
                framework_->Asset()->RemoveAssetStorage(storage->Name());
        }
        storagesReceivedFromServer.clear();
    }

    void AssetModule::HandleKristalliMessage(kNet::MessageConnection* source, kNet::message_id_t id, const char* data, size_t numBytes)
    {
        switch (id)
        {
        case cAssetDiscoveryMessage:
            {
                MsgAssetDiscovery msg(data, numBytes);
                HandleAssetDiscovery(source, msg);
            }
            break;
        case cAssetDeletedMessage:
            {
                MsgAssetDeleted msg(data, numBytes);
                HandleAssetDeleted(source, msg);
            }
            break;
        }
    }

    void AssetModule::HandleAssetDiscovery(kNet::MessageConnection* source, MsgAssetDiscovery& msg)
    {
        QString assetRef = QString::fromStdString(BufferToString(msg.assetRef));
        QString assetType = QString::fromStdString(BufferToString(msg.assetType));
        
        // Check for possible malicious discovery message and ignore it. Otherwise let AssetAPI handle
        if (!framework_->Asset()->ShouldReplicateAssetDiscovery(assetRef))
            return;
        
        // If we are server, the message had to come from a client, and we replicate it to everyone except the sender
        TundraLogic::TundraLogicModule* tundra = framework_->GetModule<TundraLogic::TundraLogicModule>();
        KristalliProtocol::KristalliProtocolModule *kristalli = framework_->GetModule<KristalliProtocol::KristalliProtocolModule>();
        if (tundra->IsServer())
            foreach(UserConnection* userConn, kristalli->GetUserConnections())
                if (userConn->connection != source)
                    userConn->connection->Send(msg);

        // Then let assetAPI handle locally
        framework_->Asset()->HandleAssetDiscovery(assetRef, assetType);
    }

    void AssetModule::HandleAssetDeleted(kNet::MessageConnection* source, MsgAssetDeleted& msg)
    {
        QString assetRef = QString::fromStdString(BufferToString(msg.assetRef));
        
        // Check for possible malicious delete message and ignore it. Otherwise let AssetAPI handle
        if (!framework_->Asset()->ShouldReplicateAssetDiscovery(assetRef))
            return;
        
        // If we are server, the message had to come from a client, and we replicate it to everyone except the sender
        TundraLogic::TundraLogicModule* tundra = framework_->GetModule<TundraLogic::TundraLogicModule>();
        KristalliProtocol::KristalliProtocolModule *kristalli = framework_->GetModule<KristalliProtocol::KristalliProtocolModule>();
        if (tundra->IsServer())
            foreach(UserConnection* userConn, kristalli->GetUserConnections())
                if (userConn->connection != source)
                    userConn->connection->Send(msg);

        // Then let assetAPI handle locally
        framework_->Asset()->HandleAssetDeleted(assetRef);
    }

    void AssetModule::OnAssetUploaded(const QString& assetRef)
    {
        // Check whether the asset upload needs to be replicated
        if (!framework_->Asset()->ShouldReplicateAssetDiscovery(assetRef))
            return;
        
        TundraLogic::TundraLogicModule* tundra = framework_->GetModule<TundraLogic::TundraLogicModule>();
        KristalliProtocol::KristalliProtocolModule *kristalli = framework_->GetModule<KristalliProtocol::KristalliProtocolModule>();

        MsgAssetDiscovery msg;
        msg.assetRef = StringToBuffer(assetRef.toStdString());
        /// \todo Would preferably need the assettype as well
        
        // If we are server, send to everyone
        if (tundra->IsServer())
        {
            foreach(UserConnection* userConn, kristalli->GetUserConnections())
                userConn->connection->Send(msg);
        }
        // If we are client, send to server
        else
        {
            kNet::MessageConnection* connection = tundra->GetClient()->GetConnection();
            if (connection)
                connection->Send(msg);
        }
    }

    void AssetModule::OnAssetDeleted(const QString& assetRef)
    {
        // Check whether the asset delete needs to be replicated
        if (!framework_->Asset()->ShouldReplicateAssetDiscovery(assetRef))
            return;
        
        TundraLogic::TundraLogicModule* tundra = framework_->GetModule<TundraLogic::TundraLogicModule>();
        KristalliProtocol::KristalliProtocolModule *kristalli = framework_->GetModule<KristalliProtocol::KristalliProtocolModule>();

        MsgAssetDeleted msg;
        msg.assetRef = StringToBuffer(assetRef.toStdString());
        
        // If we are server, send to everyone
        if (tundra->IsServer())
        {
            foreach(UserConnection* userConn, kristalli->GetUserConnections())
                userConn->connection->Send(msg);
        }
        // If we are client, send to server
        else
        {
            kNet::MessageConnection* connection = tundra->GetClient()->GetConnection();
            if (connection)
                connection->Send(msg);
        }
    }

}

using namespace Asset;

extern "C"
{
DLLEXPORT void TundraPluginMain(Framework *fw)
{
    Framework::SetInstance(fw); // Inside this DLL, remember the pointer to the global framework object.
    IModule *module = new Asset::AssetModule();
    fw->RegisterModule(module);
}
}
