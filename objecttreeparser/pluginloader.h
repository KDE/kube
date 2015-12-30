/*  -*- c++ -*-
    This file is part of libkdepim.

    Copyright (c) 2002,2004 Marc Mutz <mutz@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef PLUGINLOADER_H
#define PLUGINLOADER_H

#include "pluginloaderbase.h"

/**
 * @short A generic plugin loader for when KPart::Plugin is overkill
 * @author Marc Mutz <mutz@kde.org> based on KABC's FormatFactory
 *
 * This is a generic plugin loader / factory for small plugins that
 * don't want to be QObjects.
 *
 * @section Usage
 *
 * A PluginLoader takes two template arguments, <code>T</code> and
 * <code>T_config</code>:
 *
 * <dl>
 * <dt>T</dt><dd>The type of object to return</dd>
 * <dt>T_config::mainfunc</dt><dd>The suffix of the factory function to call
 *          in the library to obtain a new object of type <code>T</code>.
 *          The string passed to <code>KLibrary::symbol()</code> is
 *          <code>libName_mainfunc</code>.</dd>
 * <dt>T_config::path</dt><dd>The search pattern for <tt>.desktop</tt> files
 *          containing the plugin descriptions. This is the string passed as
 *          the @p filter argument to
 *          <code>KStandardDirs::findAllResources</code>.</dd>
 * </dl>
 *
 * The last two parameters being strings, they are passed via an
 * encapsulating class, of which <code>mainfunc</code> and
 * <code>path</code> are public static members:
 *
 * <pre>
 * struct MyObjectPluginLoaderConfig {
 *   static const char * const mainfunc;
 *   static const char * const path;
 * };
 * const char * const MyObjectPluginLoaderConfig::mainfunc = "myapp_create_myobject";
 * const char * const MyObjectPluginLoaderConfig::path = "myapp/plugins/ *.desktop";
 * </pre>
 *
 * You would then use a <tt>typedef</tt> to create a less unwieldy
 * name for your plugin loader:
 *
 * <pre>
 * typedef PluginLoader< MyObject, MyObjectPluginLoaderConfig > MyObjectPluginLoader;
 * </pre>
 *
 * All of this is what the
 * <code>DEFINE_PLUGIN_LOADER(pluginloadername,type,mainfunc,path)</code> macro
 * achieves.
 *
 **/
template< typename T, typename T_config >
class PluginLoader : public PluginLoaderBase
{
protected:
    PluginLoader() : PluginLoaderBase() {}

private:
    static PluginLoader<T, T_config> *mSelf;

public:
    virtual ~PluginLoader()
    {
        mSelf = 0;
    }

    /** Returns the single instance of this loader. */
    static PluginLoader<T, T_config> *instance()
    {
        if (!mSelf) {
            mSelf = new PluginLoader<T, T_config>();
            mSelf->scan();
        }
        return mSelf;
    }

    /** Rescans the plugin directory to find any newly installed
        plugins.
    **/
    void scan() Q_DECL_OVERRIDE {
        doScan(T_config::path);
    }

    /** Returns a pointer to a plugin object (of type @p T) or a null
        pointer if the type wasn't found. You can extend this method
        for when you want to handle builtin types */
    virtual T *createForName(const QString &type) const
    {
        auto main_func = mainFunc(type, T_config::mainfunc);
        if (!main_func) {
            return 0;
        }

        // cast to a pointer to a function returning T*, call it and
        // return the result; don't you love C? ;-)
        return ((T * (*)())(main_func))();
    }
};

template< typename T, typename T_config >
PluginLoader<T, T_config> *PluginLoader<T, T_config>::mSelf = 0;

#define DEFINE_PLUGIN_LOADER( pl, t, mf, p ) \
    namespace { /* don't pollute namespaces */ \
    struct Q_DECL_EXPORT pl##Config { \
        static const char * const mainfunc; \
        static const char * const path; \
    }; \
    const char * const pl##Config::mainfunc = mf; \
    const char * const pl##Config::path = p; \
    } \
    typedef PluginLoader< t, pl##Config > pl; \

#endif // PLUGINLOADER_H
