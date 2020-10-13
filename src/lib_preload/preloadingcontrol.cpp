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

#include "preloadingcontrol.h"
#include <cassert>
#include <debug.h>
#include <preloadcontroller.h>

#include <QObject>

static void customLogger(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    const char *file = context.file ? basename(context.file) : "";
    const char *function = context.function ? context.function : "";
    switch (type) {
    case QtDebugMsg:
	std::cout << _NAME << timeNow() << " DEBUG " << file
		  << ":" << context.line << " - " << function << "(): " << localMsg.constData()
		  << std::endl;
        break;
    case QtInfoMsg:
	std::cout << _NAME << timeNow() << " INFO  " << file
		  << ":" << context.line << " - " << function << "(): " << localMsg.constData()
		  << std::endl;
        break;
    case QtWarningMsg:
	std::cout << _NAME << timeNow() << " ERROR " << file
		  << ":" << context.line << " - " << function << "(): " << localMsg.constData()
		  << std::endl;
        break;
    case QtCriticalMsg:
	std::cout << _NAME << timeNow() << " CRITICAL " << file
		  << ":" << context.line << " - " << function << "(): " << localMsg.constData()
		  << std::endl;
        break;
    case QtFatalMsg:
	std::cout << _NAME << timeNow() << " FATAL " << file
		  << ":" << context.line << " - " << function << "(): " << localMsg.constData()
		  << std::endl;
        break;
    }
}

PreloadingControl::PreloadingControl(EventConsumer *ec, EventExecutor *ex) {
    qInstallMessageHandler(customLogger);

    // attach specific consumer
    assert(ec);
    test_thread_ = new QThread;
    _event_consumer = ec;
    // attach specific executor
    assert(ex);
    _event_executor = ex;
}

///
/// preload init method that has to be called
///
void PreloadingControl::initPreload() {
    // create a new Preload Controller
    PreloadController *pc =
            new PreloadController(this, _event_consumer, _event_executor);
    pc->initialize();
    pc->moveToThread(test_thread_);

    //  QObject::connect(test_thread_, SIGNAL(started()), pc, SLOT(process()));
    //  QObject::connect(pc, SIGNAL(finished()), test_thread_, SLOT(quit()));
    //  QObject::connect(pc, SIGNAL(finished()), pc, SLOT(deleteLater()));
    //  QObject::connect(test_thread_, SIGNAL(finished()), test_thread_,
    //                   SLOT(deleteLater()));
    test_thread_->start();

    DEBUG(D_PRELOAD, "(PreloadingControl::initPreload) Preload Controller "
          "instance initiallized.");
}
