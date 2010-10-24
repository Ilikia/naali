// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "ECBrowser.h"
#include "IComponent.h"
#include "ECComponentEditor.h"
#include "SceneManager.h"
#include "ECEditorModule.h"
#include "Framework.h"
#include "EC_DynamicComponent.h"
#include "ECEditorModule.h"
#include "RexTypes.h"
#include "RexUUID.h"

#include <QtBrowserItem>
#include <QLayout>
#include <QShortcut>
#include <QMenu>
#include <QDomDocument>
#include <QMimeData>

#include "LoggingFunctions.h"
DEFINE_POCO_LOGGING_FUNCTIONS("ECBrowser")

#include "MemoryLeakCheck.h"

namespace ECEditor
{
    ECBrowser::ECBrowser(Foundation::Framework *framework, QWidget *parent): 
        QtTreePropertyBrowser(parent),
        menu_(0),
        treeWidget_(0),
        framework_(framework)
    {
        InitBrowser();
    }

    ECBrowser::~ECBrowser()
    {
        clear();
    }

    void ECBrowser::AddEntity(Scene::EntityPtr entity)
    {
        PROFILE(ECBrowser_AddNewEntity);

        assert(entity);
        if(!entity)
            return;

        //If entity is already added to browser no point to continue.
        if(HasEntity(entity))
            return;
        entities_.push_back(Scene::EntityPtr(entity));

        //! @todo merge entity.h and entity.cpp from tundra to develope and begin to use their ComponentAdded and ComponentRemoved signals.
        connect(entity.get(), SIGNAL(ComponentAdded(IComponent*, AttributeChange::Type)), this, SLOT(OnComponentAdded(IComponent*, AttributeChange::Type)));
        connect(entity.get(), SIGNAL(ComponentRemoved(IComponent*, AttributeChange::Type)), this, SLOT(OnComponentRemoved(IComponent*, AttributeChange::Type)));
    }

    void ECBrowser::RemoveEntity(Scene::EntityPtr entity)
    {
        if (!entity)
            return;

        for(EntityWeakPtrList::iterator iter = entities_.begin(); iter != entities_.end(); iter++)
        {
            Scene::EntityPtr ent_ptr = (*iter).lock();
            if (ent_ptr && ent_ptr.get() == entity.get())
            {
                Scene::Entity::ComponentVector components = ent_ptr->GetComponentVector();
                for(uint i = 0; i < components.size(); i++)
                    RemoveComponentFromGroup(components[i]);
                entities_.erase(iter);
                break;
            }
        }
    }

    QList<Scene::EntityPtr> ECBrowser::GetEntities() const
    {
        QList<Scene::EntityPtr> ret;
        for(uint i = 0; i < entities_.size(); i++)
            if(!entities_[i].expired())
                ret.push_back(entities_[i].lock());
        return ret;
    }

    void ECBrowser::clear()
    {
        while(!componentGroups_.empty())
        {
            SAFE_DELETE(componentGroups_.back())
            componentGroups_.pop_back();
        }
        entities_.clear();
        QtTreePropertyBrowser::clear();
    }

    void ECBrowser::UpdateBrowser()
    {
        PROFILE(ECBrowser_UpdateBrowser);

        // Sorting tend to be heavy operation so we disable it until we have made all changes to a ui.
        if(treeWidget_)
            treeWidget_->setSortingEnabled(false);

        for(EntityWeakPtrList::iterator iter = entities_.begin(); iter != entities_.end(); iter++)
        {
            if((*iter).expired())
                continue;

            const Scene::Entity::ComponentVector components = (*iter).lock()->GetComponentVector();
            for(uint i = 0; i < components.size(); i++)
                AddNewComponentToGroup(components[i]);
        }
        if(treeWidget_)
            treeWidget_->setSortingEnabled(true);
    }

    void ECBrowser::dragEnterEvent(QDragEnterEvent *event)
    {
        QTreeWidgetItem *item = treeWidget_->itemAt(event->pos().x(), event->pos().y() - 20);
        if (event->mimeData()->hasFormat("application/vnd.inventory.item") && item && !item->childCount())
            event->acceptProposedAction();
    }

    void ECBrowser::dropEvent(QDropEvent *event)
    {
        QTreeWidgetItem *item = treeWidget_->itemAt(event->pos().x(), event->pos().y() - 20);
        if(item)
            dropMimeData(item, 0, event->mimeData(), event->dropAction());
    }

