// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "Client.h"
#include "TundraLogicModule.h"

#include "KristalliProtocolModule.h"
#include "CoreStringUtils.h"
#include "SyncManager.h"
#include "TundraMessages.h"
#include "PhysicsModule.h"

#include "SceneAPI.h"
#include "Scene.h"

#include "MsgLogin.h"
#include "MsgLoginReply.h"
#include "MsgClientJoined.h"
#include "MsgClientLeft.h"

#include "MemoryLeakCheck.h"

#include <QDomElement>

using namespace kNet;

namespace TundraLogic
{

Client::Client(TundraLogicModule* owner) :
    owner_(owner),
    framework_(owner->GetFramework()),
    loginstate_(NotConnected),
    reconnect_(false),
    client_id_(0)
{
}

Client::~Client()
{
}

void Client::Update(f64 frametime)
{
    // If we aren't a server, check pending login
    if (!owner_->IsServer())
        CheckLogin();
}

void Client::Login(const QUrl& loginUrl)
{
    // We support tundra, http and https scheme login urls
    QString urlScheme = loginUrl.scheme().toLower();
    if (urlScheme.isEmpty())
        return;
    if (urlScheme != "tundra" && 
        urlScheme != "http" && 
        urlScheme != "https")
        return;

    // Parse values from url
    QString username = loginUrl.queryItemValue("username").trimmed();
    QString password = loginUrl.queryItemValue("password");
    QString avatarurl = loginUrl.queryItemValue("avatarurl").trimmed();
    QString protocol = loginUrl.queryItemValue("protocol").trimmed().toLower();
    QString address = loginUrl.host();
    int port = loginUrl.port();

    // Validation: Username and address is the minimal set that with we can login with
    if (username.isEmpty() || address.isEmpty())
        return;
    if (username.count(" ") > 0)
        username = username.replace(" ", "-");
    if (port < 0)
        port = 2345;

    // Set custom login parameters and login
    if (!avatarurl.isEmpty())
        SetLoginProperty("avatarurl", avatarurl);

    Login(address, port, username, password, protocol);
}

void Client::Login(const QString& address, unsigned short port, const QString& username, const QString& password, const QString &protocol)
{
    // Make sure to logout, our scene manager gets confused when you login again
    // when already connected to another or same server.
    if (IsConnected())
        DoLogout();

    SetLoginProperty("address", address);
    SetLoginProperty("port", QString::number(port));
    SetLoginProperty("username", username);
    SetLoginProperty("password", password);
    SetLoginProperty("protocol", protocol);
    
    kNet::SocketTransportLayer transportLayer = kNet::InvalidTransportLayer;
    if (protocol.toLower() == "tcp")
        transportLayer = kNet::SocketOverTCP;
    else if (protocol.toLower() == "udp")
        transportLayer = kNet::SocketOverUDP;
    Login(address, port, transportLayer);
}

void Client::Login(const QString& address, unsigned short port, kNet::SocketTransportLayer protocol)
{
    if (owner_->IsServer())
    {
        ::LogError("Already running a server, cannot login to a world as a client");
        return;
    }

    reconnect_ = false;
    if (protocol == kNet::InvalidTransportLayer)
    {
        ::LogInfo("Client::Login: No protocol specified, using the default value.");
        protocol = owner_->GetKristalliModule()->defaultTransport;

    SetLoginProperty("address", address);
    QString p = "";
    if (protocol == kNet::SocketOverTCP)
        p = "tcp";
    else if (protocol == kNet::SocketOverUDP)
        p = "udp";

    SetLoginProperty("protocol", p);
    SetLoginProperty("port", QString::number(port));

    }

    KristalliProtocol::KristalliProtocolModule *kristalli = framework_->GetModule<KristalliProtocol::KristalliProtocolModule>();
    connect(kristalli, SIGNAL(NetworkMessageReceived(kNet::MessageConnection *, kNet::message_id_t, const char *, size_t)), 
        this, SLOT(HandleKristalliMessage(kNet::MessageConnection*, kNet::message_id_t, const char*, size_t)), Qt::UniqueConnection);

    connect(kristalli, SIGNAL(ConnectionAttemptFailed()), this, SLOT(OnConnectionAttemptFailed()), Qt::UniqueConnection);

    owner_->GetKristalliModule()->Connect(address.toStdString().c_str(), port, protocol);
    loginstate_ = ConnectionPending;
    client_id_ = 0;
}

void Client::Logout()
{
    QTimer::singleShot(1, this, SLOT(DelayedLogout()));
}

void Client::DelayedLogout()
{
    DoLogout(false);
}

void Client::DoLogout(bool fail)
{
    if (loginstate_ != NotConnected)
    {
        if (GetConnection())
        {
            owner_->GetKristalliModule()->Disconnect();
            ::LogInfo("Disconnected");
        }
        
        loginstate_ = NotConnected;
        client_id_ = 0;
        
        framework_->Scene()->RemoveScene("TundraClient");
        
        emit Disconnected();
    }
    
    if (fail)
    {
        emit LoginFailed();
    }
    else // An user deliberately disconnected from the world, and not due to a connection error.
    {
        // Clear all the login properties we used for this session, so that the next login session will start from an
        // empty set of login properties (just-in-case).
        properties.clear();
    }

    KristalliProtocol::KristalliProtocolModule *kristalli = framework_->GetModule<KristalliProtocol::KristalliProtocolModule>();
    disconnect(kristalli, SIGNAL(NetworkMessageReceived(kNet::MessageConnection *, kNet::message_id_t, const char *, size_t)), 
        this, SLOT(HandleKristalliMessage(kNet::MessageConnection*, kNet::message_id_t, const char*, size_t)));

    disconnect(kristalli, SIGNAL(ConnectionAttemptFailed()), this, SLOT(OnConnectionAttemptFailed()));

    ::LogInfo("Client logged out.");
}

bool Client::IsConnected() const
{
    return loginstate_ == LoggedIn;
}

void Client::SetLoginProperty(QString key, QString value)
{
    ::LogInfo(key.toStdString() + ":" + value.toStdString());
    key = key.trimmed();
    value = value.trimmed();
    if (value.isEmpty())
        properties.erase(key);
    properties[key] = value;
}

QString Client::GetLoginProperty(QString key)
{
    key = key.trimmed();
    if (properties.count(key) > 0)
        return properties[key];
    else
        return "";
}

QString Client::LoginPropertiesAsXml() const
{
    QDomDocument xml;
    QDomElement rootElem = xml.createElement("login");
    for(std::map<QString, QString>::const_iterator iter = properties.begin(); iter != properties.end(); ++iter)
    {
        QDomElement elem = xml.createElement(iter->first.toStdString().c_str());
        elem.setAttribute("value", iter->second.toStdString().c_str());
        rootElem.appendChild(elem);
    }
    xml.appendChild(rootElem);
    return xml.toString();
}

void Client::CheckLogin()
{
    kNet::MessageConnection* connection = GetConnection();
    
    switch (loginstate_)
    {
    case ConnectionPending:
        if ((connection) && (connection->GetConnectionState() == kNet::ConnectionOK))
        {
            loginstate_ = ConnectionEstablished;
            MsgLogin msg;
            emit AboutToConnect(); // This signal is used as a 'function call'. Any interested party can fill in
            // new content to the login properties of the client object, which will then be sent out on the line below.
            msg.loginData = StringToBuffer(LoginPropertiesAsXml().toStdString());
            connection->Send(msg);
        }
        break;
    
    case LoggedIn:
        // If we have logged in, but connection dropped, prepare to resend login
        if ((!connection) || (connection->GetConnectionState() != kNet::ConnectionOK))
        {
            loginstate_ = ConnectionPending;
        }
        break;
    }
}

kNet::MessageConnection* Client::GetConnection()
{
    return owner_->GetKristalliModule()->GetMessageConnection();
}

void Client::OnConnectionAttemptFailed()
{
    DoLogout(true);
}

void Client::HandleKristalliMessage(MessageConnection* source, message_id_t id, const char* data, size_t numBytes)
{
    if (source != GetConnection())
    {
        ::LogWarning("Client: dropping message " + ToString(id) + " from unknown source");
        return;
    }
    
    switch (id)
    {
    case cLoginReplyMessage:
        {
            MsgLoginReply msg(data, numBytes);
            HandleLoginReply(source, msg);
        }
        break;
    case cClientJoinedMessage:
        {
            MsgClientJoined msg(data, numBytes);
            HandleClientJoined(source, msg);
        }
        break;
    case cClientLeftMessage:
        {
            MsgClientLeft msg(data, numBytes);
            HandleClientLeft(source, msg);
        }
        break;
    }
    emit NetworkMessageReceived(id, data, numBytes);
}

void Client::HandleLoginReply(MessageConnection* source, const MsgLoginReply& msg)
{
    if (msg.success)
    {
        loginstate_ = LoggedIn;
        client_id_ = msg.userID;
        ::LogInfo("Logged in successfully");
        
        // Note: create scene & send info of login success only on first connection, not on reconnect
        if (!reconnect_)
        {
            // Create a non-authoritative scene for the client
            ScenePtr scene = framework_->Scene()->CreateScene("TundraClient", true, false);

            framework_->Scene()->SetDefaultScene(scene);
            owner_->GetSyncManager()->RegisterToScene(scene);
                        
            UserConnectedResponseData responseData;
            if (msg.loginReplyData.size() > 0)
                responseData.responseData.setContent(QByteArray((const char *)&msg.loginReplyData[0], (int)msg.loginReplyData.size()));

            emit Connected(&responseData);
        }
        else
        {
            // If we are reconnecting, empty the scene, as the server will send everything again anyway
            // Note: when we move to unordered communication, we must guarantee that the server does not send
            // any scene data before the login reply

            ScenePtr scene = framework_->Scene()->GetScene("TundraClient");
            if (scene)
                scene->RemoveAllEntities(true, AttributeChange::LocalOnly);
        }
        reconnect_ = true;
    }
    else
    {
        DoLogout(true);
    }
}

void Client::HandleClientJoined(MessageConnection* source, const MsgClientJoined& msg)
{
}

void Client::HandleClientLeft(MessageConnection* source, const MsgClientLeft& msg)
{
}

}

