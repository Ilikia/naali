// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#include "AssetModuleApi.h"
#include "IAssetStorage.h"

class QFileSystemWatcher;
class AssetAPI;

namespace Asset
{

/// Represents a single (possibly recursive) directory on the local file system.
class ASSET_MODULE_API LocalAssetStorage : public IAssetStorage
{
Q_OBJECT

public:
    LocalAssetStorage();
    ~LocalAssetStorage();

    /// Specifies the absolute path of the storage.
    QString directory;

    /// Specifies a human-readable name for this storage.
    QString name;

    /// If true, all subdirectories of the storage directory are automatically looked in when loading an asset.
    bool recursive;

    /// Starts listening on the local directory this asset storage points to.
    void SetupWatcher();

    /// Stops and deallocates the directory change listener.
    void RemoveWatcher();

    /// Load all assets of specific suffix
    void LoadAllAssetsOfType(AssetAPI *assetAPI, const QString &suffix, const QString &assetType);

    QStringList assetRefs;
    
public slots:
    bool Writable() const { return true; }

    /// Returns the full local filesystem path name of the given asset in this storage, if it exists.
    /// Example: GetFullPathForAsset("my.mesh", true) might return "C:\Projects\Tundra\bin\data\assets".
    /// If the file does not exist, returns "".
    QString GetFullPathForAsset(const QString &assetname, bool recursive);

    /// Returns the URL that should be used in a scene asset reference attribute to refer to the asset with the given localName.
    /// Example: GetFullAssetURL("my.mesh") might return "local://my.mesh".
    /// @note LocalAssetStorage ignores all subdirectory specifications, so GetFullAssetURL("data/assets/my.mesh") would also return "local://my.mesh".
    QString GetFullAssetURL(const QString &localName);

    /// Returns the type of this storage: "LocalAssetStorage".
    virtual QString Type() const;

    /// Returns all assetrefs currently known in this asset storage. Does not load the assets
    virtual QStringList GetAllAssetRefs() { return assetRefs; }
    
    /// Refresh asset refs. Issues a directory query and emits AssetRefsChanged immediately
    virtual void RefreshAssetRefs();
    
//    QFileSystemWatcher *changeWatcher;

    QString Name() const { return name; }

    QString BaseURL() const { return "local://"; }

    /// Returns a convenient human-readable representation of this storage.
    QString ToString() const { return Name() + " (" + directory + ")"; }

    /// Serializes this storage to a string for machine transfer.
    virtual QString SerializeToString() const;

private:
    void operator=(const LocalAssetStorage &);
    LocalAssetStorage(const LocalAssetStorage &);
};

}

