# Personas
Note: This is a draft only

## Roadwarrior
* Fires up Kontact quickly to see what's up next (it's not constantly open)
* Has to deal with bad/intermitted network connection
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
* Clear layering. No depending on akonadi from everywhere.
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
