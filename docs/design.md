# Architecture / Design

## Overview
Kube is supposed to be a small and concise codebase that is easy to modify and evolve.

It's following a reactive model, where in one direction we have actions generating modifications, and in the other direction models updating themselves on changes.

The overall architecture is split into three layers; Ui, Domain Logic and Infrastructure.

```
+----------------------------+
|        UI Components       |
+----------------------------+
|                            |
|        Domain Logic        |
|        Fabric/Models       |
|                            |
+--------------+------+------+
|              |      |      |
|    Sink      |Config| ...  |
|              |      |      |
+--------------+------+------+
```

The UI Layer consists of views (mostly written in QML), view-models (models that are view specific and potentially implement user interaction details), and the glue code to use various models and actions from the interface. Different UI layers may exist for different form factors.

The domain logic layer holds the application state. It povides models to access data and actions to act upon it. The domain logic is by definition Kube specific and not sharable with other applications, as it needs to be taylored exactly according to the requirements of Kube.

The infrastructure layer provides:

* Data access (Sink)
* Configuration (Config files, etc.)
* Various functionality provided by libraries (email sending, ldap, iTip handling, iCal implementation (kcalcore), vCard implementation, ...)
Various bits of the infrastructure layer may be exchanged on different platforms, to i.e. integrate into native infrastructure providers on a platform.

## UI / Application
The UI / Application layer contains all the view components, and their composition, that make up the application.
All the interactions between the different components are defined here.

## Components
The application consists of various application components. A component could be a maillist, an event-editor or the complete kube-mail application. Each component is instantiable on it's own, and has an API to interact with it. The API i.e. allows to set a folder for the maillist, or an event for the event-editor. Components can be nested (a component can instantiate another component)

A component primarily is a QML UI.

The QML UI is built on top of:

* One or more models that are instantiated to provide the data.
* The fabric to interconnect the components.

## Component interaction / Fabric
The application is made up of various nested components that often need to interact with each other.

This interaction is wired up using the Fabric.

The fabric is a pub/sub messagebus that is orthogonal to the visual hierarchy, that can be used to wire up varius parts of the UI
where a regular property binding would become problematic.

For more information see: https://cmollekopf.wordpress.com/2017/06/06/kubefabric/

If we look at the example of the org.kube.mail component:
1. The folderlist-component posts to the fabric that current folder has changed. The maillist reacts to that change and sets it's parentFolder property to display the mails of the currently selected folder.
2. The "add-note" message might either switch to the org.kube.note application as currently displayed component, or it might just display a quick-note widget directly inline.

This makes it possible for i.e. a maillist to display a note-widget directly inline, or letting the parent component handle the action to show a full note editor. If nothing handles the action, the root component (the shell)can switch to the note application component.

## Third party users of components
Since components are self contained and made available throuh the KPackage sytem, external applications can load fully functional Kube components.

For example, the KDE calendar plasmoid could load the Kube Event Viewer component when available, and thus provide Kube's full functionality of that component, including all actions etc, without having to reimplement the Domain Logic (as is the case if only data access is provided through Sink).

## Domain Logic

### Models
Self-updating models are used to implement the read-only part of the application.
By using QAbstractItemModels we can reuse the existing update mechanism, have something that works well with QML, and avoid implementing a boatload of boilerplate code for hand-coded domain objects.

Models should always be reactive and configured with a query, so they are asynchronous.

By implementing everything according to that model we can later on achieve lazy-loading of properties and zero-copy (or at least close to zero-copy) directly from storage without modifying anything besides data access.

Models are self contained and have an API to set i.e. a query for what to load. Models can load data from anywhere. Typically models are implemented in C++ to interface with the rest of the system, but some models may also be implemented directly in QML.

### Controller
Controllers are used to interact with the system. The controller is a QObject with a QObject-property for every property that should be editable, and a QValidator for every property, so editors can easily be built using property binding while providing property-level validation and feedback.

The domain object is exposed as an opaque QVariant. This way details from the infrastructure layer don't leak to the UI layer

Controllers may interact with infrastructure directly or via the fabric.

### Notifications
The system will provide notifications from various sources.

Notifications could be:

* New mails arrived
* An error occurred
* A synchronization is in progress
* ...

Notifications can be displayed in various places of the application and are transported over the Fabric.

## Infrastructure
The infrastructure layer interfaces with the rest of the system. It is the place where we can integrate with various native infrastructure parts.
The interface of the infrastructure layer, that is used by the domain logic, may not expose any implementation details of any infrastructure part, to ensure that all infrastructure parts are exchangable.

### Sink
Sink is used for primary data access and handles all synchronization.

Interactions with Sink involve:

* Adding/removing/configuring resources
* Triggering synchronization
* Querying of data
* Creating/Modifying/Removing entities

### Configuration
Configuration as traditionally stored in config files in ~/.config

### Notification
Notifications for the system.

### Files
Store/Load/Shared stuff (attachments, events, ....)
* Additional to the basic store/load stuff that may need further abstraction for mobile platforms beyond what qt provides.
* Android Intents/Libpurpose (share with various applications etc).

### Import/Export
Same as files? Import/Export calendar data

### RFC implementations
* iCal: KCalCore
* vCard: KContacts
* iTip: extract from kdepim repo
* SMTP: based on libcurl

