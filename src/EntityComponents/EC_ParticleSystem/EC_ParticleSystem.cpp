// For conditions of distribution and use, see copyright notice in license.txt

#include "DebugOperatorNew.h"
#include "EC_ParticleSystem.h"
#include "IModule.h"
#include "Entity.h"
#include "Renderer.h"
#include "EC_Placeable.h"
#include "OgreParticleAsset.h"
#include "OgreConversionUtils.h"
#include "OgreRenderingModule.h"
#include "OgreWorld.h"
#include "Scene.h"
#include "AssetAPI.h"
#include "IAssetTransfer.h"
#include "LoggingFunctions.h"
#include "AttributeMetadata.h"
#include "Framework.h"

#include <Ogre.h>
#include "MemoryLeakCheck.h"

using namespace OgreRenderer;

EC_ParticleSystem::EC_ParticleSystem(Scene* scene):
    IComponent(scene),
    particleRef(this, "Particle ref" ),
    castShadows(this, "Cast shadows", false),
    renderingDistance(this, "Rendering distance", 0.0f)
{
    if (scene)
        world_ = scene->GetWorld<OgreWorld>();
    
    static AttributeMetadata particleRefMetadata;
    AttributeMetadata::ButtonInfoList particleRefButtons;
    particleRefButtons.push_back(AttributeMetadata::ButtonInfo(particleRef.Name(), "V", "View"));
    particleRefMetadata.buttons = particleRefButtons;
    particleRef.SetMetadata(&particleRefMetadata);

    connect(this, SIGNAL(ParentEntitySet()), this, SLOT(EntitySet()));
    connect(this, SIGNAL(AttributeChanged(IAttribute*, AttributeChange::Type)), this, SLOT(OnAttributeUpdated(IAttribute*)));
    
    particleAsset_ = AssetRefListenerPtr(new AssetRefListener());
    connect(particleAsset_.get(), SIGNAL(Loaded(AssetPtr)), this, SLOT(OnParticleAssetLoaded(AssetPtr)), Qt::UniqueConnection);
    connect(particleAsset_.get(), SIGNAL(TransferFailed(IAssetTransfer*, QString)), this, SLOT(OnParticleAssetFailed(IAssetTransfer*, QString)), Qt::UniqueConnection);
}

EC_ParticleSystem::~EC_ParticleSystem()
{
    DeleteParticleSystems();
}

void EC_ParticleSystem::View(const QString &attributeName)
{
    /// @todo add implementation.
}

void EC_ParticleSystem::CreateParticleSystem(const QString &systemName)
{
    if (!ViewEnabled())
        return;
    OgreWorldPtr world = world_.lock();
    if (!world)
        return;

    try
    {
        EC_Placeable *placeable = dynamic_cast<EC_Placeable *>(FindPlaceable().get());
        if (!placeable)
        {
            LogError("Fail to create a new particle system, make sure that entity has EC_Placeable component created.");
            return;
        }

        Ogre::SceneManager* sceneMgr = world->GetSceneManager();
        Ogre::ParticleSystem* system = sceneMgr->createParticleSystem(world->GetUniqueObjectName("EC_Particlesystem"), SanitateAssetIdForOgre(systemName.toStdString()));
        if (system)
        {
            placeable->GetSceneNode()->attachObject(system);
            particleSystems_.push_back(system);
            system->setCastShadows(castShadows.Get());
            system->setRenderingDistance(renderingDistance.Get());
            return;
        }
    }
    catch(Ogre::Exception& e)
    {
        LogError("Could not add particle system " + Name().toStdString() + ": " + std::string(e.what()));
    }

    return;
}

void EC_ParticleSystem::DeleteParticleSystems()
{
    OgreWorldPtr world = world_.lock();
    if (!world)
    {
        if (particleSystems_.size())
            LogError("EC_ParticleSystem: World has expired, skipping uninitialization!");
        return;
    }
    
    Ogre::SceneManager* sceneMgr = world->GetSceneManager();
    if (!sceneMgr)
        return;

    try
    {
        EC_Placeable *placeable = dynamic_cast<EC_Placeable *>(FindPlaceable().get());
        if (placeable)
        {
            Ogre::SceneNode *node = placeable->GetSceneNode();
            if (!node)
                return;
            for(unsigned i = 0; i < particleSystems_.size(); ++i)
                node->detachObject(particleSystems_[i]);
        }
        for(unsigned i = 0; i < particleSystems_.size(); ++i)
            sceneMgr->destroyParticleSystem(particleSystems_[i]);
    }
    catch(Ogre::Exception& /*e*/)
    {
        LogError("Could not delete particle systems");
    }
    
    particleSystems_.clear();
    
    return;
}

void EC_ParticleSystem::OnAttributeUpdated(IAttribute *attribute)
{
    if(attribute == &castShadows)
    {
        for(unsigned i = 0; i < particleSystems_.size(); ++i)
            particleSystems_[i]->setCastShadows(castShadows.Get());
    }
    else if (attribute == &renderingDistance)
    {
        for(unsigned i = 0; i < particleSystems_.size(); ++i)
            particleSystems_[i]->setRenderingDistance(renderingDistance.Get());
    }
    else if (attribute == &particleRef)
    {
        if (!ViewEnabled())
            return;

        particleAsset_->HandleAssetRefChange(&particleRef);
    }
}

ComponentPtr EC_ParticleSystem::FindPlaceable() const
{
    if (ParentEntity())
        return ParentEntity()->GetComponent<EC_Placeable>();
    else
        return ComponentPtr();
}

void EC_ParticleSystem::OnParticleAssetLoaded(AssetPtr asset)
{
    assert(asset);
    if (!asset)
        return;

    OgreParticleAsset *particleAsset = dynamic_cast<OgreParticleAsset*>(asset.get());
    if (!particleAsset)
    {
        LogError("OnMaterialAssetLoaded: Material asset load finished for asset \"" +
            asset->Name().toStdString() + "\", but downloaded asset was not of type OgreParticleAsset!");
        return;
    }

    DeleteParticleSystems();
    
    for(int i = 0 ; i < particleAsset->GetNumTemplates(); ++i)
        CreateParticleSystem(particleAsset->GetTemplateName(i));
}

void EC_ParticleSystem::OnParticleAssetFailed(IAssetTransfer* asset, QString reason)
{
    DeleteParticleSystems();
    CreateParticleSystem("ParticleAssetLoadError");
}

void EC_ParticleSystem::EntitySet()
{
    Entity *entity = this->ParentEntity();
    if (!entity)
    {
        LogError("Failed to connect entity signals, component's parent entity is null");
        return;
    }
    
    QObject::connect(entity, SIGNAL(ComponentRemoved(IComponent*, AttributeChange::Type)), this, SLOT(OnComponentRemoved(IComponent*, AttributeChange::Type)), Qt::UniqueConnection);
}

void EC_ParticleSystem::OnComponentRemoved(IComponent *component, AttributeChange::Type change)
{
    // If the component is the Placeable, delete particle systems now for safety
    if (component->TypeName() == EC_Placeable::TypeNameStatic())
        DeleteParticleSystems();
}
