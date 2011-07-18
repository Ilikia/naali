// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#include "IComponent.h"
#include "CoreDefines.h"
#include "OgreModuleApi.h"

namespace OgreRenderer { class OgreRenderingModule; class CompositionHandler; };

/// Ogre compositor component
/**
<table class="header">
<tr>
<td>
<h2>OgreCompositor</h2>
Can be used to enable Ogre postprocessing effects.
Registered by OgreRenderer::OgreRenderingModule.

<b>Attributes</b>:
<ul>
<li>QString: compositorref
<div>Name of the compositor (Ogre resource name), f.ex. "HDR"</div>
<li>bool: enabled
<div>Enables or disables this compositor effect. Useful for when you don't want to recreate and delete the component just to enable / disable an effect.</div>
<li>int: priority
<div>Priority for the compositor. Lower values mean the compositor is rendered earlier. Use -1 to auto order. If there are more
than one compositor in the scene with the same priority, the order of the compositors is arbitrary.</div>
<li>QVariantList: parameters
<div>Key-value pair for shader parameters, separated with a '='. The value supports up to 4 floats (Vector4) separated by spaces.
F.ex. 'strength=1.2' or 'color=1 0 0 0.5'</div>
</ul>

<b>Exposes the following scriptable functions:</b>
<ul>
<li>...
</ul>

<b>Reacts on the following actions:</b>
<ul>
</ul>
</td>
</tr>

Does not emit any actions.

<b>Doesn't depend on any components</b>.
</table>
*/
class OGRE_MODULE_API EC_OgreCompositor : public IComponent
{
    Q_OBJECT
    
public:
    /// Do not directly allocate new components using operator new, but use the factory-based SceneAPI::CreateComponent functions instead.
    explicit EC_OgreCompositor(Scene* scene);

    virtual ~EC_OgreCompositor();

    Q_PROPERTY(bool enabled READ getenabled WRITE setenabled);
    DEFINE_QPROPERTY_ATTRIBUTE(bool, enabled);

    Q_PROPERTY(QString compositorref READ getcompositorref WRITE setcompositorref);
    DEFINE_QPROPERTY_ATTRIBUTE(QString, compositorref);

    Q_PROPERTY(int priority READ getpriority WRITE setpriority);
    DEFINE_QPROPERTY_ATTRIBUTE(int, priority);

    Q_PROPERTY(QVariantList parameters READ getparameters WRITE setparameters);
    DEFINE_QPROPERTY_ATTRIBUTE(QVariantList, parameters);

    COMPONENT_NAME("EC_OgreCompositor", 18)
public slots:

private slots:
    void OnAttributeUpdated(IAttribute* attribute);

    void OneTimeRefresh();
    
private:
    /// Enables or disables and sets the priority of the specified compositor based on the attributes
    void UpdateCompositor(const QString &compositor);

    /// Updates compositor shader parameters
    void UpdateCompositorParams(const QString &compositor);

    /// Owner module of this component
    OgreRenderer::OgreRenderingModule *owner_;
    /// Compositor handler. Used to actually add / remove post process effects.
    OgreRenderer::CompositionHandler *handler_;
    /// Stored compositor ref for internal use
    QString previous_ref_;
    /// Stored previous priority for internal use
    int previous_priority_;
};

