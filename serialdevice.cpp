#include "serialdevice.h"
#include <QQmlEngine>
#include <QtDebug>
#include <QTimer>
#ifdef Q_OS_ANDROID
#include <QtAndroidExtras>
#endif

QList<SerialDevice *> SerialDevice::m_classes;

#ifdef Q_OS_ANDROID
void SerialDevice::serialDataReady(JNIEnv *, jobject , jstring data)
{
    QString dataString = static_cast<QAndroidJniObject>(data).toString();
    for( auto clas: m_classes )
    {
        emit clas->serialDataReadyToProcessSignal( dataString );
    }
}
#endif

int SerialDevice::rowCount(const QModelIndex &) const
{
    return m_dataList.size();
}

bool SerialDevice::setData(const QModelIndex &index, const QVariant &/*value*/, int /*role*/)
{
    if ( !index.isValid() )
        return false;
    return false;
}

QVariant SerialDevice::data(const QModelIndex &index, int role) const
{
    if ( !index.isValid() )
        return QVariant();

    int itemIndex = index.row();

    QString item = m_dataList.at( itemIndex );

    if( role == IndexRole )
        return itemIndex;
    else if( role == TextRole )
        return item;
    return QVariant();
}

QHash<int, QByteArray> SerialDevice::roleNames() const
{
    return m_roles;
}

SerialDevice::SerialDevice(QObject *parent) : QAbstractListModel( parent )
{
    m_classes.push_back( this );

    connect( this, &SerialDevice::serialDataReadyToProcessSignal, this, &SerialDevice::serialDataReadyToProcess, Qt::QueuedConnection );
//    m_dataList.push_back("#############");
#ifdef Q_OS_WIN
    connect( &m_serialPort, &QSerialPort::readyRead, this, &SerialDevice::readyRead );
    auto avap = QSerialPortInfo::availablePorts();
    for( auto pi : avap )
    {
        qDebug()<<"#####" << pi.portName() << pi.description();
        if( pi.description().toLower().contains("arduino") )
            m_serialPort.setPort( pi );
    }


    //    m_serialPort.setPort( avap.at(avap.count()- 1) );

    m_serialPort.setBaudRate( QSerialPort::Baud115200 );
    m_serialPort.setDataBits( QSerialPort::Data8 );
    m_serialPort.setParity( QSerialPort::NoParity );
    m_serialPort.setStopBits( QSerialPort::OneStop );
    if( !m_serialPort.open(QSerialPort::ReadWrite) )
    {
        qDebug()<<"#####Device not found!!!";
        exit(0);
    }
#endif

#ifdef Q_OS_ANDROID
    if( !QtAndroid::androidActivity().callMethod<jboolean>("openDevice", "()Z") )
    {
        qDebug()<<"#####Device not found!!!";
        exit(0);
    }
#endif


    m_roles[IndexRole] = "indexData";
    m_roles[TextRole] = "textData";
}

SerialDevice::~SerialDevice()
{
    m_classes.removeOne( this );
    //    clear();
}

void SerialDevice::clear()
{
    beginRemoveRows( QModelIndex(), 0, m_dataList.size() - 1 );
    m_dataList.clear();
    endRemoveRows();
}

void SerialDevice::registerTypes()
{
    qmlRegisterType<SerialDevice>( "org.tdevelopers.serialdebug", 1, 0, "SerialDevice" );
}

QString SerialDevice::logText() const
{
    return m_logText;
}

void SerialDevice::sendString(QString string)
{
    QTimer::singleShot(1, [this, string](){
#ifdef Q_OS_WIN
        qDebug()<<string;
        m_serialPort.write((string + "\n").toUtf8());
#else
        QAndroidJniObject dataString = QAndroidJniObject::fromString(string + "\n");
        QtAndroid::androidActivity().callMethod<jboolean>(
                    "sendData",
                    "(Ljava/lang/String;)Z",
                    dataString.object<jstring>());
#endif
    });
}

void SerialDevice::setLogText(QString logText)
{
    if (m_logText == logText)
        return;

    m_logText = logText;
    emit logTextChanged(m_logText);
}

void SerialDevice::serialDataReadyToProcess(QString data)
{
    m_serialData += data;
    processLines();
}

void SerialDevice::readyRead()
{
    emit serialDataReadyToProcessSignal( m_serialPort.readAll() );
}

void SerialDevice::processLines()
{
    int firstIndex = m_serialData.indexOf("\n");
    if( firstIndex <= -1 )
        return;

    QString dataLine = m_serialData.left(firstIndex);
    m_serialData.remove( 0, firstIndex + QString("\n").length() );
//    qDebug()<<"LINEEEEEE" << m_serialData;
    QString code = codeFromString( &dataLine );
    //        qDebug()<<"MMMMM" << code;
    if( code.isEmpty() )
    {
        setLogText( m_logText + dataLine + "\n" );
//        if( m_dataList.isEmpty() )
//            m_dataList.insert( 0, dataLine );
//        else
//            m_dataList[0] = dataLine;
//        emit dataChanged(createIndex(0, 0), createIndex( 0, 0 ) );
    }else
    {
        bool found = false;
        for( int i = 0; i < m_dataList.size(); ++i )
        {
            if( codeFromString( &m_dataList.at(i) ) == code  )
            {
                m_dataList[i] = dataLine;
                emit dataChanged(createIndex(i, 0), createIndex( i, 0 ) );
                found = true;
                break;
            }
        }
        if( !found )
        {
            beginInsertRows( QModelIndex(), m_dataList.size(), m_dataList.size() );
            m_dataList.append( dataLine );
            endInsertRows();
        }
    }

    processLines();
}

QString SerialDevice::codeFromString(const QStringRef &str)
{
    auto start = str.indexOf( "ID: 0x" );
    if( start < 0 )
        return "";
    return str.mid( start, str.indexOf("  ") - start ).toString();
}
