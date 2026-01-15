/*
 * InputLeap -- mouse and keyboard sharing utility
 * Copyright (C) 2012-2016 Symless Ltd.
 * Copyright (C) 2002 Chris Schoeneman
 *
 * This package is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * found in the file LICENSE that should have accompanied this file.
 *
 * This package is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "common/common.h"

// Windows-only includes
#include "arch/win32/ArchDaemonWindows.h"
#include "arch/win32/ArchLogWindows.h"
#include "arch/win32/ArchMiscWindows.h"
#include "arch/win32/ArchMultithreadWindows.h"
#include "arch/win32/ArchNetworkWinsock.h"
#include "arch/win32/ArchSystemWindows.h"
#include "arch/win32/ArchTaskBarWindows.h"

#include <mutex>

namespace inputleap {

/*!
\def ARCH
This macro evaluates to the singleton Arch object.
*/
#define ARCH    (Arch::getInstance())

//! Delegating implementation of architecture dependent interfaces
/*!
This class is a centralized interface to all architecture dependent
interface implementations (except miscellaneous functions).  It
instantiates an implementation of each interface and delegates calls
to each method to those implementations.  Clients should use the
\c ARCH macro to access this object.  Clients must also instantiate
exactly one of these objects before attempting to call any method,
typically at the beginning of \c main().
*/
class Arch : public ARCH_DAEMON,
                public ARCH_LOG,
                public ARCH_MULTITHREAD,
                public ARCH_NETWORK,
                public ARCH_SYSTEM,
                public ARCH_TASKBAR {
public:
    Arch();
    Arch(Arch* arch);
    virtual ~Arch();

    //! Call init on other arch classes.
    /*!
    Some arch classes depend on others to exist first. When init is called
    these clases will have ARCH available for use.
    */
    void init() override;

    //
    // accessors
    //

    //! Return the singleton instance
    /*!
    The client must have instantiated exactly once Arch object before
    calling this function.
    */
    static Arch* getInstance();

    static void setInstance(Arch* s) { s_instance = s; }

private:
    static Arch*        s_instance;
};

} // namespace inputleap
