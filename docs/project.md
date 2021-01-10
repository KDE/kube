# What is Kube

Kube is a personal information and communication application.
This includes time and task-management (the time available, and what it's used for), as well as the necessary communication around it (currently based on email).

## Principles

These are some of the principles Kube is built on. They are intended as a guide, and not a rule.

* Simple
    * Strive for simplicity in UI and Code
    * Avoid options if possible
    * Minimal configuration
    * Opinionated where necessary
* Usable Security
    * Don't sacrifice usability for security
* Portable
    * to operating systems: Windows/macOS/Linux/...
    * to desktops: Plasma/Gnome/....
    * to formfactors: Desktop/Mobile/Tablet
* Content focused
    * Low-key presentation
    * Out of your way
    * Plaintext over HTML (simple?)
* Networked
    * Avoid local only data (e.g. local only tags)
    * The backends decide what we can work with
* Clear data ownership
    * All data belongs to individual accounts
    * No shared data with unclear account synchronization
* Maintainable
    * Comparatively small codebase
    * Self contained
    * Few dependencies
* Supportable
    * Logging
    * Scriptable

Features:
* EMail/Addressbook/Calendar/Tasks
* GPG based end to end encryption
* Conversation View
* Fulltext search, including encrypted messages
* Read-only support for protected headers
* Keyboard navigation

# Current state

* Usable as daily driver
* Requires a companion interface for tasks not yet implemented (e.g. webmail to create folders)

# Future

The focus is on quality of life improvements and features that are missing elsewhere,
rather than trying reach feature parity with other popular clients.

## Differentiators
In order to avoid simply replicating what's already existing it's important to know how this product differentiates to other existing solutions.
This section is supposed to outline that.

* To Roundcube
    * Native application
        * Responsivness of UI (assuming we can't reach that in the browser)
        * Not in a browser (assuming we can't effectively hide that)
        * Desktop integration (notifications, startmenu)
    * Offline capability
    * Cryptography
    * Multi-account support

* To Kontact/Thunderbird/...
    * Simple but powerful UI
    * Performance (lower resource usage, quick and responsive operations)
    * Easy & automated setup (scriptable setup process, syncable configuration, setup can easily be nuked and setup from scratch)
    * Codebase
        * Well automated tested
        * Efficient further development
        * Codebase can go to mobile platforms as well

* To existing mobile applications
    * Complementing usecases that make the overall product more useful (A mobile/tablet could even be the preferred interface for some interactions due to additional screen and touch capabilities)
        * Categorization
        * What's next, schedule checking
        * Todolist view
        * Notification center
    * Better integration with kolab

# Vision Statement
Kube aims to be an enterprise-ready PIM solution, that has a high-quality and rock solid core. The focus of the core is on high-quality code, maintainability, stability and performance.

We strive to keep the core to the necessary minimum, with minimal dependencies and maximum portability, and in a way that it is maintainable by a small team.
We also strive to keep the solution agile so that work by corporate partners can be executed upstream.

Experimental or advanced features are supported as optional addons, to not affect the high quality of the core product.

Kube aims to be available on various form-factors and platforms.

# Project Structure
While this is an open project that welcomes participation from everyone who's interested, we do have an explicit team strucuture to ensure it's clear to everyone who's repsonsible for what. External contributions are always welcome and the team is of course open for extension.

Team Lead/Maintainer: Christian Mollekopf

It's the team leads responsibility to:

* Organize regular online meetings
* Give direction to the product
* Direct development and oversee decisions
* Ensure documentation of decisions

## Decision making process
Decisions are generally made in discussion with the team, and result in documentation or an item on the roadmap. Non-trivial decisions are always either discussed on the mailinglist or in an online meeting.

Should the team not be able to reach consensus, the team lead makes the final decision.

## Planning
All planning happens on [invent.kde.org](https://invent.kde.org/pim/kube), the old planning is avialable on [KDE Phabricator instance](https://phabricator.kde.org/tag/kube/).

## Releases / Milestones
Releases will follow achieved milestones. Milestones are assembled from tasks on the roadmap.

## Versioning
The product will follow the semantic versioning scheme (semver.org), with each feature release corresponding to a milestone on phabricator.

## Git repository
The git repository can be found here: [https://invent.kde.org/pim/kube.git](https://invent.kde.org/pim/kube.git)

The "master" branch is used for the latest development version, releases are tagged. Branches for specific versions that receive patch releases may be created as required.

For new developments use feature branches prefixed with "dev/".
