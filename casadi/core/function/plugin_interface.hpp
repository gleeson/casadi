/*
 *    This file is part of CasADi.
 *
 *    CasADi -- A symbolic framework for dynamic optimization.
 *    Copyright (C) 2010 by Joel Andersson, Moritz Diehl, K.U.Leuven. All rights reserved.
 *
 *    CasADi is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License as published by the Free Software Foundation; either
 *    version 3 of the License, or (at your option) any later version.
 *
 *    CasADi is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with CasADi; if not, write to the Free Software
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef CASADI_PLUGIN_INTERFACE_HPP
#define CASADI_PLUGIN_INTERFACE_HPP

#include "../function/function_internal.hpp"

// For dynamic loading
#ifdef WITH_DL
#ifdef _WIN32 // also for 64-bit
#include <windows.h>
#else // _WIN32
#include <dlfcn.h>
#endif // _WIN32

// Set default shared library prefix
#ifndef SHARED_LIBRARY_PREFIX
#define SHARED_LIBRARY_PREFIX "lib"
#endif // SHARED_LIBRARY_PREFIX

// Set default shared library suffix
#ifndef SHARED_LIBRARY_SUFFIX
#define SHARED_LIBRARY_SUFFIX ".so"
#endif // SHARED_LIBRARY_SUFFIX
#endif // WITH_DL

namespace casadi {

  /** \brief Interface for accessing input and output data structures
      \author Joel Andersson
      \date 2013
  */
  template<class Derived>
  class CASADI_CORE_EXPORT PluginInterface {
    public:

    /// Fields
    struct Plugin{
      typename Derived::Creator creator;
      const char* name;
      const char* doc;
      int version;
    };

    // Plugin registration function
    typedef int (*RegFcn)(Plugin* plugin);

    /// Load a plugin dynamically
    static void loadPlugin(const std::string& name);

    /// Register an integrator in the factory
    static void registerPlugin(RegFcn regfcn);

    /// Load and get the creator function
    static Plugin& getPlugin(const std::string& name);
  };

  template<class Derived>
  void PluginInterface<Derived>::loadPlugin(const std::string& name) {
    // Issue warning and quick return if already loaded
    if (Derived::solvers_.find(name) != Derived::solvers_.end()) {
      casadi_warning("PluginInterface: Solver " + name + " is already in use. Ignored.");
      return;
    }

#ifndef WITH_DL
    casadi_error("WITH_DL option needed for dynamic loading");
#else // WITH_DL
    // Retrieve the registration function
    RegFcn reg;

    // Get the name of the shared library
    std::string lib = SHARED_LIBRARY_PREFIX "casadi_"
      + Derived::infix_ + "_" + name + SHARED_LIBRARY_SUFFIX;

    // Load the dll
    void* handle;
    std::string regName = "casadi_register_" + Derived::infix_ + "_" + name;
#ifdef _WIN32
    handle = LoadLibrary(TEXT(lib.c_str()));
    casadi_assert_message(handle!=0, "PluginInterface::loadPlugin: Cannot open function: "
                        << lib << ". error code (WIN32): "<< GetLastError());

    reg = (RegFcn)GetProcAddress(handle_, TEXT(regName.c_str()));
    if (reg==0) throw CasadiException("PluginInterface::loadPlugin: no \"" + regName + "\" found");
#else // _WIN32
  handle  = dlopen(lib.c_str(), RTLD_LAZY);
  casadi_assert_message(handle!=0, "PluginInterface::loadPlugin: Cannot open function: "
                        << lib << ". error code: "<< dlerror());

  // reset error
  dlerror();

  // Load creator
  reg = (RegFcn)dlsym(handle, regName.c_str());
  if (dlerror()) throw CasadiException("PluginInterface::loadPlugin: no \"" + regName + "\" found");
#endif // _WIN32

    // Register the plugin
  registerPlugin(reg);
#endif // WITH_DL
  }

  template<class Derived>
  void PluginInterface<Derived>::registerPlugin(RegFcn regfcn) {
    // Create a temporary struct
    Plugin plugin;

    // Set the fields
    int flag = regfcn(&plugin);
    casadi_assert(flag==0);

    // Check if the solver name is in use
    typename std::map<std::string, Plugin>::iterator it=Derived::solvers_.find(plugin.name);
    casadi_assert_message(it==Derived::solvers_.end(),
                          "Solver " << plugin.name << " is already in use");

    // Add to list of solvers
    Derived::solvers_[plugin.name] = plugin;
  }

  template<class Derived>
  typename PluginInterface<Derived>::Plugin&
  PluginInterface<Derived>::getPlugin(const std::string& name) {

    // Check if the solver has been loaded
    typename std::map<std::string, Plugin>::iterator it=Derived::solvers_.find(name);

    // Load the solver if needed
    if (it==Derived::solvers_.end()) {
      loadPlugin(name);
      it=Derived::solvers_.find(name);
    }
    casadi_assert(it!=Derived::solvers_.end());
    return it->second;
  }

} // namespace casadi


#endif // CASADI_PLUGIN_INTERFACE_HPP