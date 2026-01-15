/*
 * RedkaConnect -- mouse and keyboard sharing utility
 * Based on InputLeap
 *
 * This package is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * found in the file LICENSE that should have accompanied this file.
 */

#pragma once

#include <QObject>
#include <QString>
#include <QByteArray>

/**
 * @brief Manages SSL/TLS security for RedkaConnect
 * 
 * Simplifies the SSL certificate experience:
 * - Auto-generates certificates on first run
 * - Provides simple fingerprint verification
 * - Handles certificate storage and loading
 * 
 * Security Model:
 * 1. Each computer generates its own certificate on first run
 * 2. When connecting, both sides show a verification code
 * 3. User confirms the codes match (like Bluetooth pairing)
 * 4. Trusted fingerprints are saved for future connections
 */
class SecurityManager : public QObject
{
    Q_OBJECT

public:
    struct CertificateInfo {
        QString fingerprint;        // SHA-256 fingerprint
        QString shortFingerprint;   // First 8 chars for display
        QString commonName;         // Computer name
        QByteArray certificate;     // PEM-encoded certificate
        QByteArray privateKey;      // PEM-encoded private key
        bool isValid;
    };

    explicit SecurityManager(QObject* parent = nullptr);
    ~SecurityManager() override;

    /**
     * @brief Initialize security - generates certificate if needed
     * @param computerName Name to embed in certificate
     * @return true if initialization succeeded
     */
    bool initialize(const QString& computerName);

    /**
     * @brief Check if certificates exist
     */
    bool hasCertificate() const;

    /**
     * @brief Get our certificate info
     */
    CertificateInfo localCertificate() const;

    /**
     * @brief Get short fingerprint for display (e.g., "A1B2-C3D4")
     */
    QString displayFingerprint() const;

    /**
     * @brief Check if a remote fingerprint is trusted
     */
    bool isTrusted(const QString& fingerprint) const;

    /**
     * @brief Trust a remote fingerprint
     */
    void trustFingerprint(const QString& fingerprint, const QString& name);

    /**
     * @brief Remove trust for a fingerprint
     */
    void removeTrust(const QString& fingerprint);

    /**
     * @brief Get list of trusted fingerprints
     */
    QStringList trustedFingerprints() const;

    /**
     * @brief Get path to certificate file
     */
    QString certificatePath() const;

    /**
     * @brief Get path to private key file
     */
    QString privateKeyPath() const;

    /**
     * @brief Generate a verification code from two fingerprints
     * 
     * Both sides generate this from their own + remote fingerprint.
     * If both sides show the same code, the connection is secure.
     */
    static QString generateVerificationCode(const QString& localFingerprint, 
                                            const QString& remoteFingerprint);

    /**
     * @brief Regenerate certificates (for key rotation)
     */
    bool regenerateCertificate(const QString& computerName);

signals:
    void certificateGenerated();
    void certificateError(const QString& error);
    void trustChanged();

private:
    bool generateCertificate(const QString& computerName);
    bool generateFallbackCertificate(const QString& computerName);
    bool loadCertificate();
    bool saveCertificate();
    void loadTrustedFingerprints();
    void saveTrustedFingerprints();
    QString calculateFingerprint(const QByteArray& cert) const;

    CertificateInfo m_localCert;
    QMap<QString, QString> m_trustedFingerprints;  // fingerprint -> name
    QString m_dataPath;
};
