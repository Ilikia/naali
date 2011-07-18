/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   QtUtils.h
 *  @brief  Cross-platform utility functions using Qt.
 */

#pragma once

#include "CoreTypes.h"
#include <QStringList>

class QFileDialog;

namespace QtUtils
{
    /// Opens a non-modal file dialog
    /** @param filter The files to be shown.
        @param caption Dialog's caption
        @param dir Working directory.
        @param parent Parent widget. If you give parent widget, the file dialog will be part of the UI scene.
            Give 0 if you want the dialog to be external.
        @param initiator QObject that initiated the dialog and wants the results
        @param slot Slot on initiator object, to which dialog's finished() signal will be 
               connected. Result value will be 1 if OK was pressed. Use sender() and dynamic 
               cast to QFileDialog to get to know the chosen file(s)
        @param multipleFiles Do we want to be able to select multiple files.
        @return The created file dialog
    */
    QFileDialog* OpenFileDialogNonModal(
        const QString& filter,
        const QString& caption,
        const QString& dir,
        QWidget* parent,
        QObject* initiator,
        const char* slot,
        bool multipleFiles = false);

    /// Opens a non-modal file save dialog
    /** @param filter The files to be shown.
        @param caption Dialog's caption
        @param dir Working directory.
        @param parent Parent widget. If you give parent widget, the file dialog will be part of the UI scene.
            Give 0 if you want the dialog to be external.
        @param initiator QObject that initiated the dialog and wants the results
        @param slot Slot on initiator object, to which dialog's finished() signal will be 
               connected. Result value will be 1 if OK was pressed. Use sender() and dynamic 
               cast to QFileDialog to get to know the chosen file(s)
        @return The created file dialog
    */
    QFileDialog* SaveFileDialogNonModal(
        const QString& filter,
        const QString& caption,
        const QString& dir,
        QWidget* parent,
        QObject* initiator,
        const char* slot);

    /// Opens non-modal select directory dialog.
    /** @note Differs from other functions in that this won't send a finished signal if user closes the dialog without
              selecting a directory.
        @param caption Dialog's caption
        @param dir Working directory.
        @param parent Parent widget. If you give parent widget, the file dialog will be part of the UI scene.
            Give 0 if you want the dialog to be external.
        @param initiator QObject that initiated the dialog and wants the results
        @param slot Slot on initiator object, to which dialog's finished() signal will be 
               connected. Result value will be 1 if OK was pressed. Use sender() and dynamic 
               cast to QFileDialog to get to know the chosen file(s)
        @return The created file dialog
    */
    QFileDialog *DirectoryDialogNonModal(
        const QString &caption, 
        const QString &dir, 
        QWidget *parent, 
        QObject* initiator, 
        const char* slot);

    /// Opens the OS's open file dialog.
    /** @param filter The files to be shown.
        @param caption Dialog's caption.
        @param dir Working directory.
        @return The absolute path to the chosen file.
    */
    std::string GetOpenFileName(
        const std::string &filter,
        const std::string &caption,
        const std::string &dir);

    /// Opens the OS's save file dialog.
    /** @param filter The files to be shown.
        @param caption Dialog's caption.
        @param dir Working directory.
        @return The absolute path to the chosen file.
    */
    std::string GetSaveFileName(
        const std::string &filter,
        const std::string &caption,
        const std::string &dir);

    /// Same as GetOpenFileName but for multiple files.
    /** @return List of absolute paths to the chosen files.
    */
    StringList GetOpenFileNames(
        const std::string &filter,
        const std::string &caption,
        const std::string &dir);

    /// @return The absolute path of the application's current directory.
    std::string GetCurrentPath();
};

