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

#include "messageclientserver.h"

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <cassert>
#include <debug.h>
#include <iostream>
#include <ohtbaseconfig.h>
#include <sstream>

/// ///////////////////////////////////////////
/// /// CLIENT/SERVER /////////////////////////
/// ///////////////////////////////////////////

MessageClientServer::MessageClientServer(QObject *parent, uint port,
                                         bool isServer)
    : QTcpServer(parent) {
    setParent(parent);
    _port = port;

    ///
    /// if it is created as a server...

    if (isServer) {
        // starting listening process
        if (!this->listen(QHostAddress(SERVER_IP), _port)) {
            DEBUG(D_ERROR, "Unable to start server " << SERVER_IP << "::" << _port
                  << " ERROR = " << this->errorString().toStdString());
            close();
            return;
        }

        // connect(this, SIGNAL(newConnection()), /**/ this,
        // SIGNAL(newClientConnected()));

        DEBUG(D_COMM, "Server listening on address " << SERVER_IP << "::" << _port);
    }

    ///
    /// if it is created as a client...

    else {
        // creating a new socket and connecting to its signals
        currentSocket_.reset(new QTcpSocket(this));
        connect(currentSocket_.get(), SIGNAL(error(QAbstractSocket::SocketError)),
                this, SLOT(displayError(QAbstractSocket::SocketError)));
        connect(currentSocket_.get(), SIGNAL(readyRead()), this, SLOT(readMessage()));

        // connecting to the server
        DEBUG(D_COMM, "Connecting to address " << SERVER_IP << " and port " << _port);
        currentSocket_->connectToHost(QHostAddress(SERVER_IP), _port);
        if (currentSocket_->waitForConnected(2000)) {
            DEBUG(D_COMM, "Client successfully connected");
        }
        else {
            DEBUG(D_COMM, "Error while connecting client. Code = " +
                  QString::number(currentSocket_->error()).toStdString());
        }
    }
}

MessageClientServer::~MessageClientServer() {}

///
/// (Only for Server) Handles an incoming connection
///
#if QT_VERSION >= 0x050000
void MessageClientServer::incomingConnection(qintptr socketDescriptor)
#else
void MessageClientServer::incomingConnection(int socketDescriptor)
#endif
{
    DEBUG(D_COMM, "Socket = " + QString::number(socketDescriptor).toStdString());

    // FIXME: It only accepts one connection and erases
    // the previous ones. Have to fix this.

    // creating a new socket for the incoming connection
    ClientSocket *cs_ = new ClientSocket(socketDescriptor, this);
    connect(cs_, SIGNAL(readyRead()), this, SLOT(readMessage()));
    connect(cs_, SIGNAL(error(QAbstractSocket::SocketError)), this,
            SLOT(displayError(QAbstractSocket::SocketError)));
    connect(cs_, SIGNAL(error(const QString &)), parent(),
            SLOT(handleError(const QString &)));

    // set the socket as the current (closes the open server socket)
    currentSocket_.reset(cs_);

    // clear the buffer
    buffer_.clear();

    // send the signal
    emit newClientConnected();

    DEBUG(D_COMM, "New incomming client");
}

//
// sending data to the client
//
void MessageClientServer::writeMessage(const QString &s) {
    // DEBUG(D_COMM,"(MessageClientServer::writeMessage) Data to send " <<
    // s.toStdString());  writing a new message into the socket
    assert(currentSocket_.get());
    if (!_writeSocket(currentSocket_.get(), s)) {
        DEBUG(D_ERROR, "Error while writting sockect");
        emit error("MessageClientServer::writeMessage) Error writing into the socket.");
    }

    DEBUG(D_COMM, "Message written into the socket");
}

