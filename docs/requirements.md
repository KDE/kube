# Personas
Note: This is a draft only

## Roadwarrior
* Fires up Kube quickly to see what's up next (it's not constantly open)
* Has to deal with bad/intermittent network connection
* Relies on offline capabilities to access content
* Uses various mobile devices

## Average Desktop User
* Relies on notifications to immediately see when he's contacted.
* Requires a simple and efficient UI
* Doesn't customize a lot
* Has constant internet access
* Works 9 to 5, doesn't care outside of business hours

## Power Desktop User
* Inherits Average Desktop User Requirements
* Regularly checks email on the go (mobile)
* Regularly checks calendar on the go (mobile)
* Creates events on the go (mobile)

# Kontact Inventory
We need to go through the current codebase, assess what features are available, how the implementation looks and where it is, and to what extent the code is reusable.
This will help us in figuring out the useful feature set, and will allow us to reuse the lessons learned that are embedded in the codebase.

This inventory is currently hosted in an ikiwiki (ikiwiki.info) git repository(kde:scratch/aseigo/KontactCodebaseInventory.git)

# Target platforms
The codebase is supposed to be portable across a range of platforms.
Initially we'll work on a reasonably recent linux distribution though.

The aimed for minimum bar is:

Linux:

* Fedora 22
* Ubuntu 12.04
* Centos/RHEL 7

Windows:

* 7sp1

OS X:

* ?

Android:

* 5.0

# Dependencies
Since the codebase needs to be portable across various platforms old and new, dependencies should be managed that they are as little and as low as possible. While we don't want to reinvent the wheel constantly or work with ancient technology, each additional dependency or dependency bump needs to be justified and we need to evaluate wether that results in a problem with any of the target platforms. This evaluation of course includes transitive dependencies.

Currently available dependencies:

* GCC 4.6.3 / MSVC 2013
* Qt 5.2

# Codebase

## Requirements
* Each module has at least rudimentary tests that can then be extended
    * Tests need to be deterministic, no random timeouts to check if something already happened, only `QTRY_VERIFY` and alike is allowed.
* Clear layering. No depending on Sink from everywhere.
* Each module comes with a clear set of justified dependencies.
* Commented code is only allowed in conjunction with a task in phabricator. No dead/commented code.
* Each module requires a clear interface that allows the module internals to be replaced eventually.
* UI modules need to be separated from non-UI parts. All UI parts need to be eventually replacable by QML equivalents.
* No dialogs in non-UI parts.
* New features are only added after having been selected from the roadmap for a future release
* An accounts based configuration for everything
* No non-persistant data in config files (collection ids...)

* No KParts
* No KXMLGui

## Guidelines
* Singletons that hold a modifyable state should be avoided.
* Where standards are available we strive to follow those, and deviations from the standard are avoided as much as possible. Repurposing of standard elements should be avoided altogether.
* Fallbacks (i.e. for configs ), should be applied in a single place only, and should be avoided wherever possible.
* Libraries need to be purpose built and with clear responsibilities. No artificial boundaries that don't help something.
* Modal dialogs should be avoided.

## Coding Guidelines
* Run the tests before you push

# Roadmap
The final roadmap lives on phabricator.kde.org. This section tries to outline some of the high level aims that should help form the roadmap.

## Priorities
* High Priority
    * kolab support
    * reliability/maintainability (testability)
    * portability
    * usability
    * Mobile
    * QML interface
    * Configuration Synchronization
    * Automatic setup
    * Enterprise environment support
    * Professional product, and actual alternative to competitors
    * Open Protocols/Standards
    * Multi Accounts & Identities
    * Crypto / Privacy
    * Fast at scale
* Medium Priority
    * Search / Tags
    * More integrated with the rest of kolab
    * Closer integration between mail/tasks/events/addressbook
    * Well supported
    * Community
    * Automation (Wallace)
    * Extensible / Theming
    * Multiple open instances
    * Offline support