    void ECBrowser::dragMoveEvent(QDragMoveEvent *event)
    {
        if (event->mimeData()->hasFormat("application/vnd.inventory.item"))
        {
            QTreeWidgetItem *item = treeWidget_->itemAt(event->pos().x(), event->pos().y() - 20);
            if(item && !item->childCount())
            {
                event->accept();
                return;
            }
        }
        event->ignore();
    }

    bool ECBrowser::dropMimeData(QTreeWidgetItem *item, int index, const QMimeData *data, Qt::DropAction action)
    {
        if (action == Qt::IgnoreAction)
            return true;

        if (!data->hasFormat("application/vnd.inventory.item"))
            return false;

        QByteArray encodedData = data->data("application/vnd.inventory.item");
        QDataStream stream(&encodedData, QIODevice::ReadOnly);

        QString asset_id;
        while(!stream.atEnd())
        {
            QString mimedata, asset_type, item_id, name;
            stream >> mimedata;

            QStringList list = mimedata.split(";", QString::SkipEmptyParts);
            if (list.size() < 4)
                continue;

            item_id = list.at(1);
            if (!RexUUID::IsValid(item_id.toStdString()))
                continue;

            name = list.at(2);
            asset_id = list.at(3);
        }

        if (!RexUUID::IsValid(asset_id.toStdString()))
            return false;

        QTreeWidgetItem *rootItem = item;
        for(;rootItem->parent(); rootItem = rootItem->parent())
        {
        }

        ComponentGroupList::iterator iter = FindSuitableGroup(*rootItem);
        if(iter != componentGroups_.end())
        {
            for(uint i = 0; i < (*iter)->components_.size(); i++)
            {
                ComponentWeakPtr compWeak = (*iter)->components_[i];
                if(compWeak.expired())
                    continue;

                QStringList names;
                names << item->text(0);
                if(item->parent())
                    names << item->parent()->text(0);

                // Try to find the right attribute.
                IAttribute *attr = 0;
                for(uint i = 0; i < names.size(); i++)
                {
                    attr = compWeak.lock()->GetAttribute(names[i]);
                    if(attr)
                        break;
                }
                if(!attr)
                    continue;
                if(attr->TypenameToString() == "string")
                {
                    Attribute<QString> *attribute = dynamic_cast<Attribute<QString> *>(attr);
                    if(attribute)
                    {
                        attribute->Set(QString::fromStdString(asset_id.toStdString()), AttributeChange::Default);
                        //compWeak.lock()->ComponentChanged(AttributeChange::Default);
                    }
                }
                else if(attr->TypenameToString() == "qvariant")
                {
                    Attribute<QVariant> *attribute = dynamic_cast<Attribute<QVariant> *>(attr);
                    if(attribute)
                    {
                        if(attribute->Get().type() == QVariant::String)
                        {
                            attribute->Set(asset_id, AttributeChange::Default);
                            //compWeak.lock()->ComponentChanged(AttributeChange::Default);
                        }
                    }
                }
                else if(attr->TypenameToString() == "qvariantarray")
                {
                    Attribute<std::vector<QVariant> > *attribute = dynamic_cast<Attribute<std::vector<QVariant> > *>(attr);
                    if(attribute)
                    {
                        // We asume that item's name is form of "[0]","[1]" etc. We need to cut down those first and last characters
                        // to able to get real index number of that item that is cause sorting can make the item order a bit odd.
                        QString indexText = "";
                        QString itemText = item->text(0);
                        for(uint i = 1; i < itemText.size() - 1; i++)
                            indexText += itemText[i];
                        bool ok;
                        int index = indexText.toInt(&ok);
                        if(!ok)
                            return false;

                        std::vector<QVariant> variants = attribute->Get();
                        if(variants.size() > index)
                            variants[index] = asset_id;
                        else if(variants.size() == index)
                            variants.push_back(asset_id);
                        else
                            return false;

                        attribute->Set(variants, AttributeChange::Default);
                        //compWeak.lock()->ComponentChanged(AttributeChange::Default); 
                    }
                }
                else if(attr->TypenameToString() == "qvariantlist")
                {
                    Attribute<QVariantList > *attribute = dynamic_cast<Attribute<QVariantList > *>(attr);
                    if(attribute)
                    {
                        // We asume that item's name is form of "[0]","[1]" etc. We need to cut down those first and last characters
                        // to able to get real index number of that item that is cause sorting can make the item order a bit odd.
                        QString indexText = "";
                        QString itemText = item->text(0);
                        for(uint i = 1; i < itemText.size() - 1; i++)
                            indexText += itemText[i];
                        bool ok;
                        int index = indexText.toInt(&ok);
                        if(!ok)
                            return false;

                        QVariantList variants = attribute->Get();
                        if(variants.size() > index)
                            variants[index] = asset_id;
                        else if(variants.size() == index)
                            variants.push_back(asset_id);
                        else
                            return false;

                        attribute->Set(variants, AttributeChange::Default);
                        //compWeak.lock()->ComponentChanged(AttributeChange::Default); 
                    }
                }
            }
        }
        else
            return false;

        return true;
    }