### Cryptography
* PGP, PEP
* ObjectTreeParser

Keyselection, encryption, decryption, signing
Probably requires access to identities in some way.

see also [Cryptography](cryptography).

### MIME-Message parsing
* ObjectTreeParser
* KMime

## Testing

TBD

## Problems/Notes:
* Dynamic switching between various component UI's can be solved using KPackage

## Example usage in QML

```
KubeActions.Action {
    requestId: "org.kde.kube.mail.reply"
    onRequest {
        mail: context.mail
    }
}

KubeActions.ActionContext {
    id: actionContext
    mail: kubeMailListView.currentMail
}

KubeActions.ActionHandle {
    property int progress
    property bool complete
    property bool error
    property string errormessage
}

KubeActions.Action {
    id: markAsReadAction
    action: "org.kde.kube.action.mark-as-read"
    context: actionContext
    //execute() returns an ActionHandle
}

KubeComponents.FolderList {
    id: kubeFolderListView
}

KubeComponents.MailList {
    id: kubeMailListView
    parentFolder: kubeFolderListView.currentFolder
}

KubeComponents.MailView {
    id: kubeMailView
    mail: kubeMailListView.currentMail
}
```

## Email Domain Logic
* Folder list
    * Folder List Controller
        * Move mail to folder
        * Move/Copy/Delete folder
        * Synchronize folder
    * Folder List Model
        * Mixes Sink queries and subqueries (folder list with smart folders)
        * name
        * statistics

* Mail list
    * MailListController
        * Mark as read
        * Flag as important
        * Move to trash
    * MailListModel
        * subject
        * date
        * sender
        * folder
    * ThreadModel
        * thread leader (otherwise like maillist model)
        * number of mails in thread

* Mail Viewer
    * MailViewController
        * reply
        * forward
        * move to trash
    * MailModel
        * subject, date, sender, folder, content, attachments

## Configuration and Accounts
Kube is a groupware application, so one of its most important features is being able to work with various remote backends. We live in a world of multiple devies and applications, so it is interesting to share as much state and configuration accross all different devices and applications, which is why we try to store as much of that in the backend.

From the perspective of Kube we are working with different "Accounts". Each account represents a different backend, such as your personal IMAP or Kolab server, or a hosted offering such as GMail or Kolab Now. Each of those accounts may interact with various protocols such as imap, smtp, ldap, caldav etc.

To add support for a new backend thus means that a new account type has to be added to Kube.

An account consists of:

* One or more sink resources to access the remote data
* A configuration UI in QML that can be embedded in the accounts setup
* Potentially custom action handlers if the default action handlers are not sufficient.
* A configuration controller to modify and access the data
* A set of action pre-handler to supply the configuration to actions

### Configuration Controller
The configuraton controller is not only used in the configuration UI to provide the data, but it is also used by the rest of the system to access configuration of this account.

This allows the account to retrieve configruation data on a property-by-property basis i.e. from Sink or a local config file.

### Accounts-Plugin
The account is supplied as a kpackage based plugin. The plugin is loaded into kube directly from QML. The plugin registers it's configuration controller and potentially actions.

Note: We could have a plugin mechanism that discovers account-plugins should that become necessary at some point.

## Application Context
Various parts of the system are context sensitive. I.e. the currently selected account affects which transport is used to send an email, or which folders are currently visble.

In future iterations that context can be expanded i.e. with projects that affect prioritization of various data items.

The application context is globally available, although it may be altered locally.

## Focus handling
This section lines out how we deal with focus throughout the application.
The aim is to have consistent behaviour across controls and other elements.

The focus handling needs to take into account:

* Mouse navigation
* Keyboard navigation
* Touch input

The primary indicator for focus is the "activeFocus" property (or if available, "visualFocus"). The "focus" property is used for requesting focus and thus not what should be used for detecting focus.

There are the following different focus states:
* An element can be selected (if selectable). This includes i.e. a text editor that has focus, or a selectable element in a list view.
* An element can be hovered. This is only available with mouse pointers and indicates that the element can be clicked.
* An element can have keyboard focus. This is only available with keyboard navigation and indicates which element currently has keyboard focus.

With touch input only the selected state is relevant.

The following indicators are available to visualize selection:
* Highlight: A blue overlay over the item.
* Glow: A blue glow around the borders.
* Underlined text

It is important that a selected element can still show focus, otherwise the focus during keyboard navigation simply disappears if it moves to the selected element.

The following controls need to deal with focus:
* Buttons, IconButtons
* ListView, TreeView, GridView
* TextFields
* Copyable elements

We're indicating focus as follows:
* Active focus is indicated with a glow. This is used for both hovering and keyboard focus.
* A selected element is highlighted.
* Keyboard focus is additionally to the glow indicated with underlined text.

### FocusScope
In order to be able to deal with focus on a local scope (which is important for reusable components), a FocusScope is required to establish a border for focus handling. Internally you can set the focus as required within the focus scope, and externally you can just give focus to the FocusScope, ignoring what's going to happen internally. The FocusScope will automatically forward focus (when it receives it), to whatever element requested focus internally.

### Tab focus chain
Set "activeFocusOnTab: true" on all elements that should be included in the tab focus chain.
