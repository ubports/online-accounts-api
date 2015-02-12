namespace OnlineAccounts {

typedef uint AccountId;

class Manager: public QObject
{
    Q_OBJECT

public:
    explicit Manager(const QString &applicationId, QObject *parent = 0);
    ~Manager();

    QList<AccountId> listAccounts(const QString &service = QString());

    void requestAccess(const QString &service,
                       const AuthenticationData &authData);

Q_SIGNALS:
    void accountEnabled(AccountId account);
};

class PendingCall
{
    PendingCallPrivate *d_ptr;
};

class PendingCallWatcher: public QObject, public PendingCall
{
    Q_OBJECT

public:
    PendingCallWatcher(const PendingCall &call);
    ~PendingCallWatcher();

    bool isFinished();
    void waitForFinished();

Q_SIGNALS:
    void finished();
};

class AuthenticationReply
{
public:
    AuthenticationReply(const PendingCall &call);

    Error error() const;
};

class Account: public QObject
{
    Q_OBJECT

public:
    explicit Account(Manager *manager, AccountId id, QObject *parent);
    ~Account();

    QString displayName() const;
    QString serviceId() const;
    AuthenticationMethod authenticationMethod() const;

    QVariant setting(const QString &key) const;

    PendingCall authenticate(const AuthenticationData &authData);

Q_SIGNALS:
    void changed();
    void disabled();
};

enum AuthenticationMethod {
    AuthenticationMethodOAuth1,
    AuthenticationMethodOAuth2,
    AuthenticationMethodPassword,
};

class AuthenticationData
{
    AuthenticationMethod method() const;

    void setInteractive(bool interactive);
    void invalidateCachedReply();

    Error error();

protected:
    AuthenticationData(AuthenticationMethod method);
};

class OAuth2Data: public AuthenticationData
{
    void setClientId(const QByteArray &id);
    void setClientSecret(const QByteArray &secret);
    void setScopes(const QList<QByteArray> &scopes);
};

class OAuth2Reply: public AuthenticationReply
{
    QString accessToken() const;
    int expiresIn() const;
    QList<QByteArray> grantedScopes();
};

class OAuth1Data: public AuthenticationData ...
class OAuth1Reply: public AuthenticationReply ...

class PasswordData: public AuthenticationData ...
class PasswordReply: public AuthenticationReply ...

/* Example usage */

Manager manager("com.example.gallery_gallery");
QList<AccountId> accountIds = manager.listAccounts();
if (accountIds.isEmpty()) return;

Account account(&manager, accountsIds.at(0));
OAuth2Data data;
data.setInteractive(false);
data.setClientId("XXX");
data.setClientSecret("XXX");
PendingCallWatcher call = account.authenticate(data);
call.waitForFinished();
OAuth2Reply reply(call);
if (reply.error().isValid()) {
    qWarning() << "Got error" << error().text();
    return;
}

QByteArray token = reply.accessToken();

