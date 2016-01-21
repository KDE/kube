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
|        Actions/Models      |
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
* Actions that are instantiated in QML.

## Component interaction
The application is made up of various nested components that often need to interact with each other.

If we look at the example of the org.kube.mail component:
1. The folderlist-component current-folder property is connected to maillist parentFolder property to display the mails of the currently selected folder.
2. The "add-note" action might either switch to the org.kube.note application as currently displayed component, or it might just display a quick-note widget directly inline.

The first usecase can be achieved by the parent component doing a property binding to connect the different components together as desired.

The second usecase requires actions to interact with 'a' parent component, but without knowing with which one. Actions can thus be handled by ActionHandlers anywhere in the application.

This makes it possible for i.e. a maillist to display a note-widget directly inline, or letting the parent component handle the action to show a full note editor.
If nothing handles the action, the root component (the shell)can switch to the note application component.

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

### Actions
An action represents something that can be done, such as "mark as read", "delete", "move somewhere", but also "show this mail" or "give me a composer to write a mail".

An action has:
* an id (i.e. org.kube.actions.make-as-read)
* an active state (a property for the UI to know when the action can be triggered, that changes depending on the context)
* an action context, which contains everything the action needs to execute.
* an icon
* a name

The action context contains the dataset the action works upon plus any additional information that is required. A mark-as-read action for instance only requires the mail-set to work on, while a tag-with action requires any entity (mail, event, ...) and a tag (unless there is one action per tag...).
The action can, through property-binding, reevaluate its active state based on the currently set context that the UI continuously updates through property binding. Context objects can be shared by various actions.

#### Automatic action discovery
While in many places explicit instantiation of actions is desirable, sometimes we may want to offer all available actions for a certain type. For this it should be possible to i.e. query for all actions that apply to a mail. That way it is possible to centrally add a new action that automatically becomes available everywhere. Note that this only works for actions that don't require an additional UI, since the components would have to embedd that somewhere.

#### Implementation
Actions are objects that provide the API, and that QML can instantiate directly with it's id. The C++ implementation looks up the action handler via a broker.

* Action: The interface to execute/launch the action. Forwards request and context to broker.
* ActionHandler: A handler for a specific action. Registers itself with the broker.
* ActionBroker: Forwards action requests to handlers.
* Context: The context containing everything the handler needs to execute the action.

### Controller
For every domain object a controller is implemented to be able to edit the domain object. The domain object is a QObject with a QObject-property for every property of th edomain object and a QValidator, so editors can easily be build using property binding while providing property-level validation and feedback.

The domain object is exposed as an opaque QVariant that can i.e. be used in an action-context. This way details from the infrastructure layer don't leak to the UI layer

### Notifications
The system will provide notifications from various sources. 

Notifications could be:

* New mails arrived
* An error occurred
* A synchronization is in progress
* ...

Notifications can be displayed in various places of the application.

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
Configuration as traditionally stored in config files in ~/.kde

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

### Cryptography
* PGP, PEP
* ObjectTreeParser

Keyselection, encryption, decryption, signing
Probably requires access to identities in some way.

### MIME-Message parsing
* ObjectTreeParser
* KMime

## Testing

TBD

## Problems/Notes:
* Dynamic switching between various component UI's can be solved using KPackage
* change-requests sometimes also need to be handled by sub-components
** If you hit the reply button in the main view, but replies always work inline in the messagelist, the messagelist should still be able to capture the request and handle it. But if the request only goes to parent objects, that is not possible. Perhaps we need a pub-sub mechanism.

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

