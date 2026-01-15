/*
 * RedkaConnect -- mouse and keyboard sharing utility
 * Based on InputLeap
 *
 * This package is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * found in the file LICENSE that should have accompanied this file.
 */

#include "SecurityManager.h"

#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QSettings>
#include <QCryptographicHash>
#include <QProcess>
#include <QDateTime>
#include <QRandomGenerator>

SecurityManager::SecurityManager(QObject* parent)
    : QObject(parent)
{
    // Set data path for certificates
    m_dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(m_dataPath);
    
    loadTrustedFingerprints();
}

SecurityManager::~SecurityManager() = default;

bool SecurityManager::initialize(const QString& computerName)
{
    // Try to load existing certificate
    if (loadCertificate()) {
        return true;
    }
    
    // Generate new certificate
    return generateCertificate(computerName);
}

bool SecurityManager::hasCertificate() const
{
    return m_localCert.isValid;
}

SecurityManager::CertificateInfo SecurityManager::localCertificate() const
{
    return m_localCert;
}

QString SecurityManager::displayFingerprint() const
{
    if (m_localCert.shortFingerprint.isEmpty()) {
        return "N/A";
    }
    // Format as XXXX-XXXX for easy reading
    QString fp = m_localCert.shortFingerprint.toUpper();
    if (fp.length() >= 8) {
        return fp.left(4) + "-" + fp.mid(4, 4);
    }
    return fp;
}

bool SecurityManager::isTrusted(const QString& fingerprint) const
{
    return m_trustedFingerprints.contains(fingerprint.toLower());
}

void SecurityManager::trustFingerprint(const QString& fingerprint, const QString& name)
{
    m_trustedFingerprints[fingerprint.toLower()] = name;
    saveTrustedFingerprints();
    emit trustChanged();
}

void SecurityManager::removeTrust(const QString& fingerprint)
{
    m_trustedFingerprints.remove(fingerprint.toLower());
    saveTrustedFingerprints();
    emit trustChanged();
}

QStringList SecurityManager::trustedFingerprints() const
{
    return m_trustedFingerprints.keys();
}

QString SecurityManager::certificatePath() const
{
    return m_dataPath + "/redkaconnect.crt";
}

QString SecurityManager::privateKeyPath() const
{
    return m_dataPath + "/redkaconnect.key";
}

QString SecurityManager::generateVerificationCode(const QString& localFingerprint, 
                                                   const QString& remoteFingerprint)
{
    // Combine fingerprints in a deterministic way
    QString combined;
    if (localFingerprint < remoteFingerprint) {
        combined = localFingerprint + remoteFingerprint;
    } else {
        combined = remoteFingerprint + localFingerprint;
    }
    
    // Hash and take first 6 chars
    QByteArray hash = QCryptographicHash::hash(combined.toUtf8(), QCryptographicHash::Sha256);
    QString hex = hash.toHex().left(6).toUpper();
    
    // Format as XXX-XXX for easy comparison
    return hex.left(3) + "-" + hex.mid(3, 3);
}

bool SecurityManager::regenerateCertificate(const QString& computerName)
{
    // Remove existing certificate
    QFile::remove(certificatePath());
    QFile::remove(privateKeyPath());
    m_localCert = CertificateInfo();
    
    // Generate new one
    return generateCertificate(computerName);
}

bool SecurityManager::generateCertificate(const QString& computerName)
{
    // Use OpenSSL to generate a self-signed certificate
    // This is the same approach Input Leap uses, but automated
    
    QString certPath = certificatePath();
    QString keyPath = privateKeyPath();
    
    // Generate private key and certificate using OpenSSL
    QProcess openssl;
    
    // First, generate the private key
    QStringList keyArgs;
    keyArgs << "genrsa" << "-out" << keyPath << "2048";
    
    openssl.start("openssl", keyArgs);
    if (!openssl.waitForFinished(30000)) {
        emit certificateError("Failed to generate private key: " + openssl.errorString());
        
        // Fallback: try to create a simple certificate without OpenSSL
        return generateFallbackCertificate(computerName);
    }
    
    if (openssl.exitCode() != 0) {
        emit certificateError("OpenSSL key generation failed: " + QString::fromUtf8(openssl.readAllStandardError()));
        return generateFallbackCertificate(computerName);
    }
    
    // Generate the self-signed certificate
    QStringList certArgs;
    certArgs << "req" << "-new" << "-x509" 
             << "-key" << keyPath
             << "-out" << certPath
             << "-days" << "3650"  // Valid for 10 years
             << "-subj" << QString("/CN=%1/O=RedkaConnect").arg(computerName)
             << "-sha256";
    
    openssl.start("openssl", certArgs);
    if (!openssl.waitForFinished(30000)) {
        emit certificateError("Failed to generate certificate: " + openssl.errorString());
        return generateFallbackCertificate(computerName);
    }
    
    if (openssl.exitCode() != 0) {
        emit certificateError("OpenSSL certificate generation failed: " + QString::fromUtf8(openssl.readAllStandardError()));
        return generateFallbackCertificate(computerName);
    }
    
    // Load the generated certificate
    if (loadCertificate()) {
        emit certificateGenerated();
        return true;
    }
    
    return false;
}

