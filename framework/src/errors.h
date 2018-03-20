/*
    Copyright (c) 2018 Christian Mollekopf <mollekopf@kolabsys.com>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/
#pragma once

#include <memory>
#include <type_traits>
#include <utility>

#include <QtGlobal>

// A somewhat implementation of the expected monad, proposed here:
// https://isocpp.org/files/papers/n4015.pdf

// A class used to differentiate errors and values when they are of the same type.
template <typename Error>
class Unexpected
{

    static_assert(!std::is_same<Error, void>::value, "Cannot have an Unexpected void");

public:
    Unexpected() = delete;

    constexpr explicit Unexpected(const Error &error) : mValue(error) {}
    constexpr explicit Unexpected(Error &&error) : mValue(std::move(error)) {}

    // For implicit conversions when doing makeUnexpected(other)
    template <typename Other>
    constexpr explicit Unexpected(const Unexpected<Other> &error) : mValue(error.value())
    {
    }
    template <typename Other>
    constexpr explicit Unexpected(Unexpected<Other> &&error) : mValue(std::move(error.value()))
    {
    }

    constexpr const Error &value() const &
    {
        return mValue;
    }
    Error &value() &
    {
        return mValue;
    }

    constexpr const Error &&value() const &&
    {
        return std::move(mValue);
    }
    Error &&value() &&
    {
        return std::move(mValue);
    }

private:
    Error mValue;
};

template <class Error>
Unexpected<typename std::decay<Error>::type> makeUnexpected(Error &&e)
{
    return Unexpected<typename std::decay<Error>::type>(std::forward<Error>(e));
}

template <typename Error>
bool operator==(const Unexpected<Error> &lhs, const Unexpected<Error> &rhs)
{
    return lhs.value() == rhs.value();
}

template <typename Error>
bool operator!=(const Unexpected<Error> &lhs, const Unexpected<Error> &rhs)
{
    return lhs.value() != rhs.value();
}

namespace detail {

namespace tags {
struct Expected
{};
struct Unexpected
{};
} // namespace tags

// Write functions here when storage related and when Type != void
template <typename Error, typename Type>
struct StorageBase
{
protected:

    // To be able to define a copy constructor in a child class
    StorageBase() {}

    // Rule of 5 (copy constructors defined in StorageCopyConstructor) {{{

    StorageBase(StorageBase &&other) : mIsValue(other.mIsValue)
    {
        // This is a constructor, you have to construct object, not assign them
        // (hence the placement new)
        //
        // Here's the problem:
        //
        // Object that are part of a union are not initialized (which is
        // normal). If we replaced the placement new by a line like this:
        //
        // ```
        // mValue = other.mValue;
        // ```
        //
        // If overloaded, this will call `mValue.operator=(other.mValue);`, but
        // since we're in the constructor, mValue is not initialized. This can
        // cause big issues if `Type` / `Error` is not trivially (move)
        // assignable.
        //
        // And so, the placement new allows us to call the constructor of
        // `Type` or `Error` instead of its assignment operator.
        if (mIsValue) {
            new (std::addressof(mValue)) Type(std::move(other.mValue));
        } else {
            new (std::addressof(mError)) Unexpected<Error>(std::move(other.mError));
        }
    }

    StorageBase &operator=(StorageBase &&other)
    {
        this->~StorageBase();
        mIsValue = other.mIsValue;
        if (mIsValue) {
            mValue = std::move(other.mValue);
        } else {
            mError = std::move(other.mError);
        }
        return *this;
    }

    ~StorageBase()
    {
        if (mIsValue) {
            mValue.~Type();
        } else {
            mError.~Unexpected<Error>();
        }
    }

    // }}}

    template <typename... Args>
    constexpr StorageBase(tags::Expected, Args &&... args)
        : mValue(std::forward<Args>(args)...), mIsValue(true)
    {
    }

    template <typename... Args>
    constexpr StorageBase(tags::Unexpected, Args &&... args)
        : mError(std::forward<Args>(args)...), mIsValue(false)
    {
    }

    union
    {
        Unexpected<Error> mError;
        Type mValue;
    };
    bool mIsValue;
};

// Write functions here when storage related and when Type == void
template <typename Error>
struct StorageBase<Error, void>
{
protected:
    constexpr StorageBase(tags::Expected) : mIsValue(true) {}

    template <typename... Args>
    constexpr StorageBase(tags::Unexpected, Args &&... args)
        : mError(std::forward<Args>(args)...), mIsValue(false)
    {
    }

    Unexpected<Error> mError;
    bool mIsValue;
};

// Struct used to add the copy constructor / assignment only if both types are copy constructible
template <typename Error, typename Type,
    bool both_copy_constructible = std::is_copy_constructible<Error>::value &&std::is_copy_constructible<Type>::value>
struct StorageCopyConstructor;

template <typename Error, typename Type>
struct StorageCopyConstructor<Error, Type, true> : StorageBase<Error, Type>
{
protected:
    using StorageBase<Error, Type>::StorageBase;

    StorageCopyConstructor(const StorageCopyConstructor &other) : StorageBase<Error, Type>()
    {
        // If you're thinking WTF, see the comment in the move constructor above.
        this->mIsValue = other.mIsValue;
        if (this->mIsValue) {
            new (std::addressof(this->mValue)) Type(other.mValue);
        } else {
            new (std::addressof(this->mError)) Unexpected<Error>(other.mError);
        }
    }

    StorageCopyConstructor &operator=(const StorageCopyConstructor &other)
    {
        this->mIsValue = other.mIsValue;
        if (this->mIsValue) {
            this->mValue = other.mValue;
        } else {
            this->mError = other.mError;
        }
        return *this;
    }

};

template <typename Error, typename Type>
struct StorageCopyConstructor<Error, Type, false> : StorageBase<Error, Type>
{
protected:
    using StorageBase<Error, Type>::StorageBase;
};


// Write functions here when storage related, whether Type is void or not
template <typename Error, typename Type>
struct Storage : StorageCopyConstructor<Error, Type>
{
protected:
    // Forward the construction to StorageBase
    using StorageCopyConstructor<Error, Type>::StorageCopyConstructor;
};

// Write functions here when dev API related and when Type != void
template <typename Error, typename Type>
struct ExpectedBase : detail::Storage<Error, Type>
{
    constexpr ExpectedBase() : detail::Storage<Error, Type>(detail::tags::Expected{}) {}

    template <typename OtherError>
    constexpr ExpectedBase(const Unexpected<OtherError> &error)
        : detail::Storage<Error, Type>(detail::tags::Unexpected{}, error)
    {
    }
    template <typename OtherError>
    constexpr ExpectedBase(Unexpected<OtherError> &&error)
        : detail::Storage<Error, Type>(detail::tags::Unexpected{}, std::move(error))
    {
    }

    constexpr ExpectedBase(const Type &value)
        : detail::Storage<Error, Type>(detail::tags::Expected{}, value)
    {
    }
    constexpr ExpectedBase(Type &&value)
        : detail::Storage<Error, Type>(detail::tags::Expected{}, std::move(value))
    {
    }

    // Warning: will crash if this is an error. You should always check this is
    // an expected value before calling `.value()`
    constexpr const Type &value() const &
    {
        //FIXME: Q_ASSERT cannot be used in a constexpr with qt 5.9. See also: https://git.qt.io/consulting-usa/qtbase-xcb-rendering/commit/8ea27bb1c669e21100a6a042b0378b3346bdf671
        //Q_ASSERT(this->mIsValue);
        return this->mValue;
    }
    Type &&value() &&
    {
        Q_ASSERT(this->mIsValue);
        return std::move(this->mValue);
    }
};

// Write functions here when dev API related and when Type == void
template <typename Error>
struct ExpectedBase<Error, void> : Storage<Error, void>
{
    // Rewrite constructors for unexpected because Expected doesn't have direct access to it.
    template <typename OtherError>
    constexpr ExpectedBase(const Unexpected<OtherError> &error)
        : Storage<Error, void>(tags::Unexpected{}, error)
    {
    }
    template <typename OtherError>
    constexpr ExpectedBase(Unexpected<OtherError> &&error)
        : Storage<Error, void>(tags::Unexpected{}, std::move(error))
    {
    }
};

} // namespace detail

// Write functions here when dev API related, whether Type is void or not
template <typename Error, typename Type = void>
class Expected : public detail::ExpectedBase<Error, Type>
{
    static_assert(!std::is_same<Error, void>::value, "Expected with void Error is not implemented");

public:
    using detail::ExpectedBase<Error, Type>::ExpectedBase;

    constexpr const Error &error() const &
    {
        return this->mError.value();
    }

    constexpr bool isValue() const
    {
        return this->mIsValue;
    }
    constexpr explicit operator bool() const
    {
        return this->mIsValue;
    }
};
