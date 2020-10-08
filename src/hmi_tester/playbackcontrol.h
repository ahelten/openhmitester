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
#ifndef PLAYBACKCONTROL_H
#define PLAYBACKCONTROL_H

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <datamodel.h>
#include <datamodelmanager.h>
#include <executionobserver.h>
#include <executionthread.h>
#include <memory>

class PlaybackControl {

public:
  PlaybackControl(Comm *, PlaybackObserver *);
  ~PlaybackControl();

  /// execution process control
  bool runTestCase(DataModel::TestCase *, float speed);
  bool pauseExecution();
  bool resumeExecution();
  bool stopExecution();

  // some notification signal handlers
  void applicationFinished();
  void handleEventExecutedOnPreloadModule();

private:
  // communication
  Comm *comm_;

  PlaybackObserver *observer_;

  // Execution thread
  std::unique_ptr<ExecutionThread> executionThread_;
  boost::thread _internal_thread;
};

#endif // PLAYBACKCONTROL_H
