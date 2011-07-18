/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   IComponent.h
 *  @brief  Base class for all components. Inherit from this class when creating new components.
 */

#pragma once

#include "SceneFwd.h"
#include "AttributeChangeType.h"
#include "IAttribute.h"

#include <boost/enable_shared_from_this.hpp>

#include <QObject>

class QDomDocument;
class QDomElement;

class Framework;

/// Specifies the name and ID of this component.
/** This #define should be instantiated inside the public slots: section of the component.
    Warning: This #define alters the current visibility specifier in the class file. */
#define COMPONENT_NAME(componentName, componentTypeId)                                  \
public:                                                                                 \
    static const QString &TypeNameStatic()                                              \
    {                                                                                   \
        static const QString name(componentName);                                       \
        return name;                                                                    \
    }                                                                                   \
    static u32 TypeIdStatic()                                                     \
    {                                                                                   \
        return componentTypeId;                                                         \
    }                                                                                   \
public slots:                                                                           \
    virtual const QString &TypeName() const                                             \
    {                                                                                   \
        return TypeNameStatic();                                                        \
    }                                                                                   \
    virtual u32 TypeId() const                                                    \
    {                                                                                   \
        return componentTypeId;                                                         \
    }                                                                                   \
private: // Return the class visibility specifier to the strictest form so that the user most likely catches that this macro had to change the visibility.

//Q_PROPERTY(type attribute READ get##attribute WRITE set##attribute)
/// Exposes an existing 'Attribute<type> attribute' member as an automatically generated QProperty of name 'attribute'.
#define EXPOSE_ATTRIBUTE_AS_QPROPERTY(type, attribute) \
    type get##attribute() const { return (type)attribute.Get(); } \
    void set##atribute(type value) { attribute.Set((type)value, AttributeChange::Default); }

//Q_PROPERTY(type attribute READ get##attribute WRITE set##attribute)
/// Defines a new 'Attribute<type> attribute' member as an automatically generated QProperty of name 'attribute'.
#define DEFINE_QPROPERTY_ATTRIBUTE(type, attribute) \
    Attribute<type > attribute; \
    type get##attribute() const { return (type)attribute.Get(); } \
    void set##attribute(type value) { attribute.Set((type)value, AttributeChange::Default); }

/// The common interface for all components, which are the building blocks the scene entities are formed of.
/** Inherit your own components from this class.
    Each Component has a compile-time specified type name that identifies the class-name of the Component.
    This differentiates different derived implementations of the IComponent class. Each implemented Component
    must have a unique type name.

    Additionally, each Component has a Name string, which identifies different instances of the same Component,
    if more than one is added to an Entity.

    A Component consists of a list of Attributes, which are automatically replicatable instances of scene data.
    See IAttribute for more details.

    Every Component has a state variable 'UpdateMode' that specifies a default setting for managing which objects
    get notified whenever an Attribute change event occurs. This is used to create "Local Only"-objects as well
    as when doing batch updates of Attributes (for performance or correctness). */
class IComponent : public QObject, public boost::enable_shared_from_this<IComponent>
{
    Q_OBJECT
    Q_PROPERTY(QString name READ Name WRITE SetName)
    Q_PROPERTY(QString typeName READ TypeName)
    Q_PROPERTY(bool networkSyncEnabled READ NetworkSyncEnabled WRITE SetNetworkSyncEnabled)
    Q_PROPERTY(AttributeChange::Type updateMode READ UpdateMode WRITE SetUpdateMode)

public:
    /// Constructor.
    explicit IComponent(Scene* scene);

    /// Copy-constructor.
    IComponent(const IComponent& rhs);

    /// Destructor, does nothing.
    virtual ~IComponent();

    /// Returns the typename of this component.
    /** The typename is the "class" type of the component,
        e.g. "EC_Mesh" or "EC_DynamicComponent". The typename of a component cannot be an empty string.
        The typename of a component never changes at runtime. */
    virtual const QString &TypeName() const = 0;

    /// Returns the unique type ID of this component.
    virtual u32 TypeId() const = 0;

    /// Returns the name of this component.
    /** The name of a component is a custom user-specified name for
        this component instance, and identifies separate instances of the same component in an object. 
        The (TypeName, Name) pairs of all components in an Entity must be unique. The Name string can be empty. */
    const QString &Name() const { return name; }

