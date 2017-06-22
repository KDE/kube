# Why?
This project started with the aim to produce a product that doesn't cater to all usecases, but does what it does well.
We want a codebase that is well maintainable by a small team, and that can move fast.
We want a codebase where it is fast and easy to prototype new features and turn them eventually into full implementations, without compromising the quality of the rest of the system. Additionaly the product should be portable accross a variety of platforms, including mobile, not only due to a portable codebase, but also due to different UI's for the various formfactors.

Because no existing codebase fullfills those premises or easily allows to reach them, this project started.


## Differentiators
In order to avoid simply replicating what's already existing it's important to know how this product differentiates to other existing solutions.
This section is supposed to outline that.

* To Roundcube Next
    * Native application
        * Responsivness of UI (assuming we can't reach that in the browser)
        * Not in a browser (assuming we can't effectively hide that)
        * Desktop integration (notifications, startmenu)
    * Offline capability
    * Cryptography
    * Multi-account support

* To Kontact/Thunderbird/...
    * Simple but powerful UI (assuming we can achieve that)
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
Team Members: Michael Bohlender, Sandro Knauss, Aaron Seigo

It's the team leads responsibility to:

* Organize regular online meetings (so far on google hangouts)
* Give direction to the product and ensure it's followed
* Direct development and oversee decisions
* Ensure documentation of decisions

## Decision making process
Decisions are generally made in discussion with the team, and result in documentation or an item on the roadmap. Non-trivial decisions are always either discussed on the mailinglist or in an online meeting.

Should the team not be able to reach consensus, the team lead makes the final decision.

## Planning
All planning happens on the [KDE Phabricator instance](https://phabricator.kde.org/tag/kube/).

## Releases / Milestones
Releases will follow achieved milestones. Milestones are assembled from tasks on the roadmap.

## Versioning
The product will follow the semantic versioning scheme (semver.org), with each feature release corresponding to a milestone on phabricator.

## Git repository
The git repository can be found here: [git://anongit.kde.org/kube](git://anongit.kde.org/kube)

The "development" branch is used for the latest development version, master is always stable and always releasable. Branches for specific versions that receive patch releases may be created as required.

For new developments use feature branches prefixed with "feature/" or simply use the "git flow" tool.
