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

#include "comm.h"

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <cassert>
#include <debug.h>
#include <iostream>
#include <ohtbaseconfig.h>
#include <sstream>

/// ///////////////////////////////////////////
/// /// ComMANAGER ////////////////////////////
/// ///////////////////////////////////////////

Comm::Comm(int port, bool isServer) {
    _port = port;
    _isServer = isServer;
}

bool Comm::resetAndStart() {
    // creating the client/server
    mcs_.reset(new MessageClientServer(this, _port, _isServer));

    // connecting to its signals
    connect(this, SIGNAL(sendMessage(const QString &)), mcs_.get(),
            SLOT(writeMessage(const QString &)));
    connect(mcs_.get(), SIGNAL(receivedMessage(const QString &)), this,
            SLOT(handleReceivedMessage(const QString &)));
    connect(mcs_.get(), SIGNAL(newClientConnected()), this,
            SLOT(handleNewClientConnected()));
    connect(mcs_.get(), SIGNAL(clientDisconnected()), this,
            SLOT(handleClientDisconnected()));

    // connecting to error management
    connect(mcs_.get(), SIGNAL(error(const QString &)), this,
            SLOT(handleError(const QString &)));

    // if it is a server...
    if (_isServer) {
        DEBUG(D_COMM, "(Comm::Comm) Created new message server on port " << _port);
    }
    // if it is a client...
    else {
        DEBUG(D_COMM, "(Comm::Comm) Created new client on port " << _port);
    }

    clientConnected_ = false;
    testItemQueue_.clear();

    DEBUG(D_COMM, "(Comm::Comm) COMM STARTED");

    return true;
}

bool Comm::stop() {
    mcs_.reset(0);
    clientConnected_ = false;
    testItemQueue_.clear();

    DEBUG(D_COMM, "(Comm::Comm) COMM STOPED");
    return true;
}

///
/// send message handler
///
void Comm::handleSendMessage(const QString &s) { emit sendMessage(s); }

void Comm::handleSendMessage(const std::string &s) {
    emit sendMessage(QString(s.c_str()));
}

///
/// send message handler
///
void Comm::handleSendTestItem(const DataModel::TestItem &ti) {
    DEBUG(D_COMM, "(Comm::handleSendTestItem)");

    // If the client is not connected, store the event
    if (_isServer && !clientConnected_) {
        testItemQueue_.push_back(ti);
        DEBUG(D_COMM, "(Comm::handleSendTestItem) Item in the queue.");
    } else {
        // This step wouldn't be neccessary when we use correct sockets as streams
        std::ostringstream oss;
        // Create the test archive using a string as a buffer
        boost::archive::text_oarchive oa(oss);
        // write class instance to archive
        oa << ti;
        DEBUG(D_COMM, "(Comm::handleSendTestItem) Item serialized.");

        emit sendMessage(QString(oss.str().c_str()));

        DEBUG(D_COMM, "(Comm::handleSendTestItem) Item sent.");
    }
}

///
/// new received message handler
///
void Comm::handleReceivedMessage(const QString &s) {
    qDebug() << __PRETTY_FUNCTION__;
    std::istringstream iss(s.toStdString());
    // Create the test archive using a string as a buffer
    boost::archive::text_iarchive ia(iss);
    // write class instance to archive
    DataModel::TestItem *ti = new DataModel::TestItem();
    ia >> *ti;

    DEBUG(D_COMM,
          "(Comm::handleReceivedMessage) Emiting new received  TestItem.");
    emit receivedTestItem(ti);

    emit OurReceivedMessage(s);
}

///
/// handles the new client connection
///
void Comm::handleNewClientConnected() {
    DEBUG(D_COMM, "(Comm::handleNewClientConnected)");
    assert(clientConnected_ == false); // FIXME remove this assert
    clientConnected_ = true;

    // checks if there are stored items to be sent
    if (!testItemQueue_.size())
        return;
    do {
        handleSendTestItem(testItemQueue_.front());
        testItemQueue_.pop_front();
        DEBUG(D_COMM, "(Comm::handleNewClientConnected) One stored TestItem sent.");
    } while (testItemQueue_.size());

    DEBUG(D_COMM, "(Comm::handleNewClientConnected) Stored TestItems sent.");
}

void Comm::handleClientDisconnected() {
    DEBUG(D_COMM, "(Comm::handleClientDisconnected)");
    assert(clientConnected_ == true); // FIXME remove this assert

    // reset comm state for the next client
    clientConnected_ = false;
    testItemQueue_.clear();
}

///
/// error handling
///
void Comm::handleError(const QString &s) {
    DEBUG(D_COMM, "(Comm::handleError) ERROR: " + s.toStdString() + ".");
    emit error(s.toStdString());
}

///
/// accesor
///
MessageClientServer *Comm::messageServer() { return mcs_.get(); }

MessageClientServer *Comm::messageClient() { return mcs_.get(); }