    void ECBrowser::InitBrowser()
    {
        setMouseTracking(true);
        setAcceptDrops(true);
        setContextMenuPolicy(Qt::CustomContextMenu);
        QObject::connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(ShowComponentContextMenu(const QPoint &)));
        treeWidget_ = findChild<QTreeWidget *>();
        if(treeWidget_)
        {
            treeWidget_->setSortingEnabled(true);
            //treeWidget_->setSelectionMode(QAbstractItemView::ExtendedSelection);
            treeWidget_->setFocusPolicy(Qt::StrongFocus);
            treeWidget_->setAcceptDrops(true);
            treeWidget_->setDragDropMode(QAbstractItemView::DropOnly);
            connect(treeWidget_, SIGNAL(itemSelectionChanged()), this, SLOT(SelectionChanged()));
            QShortcut* delete_shortcut = new QShortcut(Qt::Key_Delete, treeWidget_);
            QShortcut* copy_shortcut = new QShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_C), treeWidget_);
            QShortcut* paste_shortcut = new QShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_V), treeWidget_);
            connect(delete_shortcut, SIGNAL(activated()), this, SLOT(DeleteComponent()));
            connect(copy_shortcut, SIGNAL(activated()), this, SLOT(CopyComponent()));
            connect(paste_shortcut, SIGNAL(activated()), this, SLOT(PasteComponent()));
        }
    }

    void ECBrowser::ShowComponentContextMenu(const QPoint &pos)
    {
        //! @todo position should be converted to treeWidget's space so that editor will select the right
        //! component when user right clicks on the browser. right now position is bit off and wrong item 
        //! is selected. Included fast fix that wont make it work perfectly.
        if(!treeWidget_)
            return;

        /*QPoint globalPos = mapToGlobal(pos);
        QPoint point = treeWidget_->mapFromGlobal(globalPos);*/
        QTreeWidgetItem *treeWidgetItem = treeWidget_->itemAt(pos.x(), pos.y() - 20);
        if(treeWidgetItem)
            treeWidget_->setCurrentItem(treeWidgetItem);
        else
            treeWidget_->setCurrentItem(0);

        SAFE_DELETE(menu_);
        menu_ = new QMenu(this);
        menu_->setAttribute(Qt::WA_DeleteOnClose);
        if(treeWidgetItem)
        {
            QAction *copyComponent = new QAction(tr("Copy"), menu_);
            QAction *pasteComponent = new QAction(tr("Paste"), menu_);
            QAction *editXml= new QAction(tr("Edit XML..."), menu_);
            // Delete action functionality can vary based on what QTreeWidgetItem is selected on the browser.
            // If root item is selected we assume that we want to remove component and if attributes root
            // node is selected we want to remove that attribute instead.
            QAction *deleteAction= new QAction(tr("Delete"), menu_);

            //Add shortcuts for actions
            copyComponent->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_C));
            pasteComponent->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_V));
            deleteAction->setShortcut(QKeySequence::Delete);

            QTreeWidgetItem *parentItem = treeWidgetItem;
            while(parentItem->parent())
                parentItem = parentItem->parent();
            ComponentGroupList::iterator iter = FindSuitableGroup(*parentItem);

            if(parentItem == treeWidgetItem)
            {
                QObject::connect(copyComponent, SIGNAL(triggered()), this, SLOT(CopyComponent()));
                QObject::connect(pasteComponent, SIGNAL(triggered()), this, SLOT(PasteComponent()));
                QObject::connect(editXml, SIGNAL(triggered()), this, SLOT(OpenComponentXmlEditor()));
                menu_->addAction(copyComponent);
                menu_->addAction(pasteComponent);
                menu_->addAction(editXml);
            }
            QObject::connect(deleteAction, SIGNAL(triggered()), this, SLOT(DeleteComponent()));
            menu_->addAction(deleteAction);

            if(iter != componentGroups_.end())
            {
                if((*iter)->isDynamic_)
                {
                    // Check if the selected tree widget name is same as some of the dynamic component's attribute name.
                    if((*iter)->ContainsAttribute(treeWidgetItem->text(0)))
                    {
                        QObject::disconnect(deleteAction, SIGNAL(triggered()), this, SLOT(DeleteComponent()));
                        QObject::connect(deleteAction, SIGNAL(triggered()), this, SLOT(RemoveAttribute()));
                    }
                    QAction *addAttribute = new QAction(tr("Add new attribute"), menu_);
                    QObject::connect(addAttribute, SIGNAL(triggered()), this, SLOT(CreateAttribute()));
                    menu_->addAction(addAttribute);
                }
            }
        }
        else
        {
            QAction *addComponent = new QAction(tr("Add new component ..."), menu_);
            QAction *pasteComponent = new QAction(tr("Paste"), menu_);
            menu_->addAction(addComponent);
            menu_->addAction(pasteComponent);
            QObject::connect(addComponent, SIGNAL(triggered()), this, SIGNAL(CreateNewComponent()));
            QObject::connect(pasteComponent, SIGNAL(triggered()), this, SLOT(PasteComponent()));
        }
        menu_->popup(mapToGlobal(pos));
    }
    
    void ECBrowser::SelectionChanged()
    {
        PROFILE(ECBrowser_SelectionChanged);

        QTreeWidgetItem *item = treeWidget_->currentItem();
        if(!item)
            return;
        // Go back to the root node.
        while(item->parent())
            item = item->parent();
        
        ComponentGroupList::iterator iter = FindSuitableGroup(*item);
        if(iter != componentGroups_.end())
        {
            for(uint i = 0; i < (*iter)->components_.size(); i++)
            {
                PROFILE(ECBrowser_SelectionChanged_inner);
                if((*iter)->components_[i].expired())
                    continue;
                emit ComponentSelected((*iter)->components_[i].lock().get());
            }
        }
    }
    
    void ECBrowser::OnComponentAdded(IComponent* comp, AttributeChange::Type type) 
    {
        Scene::EntityPtr entity_ptr = framework_->GetDefaultWorldScene()->GetEntity(comp->GetParentEntity()->GetId());
        if(!HasEntity(entity_ptr))
            return;
        ComponentPtr comp_ptr = comp->GetSharedPtr();
        if(!comp_ptr)
        {
            LogError("Fail to add new component to ECBroser. Make sure that component's parent entity is setted.");
            return;
        }

        ComponentGroupList::iterator iterComp = componentGroups_.begin();
        for(; iterComp != componentGroups_.end(); iterComp++)
            if((*iterComp)->ContainsComponent(comp_ptr))
            {
                LogWarning("Fail to add new component to a component group, because component was already added.");
                return;
            }
        AddNewComponentToGroup(comp_ptr);
    }

    void ECBrowser::OnComponentRemoved(IComponent* comp, AttributeChange::Type type)
    {
        Scene::EntityPtr entity_ptr = framework_->GetDefaultWorldScene()->GetEntity(comp->GetParentEntity()->GetId());
        if(!HasEntity(entity_ptr))
            return;
        ComponentPtr comp_ptr = comp->GetSharedPtr();
        if(!comp_ptr)
        {
            LogError("Fail to remove component from ECBroser. Make sure that component's parent entity is setted.");
            return;
        }

        ComponentGroupList::iterator iterComp = componentGroups_.begin();
        for(; iterComp != componentGroups_.end(); iterComp++)
        {
            if(!(*iterComp)->ContainsComponent(comp_ptr))
                continue;
            RemoveComponentFromGroup(comp_ptr);
            return;
        }
    }

    void ECBrowser::OpenComponentXmlEditor()
    {
        QTreeWidgetItem *item = treeWidget_->currentItem();
        if(!item)
            return;

        ComponentGroupList::iterator iter = componentGroups_.begin();
        for(; iter != componentGroups_.end(); iter++)
        {
            if(item == (*iter)->browserListItem_)
            {
                if(!(*iter)->components_.size())
                    return;
                ComponentWeakPtr pointer = (*iter)->components_[0];
                if(!pointer.expired())
                {
                    emit ShowXmlEditorForComponent(pointer.lock()->TypeName().toStdString());
                    break;
                }
            }
        }
    }

    void ECBrowser::CopyComponent()
    {
        QDomDocument temp_doc;
        QDomElement entity_elem;
        QClipboard *clipboard = QApplication::clipboard();

        QTreeWidgetItem *item = treeWidget_->currentItem();
        if(!item)
            return;

        ComponentGroupList::iterator iter = componentGroups_.begin();
        for(; iter != componentGroups_.end(); iter++)
        {
            if(item == (*iter)->browserListItem_)
            {
                if(!(*iter)->components_.size())
                    return;
                // Just take a first component from the componentgroup and copy it's attribute values to clipboard. 
                // Note! wont take account that other components might have different values in their attributes
                ComponentWeakPtr pointer = (*iter)->components_[0];
                if(!pointer.expired())
                {
                    pointer.lock()->SerializeTo(temp_doc, entity_elem);
                    QString xmlText = temp_doc.toString();
                    clipboard->setText(xmlText);
                    break;
                }
            }
        }
    }

    void ECBrowser::PasteComponent()
    {
        QDomDocument temp_doc;
        QClipboard *clipboard = QApplication::clipboard();
        if (temp_doc.setContent(clipboard->text()))
        {
            // Only single component can be pasted.
            //! @todo add suport to multi component copy/paste feature.
            QDomElement comp_elem = temp_doc.firstChildElement("component");
            
            for(EntityWeakPtrList::iterator iter = entities_.begin();
                iter != entities_.end();
                iter++)
            {
                if((*iter).expired())
                    continue;
                Scene::EntityPtr entity_ptr = (*iter).lock();

                ComponentPtr component;
                QString type = comp_elem.attribute("type");
                QString name = comp_elem.attribute("name");
                if(!entity_ptr->HasComponent(type, name))
                {
                    component = framework_->GetComponentManager()->CreateComponent(type, name);
                    entity_ptr->AddComponent(component, AttributeChange::Default);
                }
                else
                    component = entity_ptr->GetComponent(type, name);
                component->DeserializeFrom(comp_elem, AttributeChange::Default);
                //component->ComponentChanged(AttributeChange::Default);
            }
        }
    }

    void ECBrowser::DeleteComponent()
    {
        QTreeWidgetItem *item = treeWidget_->currentItem();
        if(!item)
            return;

        ComponentGroupList::iterator iter = componentGroups_.begin();
        for(; iter != componentGroups_.end(); iter++)
        {
            ComponentGroup *componentGroup = (*iter);
            if(item != componentGroup->browserListItem_)
                continue;

            while(!componentGroup->components_.empty())
            {
                ComponentWeakPtr pointer = componentGroup->components_.back();
                if(!pointer.expired())
                {
                    ComponentPtr comp = pointer.lock();
                    Scene::Entity *entity = comp->GetParentEntity();
                    entity->RemoveComponent(comp, AttributeChange::Default);
                }
                else
                {
                    // If component has been expired no point to keep in the component group
                    componentGroup->components_.pop_back();
                }
            }
            break;
        }
    }

    void ECBrowser::DynamicComponentChanged()
    {
        EC_DynamicComponent *component = dynamic_cast<EC_DynamicComponent*>(sender());
        if(!component)
            return;
        ComponentPtr comp_ptr = component->GetSharedPtr();
        if(!comp_ptr)
        {
            LogError("Couldn't update dynamic component " + component->Name().toStdString() + ", cause parent entity was null.");
            return;
        }

        Scene::Entity *entity = component->GetParentEntity();
        ComponentPtr compPtr = entity->GetComponent(component);
        RemoveComponentFromGroup(comp_ptr);
        AddNewComponentToGroup(comp_ptr);
    }

    void ECBrowser::ComponentNameChanged(const QString &newName)
    {
        IComponent *component = dynamic_cast<IComponent*>(sender());
        if(component)
        {
            ComponentPtr comp_ptr = component->GetSharedPtr();
            if(!comp_ptr)
            {
                LogError("Couldn't update component name, cause parent entity was null.");
                return;
            }
            RemoveComponentFromGroup(comp_ptr);
            AddNewComponentToGroup(comp_ptr);
        }
    }

    void ECBrowser::CreateAttribute()
    {
        QTreeWidgetItem *item = treeWidget_->currentItem();
        if(!item)
            return;

        QTreeWidgetItem *rootItem = item;
        while(rootItem->parent())
            rootItem = rootItem->parent();

        ComponentGroupList::iterator iter = FindSuitableGroup(*rootItem);
        if(iter == componentGroups_.end())
            return;
        if(!(*iter)->IsDynamic())
            return;

        bool ok = false;
        QStringList attributeList;
        StringVector attributeTypes = framework_->GetComponentManager()->GetAttributeTypes();
        for(uint i = 0; i < attributeTypes.size(); i++)
            attributeList.push_back(QString::fromStdString(attributeTypes[i]));

        //! @todo replace this code with a one that will use Dialog::open method and listens a signal
        //! that will tell us when dialog is closed. Should be more safe way to get this done.
        QString typeName = QInputDialog::getItem(this, tr("Give attribute type"), tr("Typename:"), attributeList, 0, false, &ok);
        if (!ok)
            return;
        QString name = QInputDialog::getText(this, tr("Give attribute name"), tr("Name:"), QLineEdit::Normal, QString(), &ok);
        if (!ok)
            return;

        std::vector<ComponentWeakPtr> components = (*iter)->components_;
        for(uint i = 0; i < components.size(); i++)
        {
            if(components[i].expired())
                continue;
            EC_DynamicComponent *component = dynamic_cast<EC_DynamicComponent*>(components[i].lock().get());
            if(component)
            {
                if(component->CreateAttribute(typeName, name))
                    component->ComponentChanged(AttributeChange::Default);
            }
        }
    }

    void ECBrowser::RemoveAttribute()
    {
        QTreeWidgetItem *item = treeWidget_->currentItem();
        if(!item)
            return;

        QTreeWidgetItem *rootItem = item;
        while(rootItem->parent())
            rootItem = rootItem->parent();

        ComponentGroupList::iterator iter = FindSuitableGroup(*rootItem);
        if(iter == componentGroups_.end())
            return;
        if(!(*iter)->IsDynamic())
            return;

        std::vector<ComponentWeakPtr> components = (*iter)->components_;
        for(uint i = 0; i < components.size(); i++)
        {
            if(components[i].expired())
                continue;
            EC_DynamicComponent *comp = dynamic_cast<EC_DynamicComponent*>(components[i].lock().get());
            if(comp)
            {
                comp->RemoveAttribute(item->text(0));
                comp->ComponentChanged(AttributeChange::Default);
            }
        }
        //RemoveComponentGroup(*iter);
    }

    ComponentGroupList::iterator ECBrowser::FindSuitableGroup(ComponentPtr comp)
    {
        PROFILE(ECBrowser_FindSuitableGroup);

        ComponentGroupList::iterator iter = componentGroups_.begin();
        for(; iter != componentGroups_.end(); iter++)
            if((*iter)->IsSameComponent(comp))
                return iter;
        return iter;
    }

    ComponentGroupList::iterator ECBrowser::FindSuitableGroup(const QTreeWidgetItem &item)
    {
        ComponentGroupList::iterator iter = componentGroups_.begin();
        for(; iter != componentGroups_.end(); iter++)
        {
            if ((*iter)->browserListItem_ == &item)
                return iter;
        }
        return iter;
    }

    void ECBrowser::AddNewComponentToGroup(ComponentPtr comp)
    {
        assert(comp);
        if (!comp.get() && !treeWidget_)
            return;

        ComponentGroupList::iterator iter = FindSuitableGroup(comp);
        if (iter != componentGroups_.end())
        {
            ComponentGroup *comp_group = *iter;
            // No point to add same component multiple times.
            if (comp_group->ContainsComponent(comp))
                return;

            comp_group->editor_->AddNewComponent(comp, false);
            comp_group->components_.push_back(ComponentWeakPtr(comp));
            if (comp_group->IsDynamic())
            {
                EC_DynamicComponent *dc = dynamic_cast<EC_DynamicComponent*>(comp.get());
                connect(dc, SIGNAL(AttributeAdded(IAttribute *)), SLOT(DynamicComponentChanged()));
                connect(dc, SIGNAL(AttributeRemoved(const QString &)), SLOT(DynamicComponentChanged()));
                connect(dc, SIGNAL(OnComponentNameChanged(const QString&, const QString&)),
                        SLOT(ComponentNameChanged(const QString&)));
            }
            return;
        }

        QSet<QTreeWidgetItem*> oldList;
        QSet<QTreeWidgetItem*> newList;

        // Find a new QTreeWidgetItem from the browser and save the information to ComponentGroup object.
        for(uint i = 0; i < treeWidget_->topLevelItemCount(); i++)
            oldList.insert(treeWidget_->topLevelItem(i));
        ECComponentEditor *componentEditor = new ECComponentEditor(comp, this);
        for(uint i = 0; i < treeWidget_->topLevelItemCount(); i++)
            newList.insert(treeWidget_->topLevelItem(i));
        QSet<QTreeWidgetItem*> changeList = newList - oldList;
        QTreeWidgetItem *newItem = 0;
        if(changeList.size() == 1)
            newItem = (*changeList.begin());
        else
        {
            ECEditorModule::LogError("Failed to add a new component to ECEditor, for some reason the QTreeWidgetItem was not created."
                                     " Make sure that ECComponentEditor was intialized properly.");
            return;
        }
        //treeWidget_->sortItems(treeWidget_->indexOfTopLevelItem(newItem), Qt::AscendingOrder);

        bool dynamic = comp->TypeName() == "EC_DynamicComponent";
        if(dynamic)
        {
            EC_DynamicComponent *dc = dynamic_cast<EC_DynamicComponent*>(comp.get());
            connect(dc, SIGNAL(AttributeAdded(IAttribute *)), SLOT(DynamicComponentChanged()));
            connect(dc, SIGNAL(AttributeRemoved(const QString &)), SLOT(DynamicComponentChanged()));
            connect(dc, SIGNAL(OnComponentNameChanged(const QString &, const QString &)), SLOT(ComponentNameChanged(const QString&)));
        }
        ComponentGroup *compGroup = new ComponentGroup(comp, componentEditor, newItem, dynamic);
        if(compGroup)
            componentGroups_.push_back(compGroup);

        if(treeWidget_)
        {
            treeWidget_->expandItem(newItem);
            for(uint i = 0; i < newItem->childCount(); i++)
                treeWidget_->collapseItem(newItem->child(i));
        }
    }

    void ECBrowser::RemoveComponentFromGroup(ComponentPtr comp)
    {
        ComponentGroupList::iterator iter = componentGroups_.begin();
        for(; iter != componentGroups_.end(); iter++)
        {
            ComponentGroup *comp_group = *iter;
            if(!comp_group->ContainsComponent(comp))
                continue;
            if(comp_group->IsDynamic())
            {
                EC_DynamicComponent *dc = dynamic_cast<EC_DynamicComponent *>(comp.get());
                disconnect(dc, SIGNAL(AttributeAdded(IAttribute *)), this, SLOT(DynamicComponentChanged()));
                disconnect(dc, SIGNAL(AttributeRemoved(const QString &)), this, SLOT(DynamicComponentChanged()));
                disconnect(dc, SIGNAL(OnComponentNameChanged(const QString&, const QString &)), this, SLOT(ComponentNameChanged(const QString&)));
            }
            comp_group->RemoveComponent(comp);
            //Ensure that coponent group is valid and if it's not, remove it from the browser list.
            if(!comp_group->IsValid())
            {
                SAFE_DELETE(comp_group);
                componentGroups_.erase(iter);
            }
            break;
        }
    }

    void ECBrowser::RemoveComponentGroup(ComponentGroup *componentGroup)
    {
        ComponentGroupList::iterator iter = componentGroups_.begin();
        for(; iter != componentGroups_.end(); iter++)
        {
            if (componentGroup != (*iter))
                continue;

            SAFE_DELETE(*iter)
            componentGroups_.erase(iter);
            break;
        }
    }

    bool ECBrowser::HasEntity(Scene::EntityPtr entity) const
    {
        //assert(entity);
        for(uint i = 0; i < entities_.size(); i++)
        {
            if(!entities_[i].expired() && entities_[i].lock().get() == entity.get())
                return true;
        }
        return false;
    }
}
