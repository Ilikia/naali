// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#include "TundraLogicModuleApi.h"
#include "SceneFwd.h"
#include "AttributeChangeType.h"
#include "Math/float3.h"
#include "Math/Quat.h"
#include <QPair>

#include <map>

class QDomElement;
class Transform;

namespace TundraLogic
{

/// Stores information about material script.
struct MaterialInfo
{
    QString source; ///< Source file of the material script.
    QString name; ///< Name of the material.
    QString data; ///< Data (the actual material script).

    /// Less than operator. Compares source and name.
    bool operator <(const MaterialInfo &rhs) const
    {
        if (source < rhs.source) return true; else if (source > rhs.source) return false;
        if (name < rhs.name) return true; else if (name > rhs.name) return false;
        return false;
    }
};

typedef std::set<MaterialInfo> MaterialInfoList; ///< Set of MaterialInfo structs.

/// Importer tool for OGRE .scene and .mesh files.
/** You can use SceneImporter to directly create and instantiate content from OGRE .mesh and .scene files,
    or to create scene descriptions for the aforementioned file formats. The scene descriptions can be modified
    and then instantiated using Scene::CreateContentFromSceneDesc().
    SceneImporter contains also useful OGRE material file utility functions. */
class TUNDRALOGIC_MODULE_API SceneImporter
{
public:
    /// Constructs the importer.
    /** @param scene Destination scene. */
    explicit SceneImporter(const ScenePtr &scene);

    /// Destroys the importer.
    ~SceneImporter();

    /// Imports a single mesh. Scans the mesh for needed skeleton & materials.
    /** @param filename Filename of mesh
        @param in_asset_dir Where to read input assets. Typically same as the input file path
        @param out_asset_dir Where to put resulting assets
        @param worldtransform Transform to use for the entity's placeable.
            You can use Transform's default ctor if you don't want to spesify custom Transform.
        @param entity_prefab_xml Prefab data (entity & components in xml serialized format) to use for the entity
        @param prefix 
        @param change What changetype to use in scene operations
        @param inspect Load and inspect mesh for materials and skeleton
        @param meshName Name of mesh inside the file
        @return Entity pointer if successful, null if failed. */
    EntityPtr ImportMesh(const std::string& filename, std::string in_asset_dir, const Transform &worldtransform,
        const std::string& entity_prefab_xml, const QString &prefix, AttributeChange::Type change, bool inspect = true,
        const std::string &meshName = std::string());

    /// Imports a dotscene.
    /** @param filename Input filename
        @param in_asset_dir Where to read input assets. Typically same as the input file path
        @param worldtransform Transform to use for the entity's placeable
        @param prefix 
        @param change What changetype to use in scene operations
        @param clearscene Whether to clear scene first. Default false
        @param replace Whether to search for entities by name and replace just the visual components (placeable, mesh) if an existing entity is found.
               Default true. If this is false, all entities will be created as new
        @return List of created entities, or an empty list if import failed. */
    QList<Entity *> Import(const std::string& filename, std::string in_asset_dir, const Transform &worldtransform,
        const QString &prefix, AttributeChange::Type change, bool clearscene = false, bool replace = true);

    /// Parse a mesh for materials & skeleton ref
    /** @param meshname Full path & filename of mesh
        @param material_names Return vector for material names
        @param skeleton_name Return value for skeleton ref */
    bool ParseMeshForMaterialsAndSkeleton(const QString& meshname, QStringList& material_names, QString& skeleton_name) const;

    /// Inspects OGRE .mesh file and returns a scene description structure of the contents of the file.
    /** @param source Mesh source, can be filename or an URL.
        @note For URLs, this will not dowload the binary data but only adds an entity with the URL mesh ref in it.*/
    SceneDesc CreateSceneDescFromMesh(const QString &source) const;

    /// Inspects OGRE .scene file and returns a scene description structure of the contents of the file.
    /** @param filename Filename. */
    SceneDesc CreateSceneDescFromScene(const QString &filename);

    /// Process a material file, searching for used materials and recording used textures
    /** @param matfilename Material file name, including full path
        @param used_materials Set of materials that are needed. Any materials in the file that are not listed will be skipped
        @return Set of used textures */
    QSet<QString> ProcessMaterialFileForTextures(const QString& matfilename, const QSet<QString>& used_materials) const;

    /// Process material script and searches for texture references.
    /** @param material Material script.
        @return Set of used texture references/names. */
    QSet<QString> ProcessMaterialForTextures(const QString &material) const;

    /// Loads single material script from material file and returns it as a string.
    /** @param filename Filename.
        @param materialName Material name.
        @return Material script as a string, or an empty string if material was not found. */
    QString LoadSingleMaterialFromFile(const QString &filename, const QString &materialName) const;

    /// Loads all (uniquely named) material scripts found within a material script file.
    /** @param filename Filename.
        @param materialNames Names of materials to be loaded.
        @return List of material names - material script pairs as strings. */
    MaterialInfoList LoadAllMaterialsFromFile(const QString &filename) const;

    /// Searches directory recursively and returns list of found material files.
    /** @param dir Directory to be searched. */
    QStringList FindMaterialFiles(const std::string &dir) const;

private:
    /// Process the asset references of a node, and its child nodes
    /** @param node_elem Node element. */
    void ProcessNodeForAssets(QDomElement node_elem, const std::string& in_asset_dir);

    /// Process node and its child nodes for creation of entities & components. Done after asset pass
    /** @param [out] entities List of created entities
        @param node_elem Node element
        @param pos Current position
        @param rot Current rotation
        @param scale Current scale
        @param change What changetype to use in scene operations
        @param flipyz Whether to switch y/z axes from Ogre to OpenSim convention
        @param prefix 
        @param replace Whether to replace contents of entities by name. If false, all entities will be created as new. */
    void ProcessNodeForCreation(QList<Entity *> &entities, QDomElement node_elem, float3 pos, Quat rot, float3 scale,
        AttributeChange::Type change, const QString &prefix, bool flipyz, bool replace);

    /// Process node and its child nodes for creation of scene description.
    /** @param desc 
        @param node_elem Node element
        @param pos Current position
        @param rot Current rotation
        @param scale Current scale
        @param prefix 
        @param flipyz Whether to switch y/z axes from Ogre to OpenSim convention
    */
//    void ProcessNodeForDesc(SceneDesc &desc, QDomElement node_elem, float3 pos, Quaternion rot, float3 scale,
//        const QString &prefix, bool flipyz);

    /// Creates asset descriptions from the provided lists of OGRE resource filenames.
    /** @param path 
        @param meshFiles 
        @param skeletons 
        @param materialFiles 
        @param usedMaterials 
        @param desc */
    void CreateAssetDescs(const QString &path, const QStringList &meshFiles, const QStringList &skeletons,
        const QStringList &materialFiles, const QSet<QString> &usedMaterials, SceneDesc &desc) const;

    ///
    /** @param filename
        @param ref
    */
//    void RewriteAssetRef(const QString &sceneFileName, QString &ref) const;

    QMap<QString, QStringList> mesh_default_materials_; ///< Materials read from meshes, in case of no subentity elements
    std::set<std::string> material_names_; /// Materials encountered in scene
    std::set<std::string> node_names_; ///< Nodes already created into the scene. Used for name-based "update import" logic
    ScenePtr scene_; ///< Destination scene.

    /// Meshes encountered in scene
    /** For supporting binary duplicate detection, this is a map which maps the original names to actual assets that will be stored. */
    std::map<std::string, std::string> mesh_names_;
};

}

