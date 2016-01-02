# Architecture / Design

## Overview
Kube is supposed to be a small and concise codebase that is easy to modify and evolve.

It's following a reactive model, where in one direction we have controllers generating modifications, and in the other direction models updating themselves on changes.

The overall architecture is split into three layers; Ui, Domain Logic and Infrastructure.

```
+----------------------------+
|      UI / Application      |
+----------------------------+
|                            |
|        Domain Logic        |
|                            |
+--------------+------+------+
|              |      |      |
| Akonadi Next |Config| ...  |
|              |      |      |
+--------------+------+------+
```

The UI Layer consists of views (mostly written in QML), view-models (models that are view specific and potentially implement user interaction details), and the glue code to use various controllers from the interface. Different UI layers may exist for different form factors.

The domain logic layer holds the application state. It povides models to access data and controllers to act upon it. The domain logic is by definition Kube specific and not sharable with other applications, as it needs to be taylored exactly according to the requirements of Kube.

The infrastructure layer provides:

* Data access (Akonadi Next)
* Configuration (Config files, etc.)
* Various functionality provided by libraries (email sending, ldap, iTip handling, iCal implementation (kcalcore), vCard implementation, ...)
Various bits of the infrastructure layer may be exchanged on different platforms, to i.e. integrate into native infrastructure providers on a platform.

Note: By using the onion architecture we ensure the infrastructure is exchangable just as well as the UI.
Note: The domain objects might also be specified in Akonadi Next.

## UI / Application
The UI / Application layer contains all the views, and their composition, that make up the application.
All the interactions between the different components are defined here.

### UI Component Interaction
UI components will have to be able to:

* Load certain views directly inline (e.g. load a calendar into a mail with an invitation, or a maileditor into the maillist)
* Change the overall application state by i.e. triggering the calendar to show a certain date-range.

This is ideally decoupled in a way that the provider of such a service can be replaced without changing the users of the service. Additionally it should be possible for external applications (such as a plasmoid), to load the same views, possibly through the same mechanism.

## Domain Logic

### Models
Self-updating models are used to implement the read-only part of the application.
By using QAbstractItemModels we can reuse the existing update mechanism, have something that works well with QML, and avoid implementing a boatload of boilerplate code for hand-coded domain objects.

Models should always be reactive and configured with a query, so they are asynchronous.

By implementing everything according to that model we can later on achieve lazy-loading of properties and zero-copy (or at least close to zero-copy) directly from storage without modifying anything besides data access.

### Controllers
Use-case specific controllers are used to operate on the data. Controllers allow to completely separate the modifications from the view.
Rather than splitting controllers by domain type (e.g. an email controller, or a calendar controller), we specifically write controllers for specific usecases (e.g. an email editor), that exposes all required actions. That way we ensure that the API's a UI is working with is always clear an consice, and that we have all domain logic captured in the domain logic layer, rather than the UI layer.
Of course controllers will need to share functionality internally as soon as an action is available from more than one place.

### Email Domain Logic
* Folder list
    * Folder List Controller
        * Move mail to folder
        * Move/Copy/Delete folder
        * Synchronize folder
    * Folder List Model
        * Mixes akonadi next queries and subqueries (folder list with smart folders)
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

## Infrastructure

The infrastructure layer interfaces with the rest of the system. It is the place where we can integrate with various native infrastructure parts.
The interface of the infrastructure layer, that is used by the domain logic, may not expose any implementation details of any infrastructure part, to ensure that all infrastructure parts are exchangable.

Note: The infrastructure blocks will use only types provided by the domain logic. This means i.e. that no KCalCore containers may be used in such an interface. To avoid hard dependencies on any specific implementation, the infrastructure parts will have to have interfaces, and a factory must be used to supply concrete implementations. That way it is also possible to inject dummy implementations.

### Akonadi Next
Akonadi Next is used for primary data access and handles all synchronization.

Interactions with Akonadi Next involve:
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

## Interaction with external applications
External applications, like the KDE calendar plasmoid, should be able to load parts of Kube when available. It should for instance be possible to load the Event editor as embeddable QML component, that is fully functional. That way it becomes very easy for third parties to provide extra functionality if Kube is installed, without having to reimplement the Domain Logic (as is the case if only data access is provided through akonadi).

The same mechanism should probably be used by Kube itself to ensure loose coupling and allow mashups with various content types.

Note: We'll probably want a component-viewer application to easily load and test individual components (similar to plasmoidviewer).

## Testing

* Controllers can be tested by providing mock implementations of the relevant infrastructure parts.
* Models can be tested by providing fake implementations of the relevant infrastructure parts.
* Infrastructure parts can be tested individually.

# The new design

The application consists of various application components. A component could be a maillist, an event-editor or the complete kube-mail application. Each component is instantiable on it's own, and has an API to interact with it. The API i.e. allows to set a folder for the maillist, or an event for the event-editor. Components can be nested (a component can instantiate another component)

Components are requested from a factory, allowing to return different versions depending on modifiers passed in, or the global application context (i.e. mobile variants of the UI).

A component primarily is a QML UI.
The QML UI is built on top of:
* One or more models that are instantiated to provide the data.
* Actions that are instatiated in QML.

## Models

Models are self contained and have an API to set i.e. a query for what to load. Models can load data from anywhere.


## Actions

An action represents something that can be done, such as "mark as read", "delete", "move somewhere".

An action has:
* an id (org.kube.mail.make-as-read)
* an active state (a property for the UI to know when the action can be triggered, that changes depending on the context)
* an action context, which contains everything the action needs to execute.
* an action icon
* an action name

The action context contains the dataset the action works upon plus any additional information that is required. A mark-as-read action for instance only requires the mail-set to work on, while a tag-with action requires any entity (mail, event, ...) and a tag (unless there is one action per tag...).
The action can through property-binding reevaluate its active state based on the currently set context that the UI continuously updates through property binding. Context objects can be shared by various actions.

### Automatic action discovery
While in may places explicit instantiation of actions is desirable, sometimes we may want to offer all available actions for a certain type. For this it should be possible to i.e. query for all actions that apply to a mail. That way it is possible to centrally add a new action that automatically becomes available everywhere. Note that this only works for actions that don't require an additional UI, since the components would have to embedd that somewhere.

### Implementation

Actions could be simply objects that provide the API, and that QML can instantiate directly with it's id. The C++ implementation of the action can then lookup the action implementation using the id and call it with it's context when executed.

## Component interaction

The application is made up of various nested components that sometimes need to interact with each other.

If we look at the example of the org.kube.mail component:
1. The folderlist-component current-folder property is connected to maillist parentFolder property to display the mails of the currently selected folder.
2. The "add-note" action might either switch the application state to the org.kube.note application, or it might just display a quick-note widget directly inline.

The first usecase can be achieved by the parent component doing a property binding to connect the different components together as desired.

The second usecase requires actions to interact with 'a' parent component, but without knowing with which one. Actions can thus post requests for application state changes, that bubble up through the components and can be catched by any of the parent components.

This makes it possible for i.e. a maillist to display a note-widget directly inline, or handing the request up to the parent component which could show a full note editor.
And if nothing handles the request it bubbles up to the root component (the shell), which can then switch to the note application component.

### Application state changes requests

A request always requires a context that tells the handler what to do. This could be the note to edit, or the date-range to display in the calendar, or the recepient for the mail composer.