    /// Sets the name of the component.
    /** This call will silently fail if there already exists a component with the
        same (TypeName, Name) pair in this entity. When this function changes the name of the component,
        the signal ComponentNameChanged is emitted.
        @param name The new name for this component. This may be an empty string. */
    void SetName(const QString& name);

    /// Stores a pointer of the Entity that owns this component into this component.
    /** This function is called at component initialization time to attach this component to its owning Entity.
        Although public, it is not intended to be called by users of IComponent. */
    void SetParentEntity(Entity* entity);

    /// Returns the list of all Attributes in this component for reflection purposes.
    const AttributeVector& Attributes() const { return attributes; }

    /// Finds and returns an attribute of type 'Attribute<T>' and given name.
    /** @param T The Attribute type to look for.
        @param name The name of the attribute.
        @return If there exists an attribute of type 'Attribute<T>' which has the given name, a pointer to
                that attribute is returned, otherwise returns null. */
    template<typename T>
    Attribute<T> *GetAttribute(const std::string &name) const ///\todo Replace std::string with QString to avoid std::string->QString conversion below!
    {
        for(size_t i = 0; i < attributes.size(); ++i)
            if (attributes[i]->Name() == QString(name.c_str()))
                return dynamic_cast<Attribute<T> *>(&attributes[i]);
        return 0;
    }

    /// Serializes this component and all its Attributes to the given XML document.
    /** @param doc The XML document to serialize this component to.
        @param baseElement Points to the <entity> element of the document doc. This element is the Entity that
                owns this component. This component will be serialized as a child tree of this element. */
    virtual void SerializeTo(QDomDocument& doc, QDomElement& baseElement) const;

    /// Deserializes this component from the given XML document.
    /** @param element Points to the <component> element that is the root of the serialized form of this Component.
        @param change Specifies the source of this change. This field controls whether the deserialization
                     was initiated locally and must be replicated to network, or if the change was received from
                     the network and only local application of the data suffices. */
    virtual void DeserializeFrom(QDomElement& element, AttributeChange::Type change);

    /// Serialize attributes to binary
    /** @note does not include syncmode, typename or name. These are left for higher-level logic, and
        it depends on the situation if they are needed or not */
    virtual void SerializeToBinary(kNet::DataSerializer& dest) const;

    /// Deserialize attributes from binary
    /** @note does not include syncmode, typename or name. These are left for higher-level logic, and
        it depends on the situation if they are needed or not. */
    virtual void DeserializeFromBinary(kNet::DataDeserializer& source, AttributeChange::Type change);

    /// Returns an Attribute of this component with the given @c name.
    /** This function iterates through the attribute vector and tries to find a member attribute with the given name.
        @param The name of the attribute to look for.
        @return A pointer to the attribute, or null if no attribute with the given name exists.

        \todo: was made a slot, but interfered with a slot with the same name in EC_DynamicComponent, and this version
        doesn't work right for py&js 'cause doesn't return a QVariant .. so not a slot now as a temporary measure. */
    IAttribute* GetAttribute(const QString &name) const;

public slots:
    /// Returns a pointer to the Framework instance.
    Framework *GetFramework() const { return framework; }

    /// Enables or disables network synchronization of changes that occur in the attributes of this component.
    /** By default, this flag is set for all created components.
        When network synchronization is disabled, changes to the attributes of this component affect
        only locally and will not be pushed to network. */
    void SetNetworkSyncEnabled(bool enabled);

    /// Returns true if network synchronization of the attributes of this component is enabled.
    bool NetworkSyncEnabled() const { return networkSync; }

    /// Sets the default mode for attribute change operations
    void SetUpdateMode(AttributeChange::Type defaultmode);
    
    /// Gets the default mode for attribute change operations
    AttributeChange::Type UpdateMode() const { return updateMode; }

    /// Returns true if component has dynamic attribute structure
    virtual bool HasDynamicStructure() const { return false; }

    /// Returns the number of Attributes in this component.
    int NumAttributes() const { return attributes.size(); }

    /// Informs this component that the value of a member Attribute of this component has changed.
    /** You may call this function manually to force Attribute change signal to
        occur, but it is not necessary if you use the Attribute::Set function, since
        it notifies this function automatically.
        @param attribute The attribute that was changed. The attribute passed here must be an Attribute member of this component.
        @param change Informs to the component the type of change that occurred.

        This function calls EmitAttributeChanged and triggers the 
        OnAttributeChanged signal of this component.

        This function is called by IAttribute::Changed whenever the value in that
        attribute is changed. */
    void EmitAttributeChanged(IAttribute* attribute, AttributeChange::Type change);

