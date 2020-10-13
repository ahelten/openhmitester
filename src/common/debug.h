// -*- mode: c++; c-basic-offset: 4; c-basic-style: bsd; -*-
/*
 *   This program is free software; you can redistribute it and/or
 *   modify
 *   it under the terms of the GNU Lesser General Public License as
 *   published by the Free Software Foundation; either version 3.0 of
 *   the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *   02111-1307 USA
 *
 *   This file is part of the Open-HMI Tester,
 *   http://openhmitester.sourceforge.net
 *
 */
#ifndef DEBUG_H
#define DEBUG_H

#include <cassert>
#include <iostream>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <qlogging.h>

///
/// custom debug
///

#define DEBUG_ENABLED (D_ERROR | D_PLAYBACK | D_RECORDING)

#undef qDebug
#define qDebug() QMessageLogger(__FILE__, __LINE__, __FUNCTION__).debug()
#undef qWarning
#define qWarning() QMessageLogger(__FILE__, __LINE__, __FUNCTION__).warning()
#undef qCritical
#define qCritical() QMessageLogger(__FILE__, __LINE__, __FUNCTION__).critical()

/// The number of nano seconds in a second
static const uint32_t NSECS_IN_A_SEC = 1000000000;
/// The number of milli seconds in a second
static const uint16_t MSECS_IN_A_SEC = 1000;
/// The number of nano seconds in a millisecond (convert msecs->nsecs)
static const uint32_t NSECS_IN_A_MSEC = (NSECS_IN_A_SEC / MSECS_IN_A_SEC);

inline std::string timeNow()
{
    timespec t;
    std::string ret;
    if (clock_gettime(CLOCK_REALTIME, &t) == 0) {
        std::tm tmTime;
        if (localtime_r(&t.tv_sec, &tmTime) != nullptr) {
            std::stringstream ss;
            ss << std::put_time(&tmTime, "%Y-%m-%d %X");
            ss << "." << std::setfill('0') << std::setw(3) << (t.tv_nsec / 1000000);
            ret = ss.str();
        }
    }

    return ret;
}

#define _NAME   "HMI-Tester: "

/// Uses qDebug() so things like QWidgets are displayed with useful info
#define QTDEBUG(content)                                                        \
do {                                                                            \
  if (DEBUG_ENABLED) {                                                          \
    qDebug() << content;                                                        \
  }                                                                             \
} while(false)

#define DEBUGc(content)                                                         \
do {                                                                            \
  if (DEBUG_ENABLED) {                                                          \
    std::cout << _NAME << timeNow() << " DEBUGc " << basename(__FILE__)         \
              << ":" << __LINE__ << " - " << __FUNCTION__ << "(): " << content  \
              << std::endl;                                                     \
  }                                                                             \
} while(false)

#define _d(content)                                                             \
do {                                                                            \
  if (DEBUG_ENABLED) {                                                          \
    std::cout << _NAME << timeNow() << " _d " << basename(__FILE__)             \
              << ":" << __LINE__ << " - " << __FUNCTION__ << "(): " << content  \
              << std::endl;                                                     \
  }                                                                             \
} while(false)

///
/// parameterized debug
///

// Debug types enabled are values > 0
// Debug types disabled are values == 0
// Debug types enabled should have different values than D_ERROR, therefore
// the OHT framework can differentiate between errors and other debug outputs

#define D_ERROR 1
#define D_PLAYBACK 2
#define D_RECORDING 2
#define D_BOTH 1
#define D_PRELOAD 2
#define D_EXECUTOR 1
#define D_CONSUMER 1
#define D_GUI 1
#define D_COMM 1

// method
#define LOG_ERR(content)                                                            \
do {                                                                                \
    std::cout << _NAME << timeNow() << " ERROR " << basename(__FILE__)              \
              << ":" << __LINE__ << " - " << __FUNCTION__ << "(): " << content      \
              << std::endl;                                                         \
} while(false)

#define LOG_DBG(content)                                                            \
do {                                                                                \
    std::cout << _NAME << timeNow() << " DEBUG " << basename(__FILE__)              \
              << ":" << __LINE__ << " - " << __FUNCTION__ << "(): " << content      \
              << std::endl;                                                         \
} while(false)

/// Accepts a log type and uses qDebug() so things like QWidgets are displayed with useful info
#define QTDEBUGT(type, content)                                                     \
do {                                                                                \
  if (type == D_ERROR) {                                                            \
    qWarning() << content;                                                          \
  }                                                                                 \
  else if (type && DEBUG_ENABLED) {                                                 \
    qDebug() << content;                                                            \
  }                                                                                 \
} while(false)

#define DEBUG(type, content)                                                        \
do {                                                                                \
  if (type == D_ERROR) {                                                            \
    std::cout << _NAME << timeNow() << " ERROR " << basename(__FILE__)              \
              << ":" << __LINE__ << " - " << __FUNCTION__ << "(): " << content      \
              << std::endl;                                                         \
  }                                                                                 \
  else if (type && DEBUG_ENABLED) {                                                 \
    std::cout << _NAME << timeNow() << " DEBUG " << basename(__FILE__)              \
              << ":" << __LINE__ << " - " << __FUNCTION__ << "(): " << content      \
              << std::endl;                                                         \
  }                                                                                 \
} while(false)

#endif // DEBUG_H