//
// reading data from the client
//
void MessageClientServer::readMessage() {
    DEBUG(D_COMM, "enter");
    // if we cannot read...
    assert(currentSocket_.get());
    if (!_readSocket(currentSocket_.get(), &buffer_)) {
        DEBUG(D_ERROR, "Error reading from the socket");
        emit error("(MessageClientServer::readMessage) Error reading from the socket.");
    }
    // if we can, we store the data into the buffer and emit complete messages
    else {
        // for each message
        QString message = buffer_.getOneString();
        while (message != "") {
            // DEBUG(D_COMM, "(MessageClientServer::readMessage) One message
            // received:");  DEBUG(D_COMM, "(MessageClientServer::readMessage) " <<
            // message.toStdString());  emitting the received message signal
            emit receivedMessage(message);
            DEBUG(D_COMM, "Message emitted");
            // reading the next message (if it exists)
            message = buffer_.getOneString();
        }
    }
    DEBUG(D_COMM, "Exit");
}

///
/// Displays error description
///
void MessageClientServer::displayError(
        QAbstractSocket::SocketError socketError) {
    QString errorString =
            "(Client::displayError) " + QString::number(socketError) + ": ";

    switch (socketError) {

        case QAbstractSocket::RemoteHostClosedError:
            errorString += "Remote host closed connection.";
            // here it also handles the client disconnection
            handleClientDisconnected();
            break;

        case QAbstractSocket::HostNotFoundError:
            errorString +=
                    "The host was not found. Please check the host name and port settings.";
            break;

        case QAbstractSocket::ConnectionRefusedError:
            errorString += "The connection was refused by the peer. Make sure the "
                           "fortune server is running, and check that the host name "
                           "and port settings are correct.";
            break;

        default:
            errorString += "Unknown error.";
    }

    emit error(errorString);
}

///
/// handles the client disconnection
///

void MessageClientServer::handleClientDisconnected() {
    // clear the buffer
    buffer_.clear();

    // emit the signal
    emit clientDisconnected();
}

/// ///////////////////////////////////////////
/// /// SOCKET READ AND WRITE METHODS /////////
/// ///////////////////////////////////////////

///
/// read from socket
///
bool MessageClientServer::_readSocket(QTcpSocket *s,
                                      QCircularByteArray_ *buffer) {
    DEBUG(D_COMM, "enter");
    assert(buffer);
    assert(s);
    // if it is connected and bytes available...
    if (s->state() == QAbstractSocket::ConnectedState) {
        if (s->bytesAvailable() > 0) {
            // reading and storing into the buffer
            QByteArray data = s->readAll();
            assert(buffer->addData(data));

            DEBUG(D_COMM, "adding data to the buffer");
        }
        // return OK
        return true;
    } else {
        DEBUG(D_COMM, "Error while reading from the socket");
        // return KO
        return false;
    }
}

///
/// write in socket
///
bool MessageClientServer::_writeSocket(QTcpSocket *s, const QString &message) {
    DEBUG(D_COMM, "enter");
    assert(message != ""); // FIXME: remove this assert
    // if it is connected...
    if (s && s->state() == QAbstractSocket::ConnectedState) {
        // writting the message
        // s->write ( reinterpret_cast<const char*>(message.data()),
        //           message.size() + 1 );
        std::string msg = message.toStdString();
        s->write(msg.c_str(), msg.size() + 1);

        DEBUG(D_COMM, "Written");
        s->flush();
        // return OK
        return true;
    } else {
        DEBUG(D_ERROR, "Error while writing into the socket. Socket NULL or not connected");
        // return KO
        return false;
    }
}

///
/// (Only for Server) Client socket for incoming connections
///
ClientSocket::ClientSocket(int sock, QObject *parent) : QTcpSocket(parent) {
    // trying to connect
    if (!setSocketDescriptor(sock, QAbstractSocket::ConnectedState, QIODevice::ReadWrite)) {
        emit error("(ClientSocket::ClientSocket) Error connecting to the socket descriptor.");
        return;
    }
}

ClientSocket::~ClientSocket() {}