bool SecurityManager::generateFallbackCertificate(const QString& computerName)
{
    // If OpenSSL is not available, create a placeholder
    // The actual SSL connection will fail, but at least the app works
    
    QString certPath = certificatePath();
    QString keyPath = privateKeyPath();
    
    // Generate a random "fingerprint" for identification
    QByteArray randomData;
    for (int i = 0; i < 32; ++i) {
        randomData.append(static_cast<char>(QRandomGenerator::global()->bounded(256)));
    }
    
    m_localCert.fingerprint = QCryptographicHash::hash(randomData, QCryptographicHash::Sha256).toHex();
    m_localCert.shortFingerprint = m_localCert.fingerprint.left(8);
    m_localCert.commonName = computerName;
    m_localCert.isValid = false;  // Mark as invalid since it's not a real cert
    
    // Save the fingerprint for consistency
    QSettings settings;
    settings.setValue("security/fingerprint", m_localCert.fingerprint);
    settings.setValue("security/computerName", computerName);
    
    emit certificateError("OpenSSL not found. Encryption disabled - connections will be unencrypted.");
    
    return false;
}

bool SecurityManager::loadCertificate()
{
    QFile certFile(certificatePath());
    QFile keyFile(privateKeyPath());
    
    if (!certFile.exists() || !keyFile.exists()) {
        return false;
    }
    
    if (!certFile.open(QIODevice::ReadOnly)) {
        return false;
    }
    m_localCert.certificate = certFile.readAll();
    certFile.close();
    
    if (!keyFile.open(QIODevice::ReadOnly)) {
        return false;
    }
    m_localCert.privateKey = keyFile.readAll();
    keyFile.close();
    
    // Calculate fingerprint
    m_localCert.fingerprint = calculateFingerprint(m_localCert.certificate);
    m_localCert.shortFingerprint = m_localCert.fingerprint.left(8);
    
    // Extract common name (simplified)
    QString certStr = QString::fromUtf8(m_localCert.certificate);
    int cnStart = certStr.indexOf("CN=");
    if (cnStart >= 0) {
        int cnEnd = certStr.indexOf("/", cnStart);
        if (cnEnd < 0) cnEnd = certStr.indexOf("\n", cnStart);
        if (cnEnd > cnStart) {
            m_localCert.commonName = certStr.mid(cnStart + 3, cnEnd - cnStart - 3);
        }
    }
    
    m_localCert.isValid = true;
    return true;
}

bool SecurityManager::saveCertificate()
{
    // Certificates are already saved by OpenSSL
    return true;
}

void SecurityManager::loadTrustedFingerprints()
{
    QSettings settings;
    int count = settings.beginReadArray("trustedFingerprints");
    for (int i = 0; i < count; ++i) {
        settings.setArrayIndex(i);
        QString fp = settings.value("fingerprint").toString();
        QString name = settings.value("name").toString();
        if (!fp.isEmpty()) {
            m_trustedFingerprints[fp] = name;
        }
    }
    settings.endArray();
}

void SecurityManager::saveTrustedFingerprints()
{
    QSettings settings;
    settings.beginWriteArray("trustedFingerprints");
    int i = 0;
    for (auto it = m_trustedFingerprints.begin(); it != m_trustedFingerprints.end(); ++it, ++i) {
        settings.setArrayIndex(i);
        settings.setValue("fingerprint", it.key());
        settings.setValue("name", it.value());
    }
    settings.endArray();
}

QString SecurityManager::calculateFingerprint(const QByteArray& cert) const
{
    // Calculate SHA-256 fingerprint of the certificate
    QByteArray hash = QCryptographicHash::hash(cert, QCryptographicHash::Sha256);
    return hash.toHex().toLower();
}