## Features
A list of features that has to be refined and put on the roadmap on phabricator.
This is very much WIP and the features listed here are largely coming from what is existing in Kontact and the Kolab Groupware Server.

### General
* autoconfiguration/automatic setup/preconfiguration: The complete setup process should require a minimum of configuration and should be fully scriptable.

### Mail
* folderlist (with search)
* smart folders
* multi account & identity
* threading/conversation view
* actions
    * flags: read/unread/important
    * delete/move/copy
    * reply to/reply to all/forward
    * bulk operations on selected/thread
    * move to special folder
        * archive
        * move to trash
* attachments
* crypto
* search
* tags
* create event/todo from mail
* snippets
* mail composer
* shared folders / acls
* undo

### Calendar
* calendarlist (with search)
* smart calendars
* multi account & identity
* create/edit/modify event/todo (journal?)
* week/month view
* ical import/export
* delegation of events/todos
* iTip handling
* freebusy for scheduling
* tags

### Notes
* notebooks
* create/edit/modify note
* tags
* note editor
    * title/content

### Todos
* todolists
* create/edit/modify todo
* tags
* todo editor
    * summary/content/start date/due date
* delegation of todos

## Feature Brainstorming
* Why is it the sender of a message that dictates how/where I receive/read the message?
* VOIP system knows when you're away, allows to forward the call to your mobile

### Desktop
* Autocomplete conversations from sent folder: Automatically merge sent messages that belong to a conversation/thread from the sent folder (making it unnecessary to send a copy to yourself or alike)
* Inbox for everything: upcoming events, uncategorized todos, open invitations, email, delegated todos. This could either be a mixed inbox for everything (as in what you have to go through), or an overview page with multiple inboxes.
* No invitation in mail inbox: Mail is a transport mechanism and there is little reason to clutter your mail inbox with invitations. So move invitations to a separate queue.
* Fuzzy match on folder search: It should be as easy and fast as command-t for vim (meaning as fast as you type)

### Mobile
* Swipe left right through email inbox (tinder for kolab aka "kinder")
    * Same works for invitations (accept/decline)
    * Same for todos, done/do later (if not touched keep for today)
* Quick inline reply in mails (what's app style)
* Note taking/todo management on the run, with categorization workflow on the desktop (or also on mobile)

## Platforms
Desktop linux is the primary platform that we'll pursue first, because it's the easist target. As soon as we have a viable proof of concept and the architecture is set, Windows and OS X will follow, so we ensure early on that the project works on all platforms.

Android will be tried in the form of a research project, and depending on the difficulties we face the situation will be reevaluated.

# Deliverables
These are the high-level aims that we have to work towards. This list is not a final list of deliverables, but should convey the areas we need to work on. More detailed information should eventually be available on phabricator.

* Project Vision
    * Target Users & Usecases
        * Personas
        * Scenarios
        * Description of environment
    * UI Mockups for envisioned clients
    * The target feature-set

* Milestones
    * First working product: A simple email application for the linux desktop
        * read-only first
        * read-write second
    * Some intermediate releases: Largely depends on what deliverables we want, and wether we can use releases that only contain a subset of the groupware types.
        * Application by application (calendar, email, ...)
        * First release on other platforms (e.g. android)
    * Production ready (1.0): Includes calendar, email, addressbook, notes, tasks with basic functionality (which we need to define)
    * From here on we implement feature by feature from the roadmap

* Implementation
    * Inventory of exiting kdepim: This will help to fill the functional blocks, and help in carving out the require featureset.
    * Functional blocks: We need to identify the function blocks that we require, see to what extent they are already existing and how we can reuse what's there already. The functinal blocks should largely follow from the identified requirements.
    * Prototype the domain logic: We need to prototype the domain logic as envisioned to see wether that works out. This will be an ongoing process especially while working towards the first milestone.
    * Prototype with domain logic + Sink + trivial UI. Show that this can work in it's basics.