    /// This is an overloaded function.
    /** @param attributeName Name of the attribute that changed. @note this is a no-op if the named attribute is not found.
        @param change Informs to the component the type of change that occurred. */
    void EmitAttributeChanged(const QString& attributeName, AttributeChange::Type change);

    /// Informs that every attribute in this Component has changed with the change
    /** you specify. If change is Replicate, or it is Default and the UpdateMode is Replicate,
        every attribute will be synced to the network. */
    void ComponentChanged(AttributeChange::Type change);

    /// Returns the Entity this Component is part of.
    /** @note Calling this function will return null if it is called in the ctor of this Component. This is
              because the parent entity has not yet been set with a call to SetParentEntity at that point. */
    Entity* ParentEntity() const;

    /// Returns the scene this Component is part of.
    /** May return null if component is not in an entity or entity is not in a scene. */
    Scene* ParentScene() const;

    /// Sets whether component is temporary. Temporary components won't be saved when the scene is saved.
    void SetTemporary(bool enable);

    /// Returns whether component is temporary. Temporary components won't be saved when the scene is saved.
    /** @note if parent entity is temporary, this returns always true regardless of the component's temporary flag. */
    bool IsTemporary() const;

    /// Returns whether the component is in a view-enabled scene, or not.
    /** If the information is not available (component is not yet in a scene, will guess "true. */
    bool ViewEnabled() const;

    /// Returns an attribute of this component as a QVariant
    /** @param name of attribute
        @return values of the attribute */
    QVariant GetAttributeQVariant(const QString &name) const;

    /// Returns list of attribute names of the component
    QStringList GetAttributeNames() const;

signals:
    /// This signal is emitted when an Attribute of this Component has changed. 
    void AttributeChanged(IAttribute* attribute, AttributeChange::Type change);

    ///\todo In the future, provide a method of listening to a change of specific Attribute, instead of having to
    /// always connect to the above function and if(...)'ing if it was the change we were interested in.

    /// This signal is emitted when the name of this Component has changed.
    /** Use this signal to keep track of a component with specified custom name.*/
    void ComponentNameChanged(const QString &newName, const QString &oldName);

    /// This signal is emitted when this Component is attached to its owning Entity.
    void ParentEntitySet();

    /// This signal is emitted when this Component is detached from its parent, i.e. the new parent is set to null.
    void ParentEntityDetached();

protected:
    /// Helper function for starting component serialization.
    /** This function creates an XML element <component> with the name of this component, adds it to the document, and returns it. */
    QDomElement BeginSerialization(QDomDocument& doc, QDomElement& baseElement) const;

    /// Helper function for adding an attribute to the component xml serialization.
    void WriteAttribute(QDomDocument& doc, QDomElement& compElement, const QString& name, const QString& value) const;

    /// Helper function for adding an attribute and it's type to the component xml serialization.
    void WriteAttribute(QDomDocument& doc, QDomElement& compElement, const QString& name, const QString& value, const QString &type) const;

    /// Helper function for starting deserialization. 
    /** Checks that XML element contains the right kind of EC, and if it is right, sets the component name.
        Otherwise returns false and does nothing. */
    bool BeginDeserialization(QDomElement& compElement);

    /// Helper function for getting an attribute from serialized component.
    QString ReadAttribute(QDomElement& compElement, const QString &name) const;

    /// Helper function for getting a attribute type from serialized component.
    QString ReadAttributeType(QDomElement& compElement, const QString &name) const;

    /// Points to the Entity this Component is part of, or null if this Component is not attached to any Entity.
    Entity* parentEntity;

    /// The name of this component, by default an empty string.
    QString name;

    /// Attribute list for introspection/reflection.
    AttributeVector attributes;

    /// Network sync enable flag
    bool networkSync;

    /// Default update mode for attribute changes
    AttributeChange::Type updateMode;

    /// Framework pointer. Needed to be able to perform important uninitialization etc. even when not in an entity.
    Framework* framework;

    /// Temporary-flag
    bool temporary;

private:
    friend class ::IAttribute;

    /// Called by IAttribute on initialization of each attribute
    void AddAttribute(IAttribute* attr) { attributes.push_back(attr); }
};
