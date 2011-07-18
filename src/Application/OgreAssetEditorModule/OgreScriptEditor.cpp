/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   OgreScriptEditor.cpp
 *  @brief  Editing tool for OGRE material and particle scripts.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "OgreScriptEditor.h"
#include "OgreAssetEditorModule.h"
#include "OgreMaterialProperties.h"
#include "PropertyTableWidget.h"
#include "OgreScriptHighlighter.h"

#include "OgreConversionUtils.h"
#include "LoggingFunctions.h"
#include "Application.h"
#include "AssetAPI.h"
#include "IAsset.h"

#include <QUiLoader>
#include <QFile>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QTableWidget>
#include <QColor>
#include <QVBoxLayout>

#include "MemoryLeakCheck.h"

OgreScriptEditor::OgreScriptEditor(const AssetPtr &assetPtr, AssetAPI *assetAPI, QWidget *parent) :
    QWidget(parent),
    assetApi(assetAPI),
    asset(assetPtr),
    lineEditName(0),
    buttonSaveAs(0),
    textEdit(0),
    propertyTable(0),
    materialProperties(0)
{
    if (asset->Type() != "OgreMaterial" && asset->Type() != "OgreParticle")
        LogWarning("Created OgreScriptEditor for non-supported asset type " + asset->Type() + ".");

    // Create widget from ui file
    QUiLoader loader;
    QFile file(Application::InstallationDirectory() + "data/ui/ogrescripteditor.ui");
    if (!file.exists())
    {
        LogError("Cannot find OGRE Script Editor .ui file.");
        return;
    }

    QWidget *mainWidget = loader.load(&file, this);
    file.close();

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(mainWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    lineEditName = mainWidget->findChild<QLineEdit *>("lineEditName");
    buttonSaveAs = mainWidget->findChild<QPushButton *>("buttonSaveAs");
    QPushButton *buttonSave = mainWidget->findChild<QPushButton *>("buttonSave");
    QPushButton *buttonCancel = mainWidget->findChild<QPushButton *>("buttonCancel");

    ///\todo Save as -functionality disabled for now.
    lineEditName->setDisabled(true);
    buttonSaveAs->setDisabled(true);

    connect(buttonSave, SIGNAL(clicked()), SLOT(Save()));
    connect(buttonSaveAs, SIGNAL(clicked()), SLOT(SaveAs()));
    connect(buttonCancel, SIGNAL(clicked(bool)), SLOT(close()));
    connect(lineEditName, SIGNAL(textChanged(const QString &)), SLOT(ValidateScriptName(const QString &)));

    lineEditName->setText(asset->Name());
    buttonSaveAs->setEnabled(false);

    setWindowTitle(tr("OGRE Script Editor"));
}

OgreScriptEditor::~OgreScriptEditor()
{
    SAFE_DELETE(propertyTable);
    SAFE_DELETE(materialProperties);
}

void OgreScriptEditor::Open()
{
    ///\todo
/*
    bool edit_raw = false;

    if (asset->Type() == "OgreParticle")
        edit_raw = true;

    if (asset->Type() == "OgreMaterial")
    {
        materialProperties = new OgreMaterialProperties(asset->Name(), asset);
        if (materialProperties && materialProperties->HasProperties())
            CreatePropertyEditor();
        else
            edit_raw = true;
    }

    if (edit_raw)
    {
        QString script(asset->GetRawData());
        if (script.isEmpty() && script.isNull())
        {
            LogError("Invalid data for generating an OGRE script.");
            return;
        }

        // Replace tabs (ASCII code decimal 9) with 4 spaces because tabs might appear incorrectly.
        script = script.trimmed();
        script.replace(QChar(9), "    ");

        CreateTextEdit();
        textEdit->setText(script);
    }
*/
    if (asset->Type() == "OgreMaterial" || asset->Type() == "OgreParticle")
    {
        std::vector<u8> data;
        if (asset->SerializeTo(data))
        {
            data.push_back('\0');
            QString script((const char *)&data[0]);
            script = script.trimmed();
            script.replace(QChar(9), "    ");

            CreateTextEdit();
            textEdit->setText(script);
            OgreScriptHighlighter *hl= new OgreScriptHighlighter(asset->Type(), textEdit);
            hl->setDocument(textEdit->document());
        }
    }
}

void OgreScriptEditor::Save()
{
    if (asset->Type() == "OgreMaterial" || asset->Type() == "OgreParticle")
    {
        QByteArray bytes = textEdit->toPlainText().toAscii().data();
        const char *data = bytes.data();
        asset->LoadFromFileInMemory((u8 *)data, (size_t)bytes.size());
    }
}

void OgreScriptEditor::SaveAs()
{
///\todo Enable save as feature
/*
    // Get the script.
    QString script;
    if (asset->Type() == "OgreParticle")
    {
        script = textEdit->toPlainText();
        script = script.trimmed();
        if (script.isEmpty() || script.isNull())
        {
            LogError("Empty script cannot be saved.");
            return;
        }
    }

    if (asset->Type() == "OgreMaterial" && materialProperties)
        script = materialProperties->ToString();

    // Get the name.
    QString filename = lineEditName->text();
    if (filename.isEmpty() || filename.isNull())
    {
        LogError("Empty name for the script, cannot save.");
        return;
    }
*/
}

void OgreScriptEditor::ValidateScriptName(const QString &name)
{
    if (assetApi->GetAsset(name) || name.isEmpty() || name.isNull())
        buttonSaveAs->setEnabled(false);
    else
        buttonSaveAs->setEnabled(true);
}

void OgreScriptEditor::PropertyChanged(int row, int column)
{
    QTableWidgetItem *nameItem = propertyTable->item(row, column - 2);
    QTableWidgetItem *typeItem = propertyTable->item(row, column - 1);
    QTableWidgetItem *valueItem = propertyTable->item(row, column);
    if (!nameItem || !typeItem || !valueItem)
        return;

    QString newValueString(valueItem->text());
    newValueString = newValueString.trimmed();
    bool valid = true;

    QString type = typeItem->text();
    if (type == "TEX_1D" || type == "TEX_2D" || type == "TEX_3D" && type == "TEX_CUBEMAP")
    {
        // If the texture name is not valid UUID or URL it can't be used.
        if (1/*RexUUID::IsValid(newValueString)*/)
            valid = true;
        else if(newValueString.indexOf("http://") != -1)
            valid = true;
        else
            valid = false;
    }
    else
    {
        int i = 0, j = 0;
        while(j != -1 && valid)
        {
            j = newValueString.indexOf(' ', i);
            QString newValue = newValueString.mid(i, j == -1 ? j : j - i);
            if (!newValue.isEmpty())
                newValue.toFloat(&valid);
            i = j + 1;
        }
    }

    if (valid)
    {
        valueItem->setBackgroundColor(QColor(QColor(81, 255, 81)));
        QMap<QString, QVariant> typeValuePair;
        typeValuePair[typeItem->text()] = newValueString;
        materialProperties->setProperty(nameItem->text().toLatin1(), QVariant(typeValuePair));
        ValidateScriptName(lineEditName->text());
    }
    else
    {
        valueItem->setBackgroundColor(QColor(255, 73, 73));
        buttonSaveAs->setEnabled(false);
    }

    propertyTable->setCurrentItem(valueItem, QItemSelectionModel::Deselect);
}

void OgreScriptEditor::CreateTextEdit()
{
    // Raw text edit for particle scripts or material scripts without properties.
    textEdit = new QTextEdit(this);
    textEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    textEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    textEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    textEdit->setLineWrapMode(QTextEdit::NoWrap);

    QVBoxLayout *layout  = findChild<QWidget *>("OgreScriptEditor")->findChild<QVBoxLayout *>("verticalLayoutEditor");
    layout->addWidget(textEdit);
    textEdit->show();
}

void OgreScriptEditor::CreatePropertyEditor()
{
    PropertyMap propMap = materialProperties->GetPropertyMap();
    PropertyMapIter it(propMap);

    propertyTable = new PropertyTableWidget(propMap.size(), 3);
    QVBoxLayout *layout = findChild<QWidget *>("OgreScriptEditor")->findChild<QVBoxLayout *>("verticalLayoutEditor");
    layout->addWidget(propertyTable);

    int row = 0;
    while(it.hasNext())
    {
        it.next();
        QMap<QString, QVariant> typeValuePair = it.value().toMap();

        // Property name, set non-editable.
        QTableWidgetItem *nameItem = new QTableWidgetItem(it.key());
        nameItem->setFlags(Qt::ItemIsEnabled);

        // Property type, set non-editable.
        QTableWidgetItem *typeItem = new QTableWidgetItem(typeValuePair.begin().key());
        typeItem->setFlags(Qt::ItemIsEnabled);

        // Property value
        QTableWidgetItem *valueItem = new QTableWidgetItem;

        // Disable drop support for non-texture properties.
        if (nameItem->text().indexOf(" TU") == -1)
        {
            Qt::ItemFlags flags = valueItem->flags();
            flags &= ~Qt::ItemIsDropEnabled;
            valueItem->setFlags(flags);
        }

        valueItem->setData(Qt::DisplayRole, typeValuePair.begin().value());
        valueItem->setBackgroundColor(QColor(81, 255, 81));

        propertyTable->setItem(row, 0, nameItem);
        propertyTable->setItem(row, 1, typeItem);
        propertyTable->setItem(row, 2, valueItem);
        ++row;
    }

    propertyTable->show();

    connect(propertyTable, SIGNAL(cellChanged(int, int)), this, SLOT(PropertyChanged(int, int)));
}
