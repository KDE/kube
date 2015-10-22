# Architecture / Design

## Overview
Kontact Quick is supposed to be a small and concise codebase that is easy to modify and evolve.

It's following a reactive model, where in one direction we have controllers generating modifications, and in the other direction models updating themselves on changes.

The overall architecture is split into three layers; Ui, Domain Logic and Infrastructure.

```
+----------------------------+
|             UI             |
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

The domain logic layer holds the application state. It povides models to access data and controllers to act upon it. The domain logic is by definition Kontact Quick specific and not sharable with other applications, as it needs to be taylored exactly according to the requirements of Kontact Quick.

The infrastructure layer provides:

* Data access (Akonadi Next)
* Configuration (Config files, etc.)
* Various functionality provided by libraries (email sending, ldap, iTip handling, iCal implementation (kcalcore), vCard implementation, ...)
Various bits of the infrastructure layer may be exchanged on different platforms, to i.e. integrate into native infrastructure providers on a platform.

Note: By using the onion architecture we ensure the infrastructure is exchangable just as well as the UI.

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
External applications, like the KDE calendar plasmoid, should be able to load parts of Kontact Quick when available. It should for instance be possible to load the Event editor as embeddable QML component, that is fully functional. That way it becomes very easy for third parties to provide extra functionality if Kontact Quick is installed, without having to reimplement the Domain Logic (as is the case if only data access is provided through akonadi).

The same mechanism should probably be used by Kontact Quick itself to ensure loose coupling and allow mashups with various content types.

Note: We'll probably want a component-viewer application to easily load and test individual components (similar to plasmoidviewer).

## Testing

* Controllers can be tested by providing mock implementations of the relevant infrastructure parts.
* Models can be tested by providing fake implementations of the relevant infrastructure parts.
* Infrastructure parts can be tested individually.
