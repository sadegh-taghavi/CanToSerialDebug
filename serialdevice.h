#ifndef SERIALDEVICE_H
#define SERIALDEVICE_H

#include <QObject>
#include <QQuickItem>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QAbstractListModel>
#ifdef Q_OS_ANDROID
#include <QtAndroidExtras/QAndroidJniEnvironment>
#endif

class SerialDevice : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(QString logText READ logText WRITE setLogText NOTIFY logTextChanged)
public:
#ifdef Q_OS_ANDROID
    static void serialDataReady(JNIEnv *, jobject, jstring data);
#endif

    enum Roles {
        IndexRole = Qt::UserRole + 1,
        TextRole,
    };

    Q_ENUMS( Roles )

    SerialDevice(QObject *parent = nullptr);
    virtual ~SerialDevice();
    int rowCount(const QModelIndex & /* parent */) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray> roleNames() const;

    Q_INVOKABLE void clear();
    static void registerTypes();
    QString logText() const;

signals:
    void itemsCountChanged( int count );
    void serialDataReadyToProcessSignal( QString data );
    void logTextChanged(QString logText);

public slots:
    void sendString( QString string );
    void setLogText(QString logText);

private slots:
    void serialDataReadyToProcess( QString data );
    void readyRead();

private:
    void processLines();
    QString m_serialData;
    QList<QString> m_dataList;
    QHash<int, QByteArray> m_roles;
    QSerialPort m_serialPort;
    QString codeFromString(const QStringRef &str );
    static QList<SerialDevice *> m_classes;
    QString m_logText;
};


#endif // SERIALDEVICE_H
