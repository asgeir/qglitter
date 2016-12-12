#include "Crypto/Crypto.h"

#include <QIODevice>
#include <QString>
#include <QDebug>

#include <Windows.h>
#include <WinCrypt.h>

void QGlitter::cryptoInit()
{
}

static QString s_qglitterErrorMessage = "";

const QString &QGlitter::errorMessage()
{
	return s_qglitterErrorMessage;
}

static void setErrMsgFromGetLastError(DWORD dw)
{
    LPVOID msgBuf;
    (void)FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,
                   NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&msgBuf, 0, NULL);
    s_qglitterErrorMessage = QString("%1: %2").arg(dw).arg(QString((char*)msgBuf).trimmed());
    (void)LocalFree(msgBuf);
    qDebug() << "ERROR:" << s_qglitterErrorMessage;
}

bool QGlitter::dsaKeygen(int, const QString &)
{
    s_qglitterErrorMessage = "Not implemented";
    return false;
}

bool QGlitter::dsaVerify(QIODevice &sourceData, const QByteArray &signature, const QByteArray &publicKey)
{
    // NOTE: this currently only verifies RSA signatures.
    // Sparkle for OS X currently (8/14/2013) only verifies DSA signatures,
    // but its script to generate keys uses 2048 dsa keys, which MS Crypto
    // doesn't support - it only supports up to 1024. So we've changed QGlitter
    // to support RSA keys.

    bool verified = false;

    // Convert the public key from base64 to binary
    unsigned char derPubKey[2048];
    DWORD derPubKeyLen = sizeof(derPubKey);
    if (!CryptStringToBinaryA(publicKey.constData(), publicKey.length(), CRYPT_STRING_BASE64_ANY, derPubKey, &derPubKeyLen, NULL, NULL)) {
        DWORD dw = GetLastError();
        qDebug() << "CryptStringToBinary public key" << dw;
        setErrMsgFromGetLastError(dw);
        return false;
    }

    // Make a CERT_PUBLIC_KEY_INFO object from the binary data
    CERT_PUBLIC_KEY_INFO *publicKeyInfo = NULL;
    DWORD publicKeyInfoLen = 0;
    if (!CryptDecodeObjectEx(X509_ASN_ENCODING|PKCS_7_ASN_ENCODING, X509_PUBLIC_KEY_INFO, derPubKey, derPubKeyLen,
                            CRYPT_DECODE_ALLOC_FLAG, NULL, &publicKeyInfo, &publicKeyInfoLen)) {
        DWORD dw = GetLastError();
        qDebug() << "CryptStringToBinaryA" << dw;
        setErrMsgFromGetLastError(dw);
        return false;
    }

    // Create the crypt context.
    HCRYPTPROV hCryptProv;
    if (!CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT|CRYPT_SILENT)) {
        DWORD dw = GetLastError();
        qDebug() << "CryptAcquireContext" << dw;
        setErrMsgFromGetLastError(dw);
        return false;
    }

    // Import the public key
    HCRYPTKEY hPubKey;
    if (!CryptImportPublicKeyInfo(hCryptProv, X509_ASN_ENCODING, publicKeyInfo, &hPubKey)) {
        DWORD dw = GetLastError();
        qDebug() << "CryptImportPublicKeyInfo" << dw;
        setErrMsgFromGetLastError(dw);
        return false;
    }

    // Create the hash of the source data
    HCRYPTHASH hHash;
    if (!CryptCreateHash(hCryptProv, CALG_SHA1, 0, 0, &hHash)) {
        DWORD dw = GetLastError();
        qDebug() << "CryptCreateHash" << dw;
        setErrMsgFromGetLastError(dw);
        return false;
    }
    while (!sourceData.atEnd()) {
        QByteArray buffer = sourceData.read(4096);
        if (!CryptHashData(hHash, (const BYTE*)buffer.constData(), buffer.size(), 0)) {
            DWORD dw = GetLastError();
            qDebug() << "CryptHashData" << dw;
            setErrMsgFromGetLastError(dw);
            break;
        }
    }

    if (!CryptVerifySignature(hHash, (const BYTE*)signature.constData(), signature.length(), hPubKey, NULL, 0)) {
        DWORD dw = GetLastError();
        qDebug() << "CryptVerifySignature" << dw;
        setErrMsgFromGetLastError(dw);

        // Try again, but this time reverse the signature bytes because OpenSSL signatures are big endian and MS crypto requires little
        const BYTE *signatureBytes = (const BYTE*)signature.constData();
        int numBytes = signature.length();
        BYTE *reversedSignature = new BYTE[signature.length()];
        for (int i = 0; i < numBytes; ++i) {
            reversedSignature[i] = signatureBytes[(numBytes - 1) - i];
        }
        if (!CryptVerifySignature(hHash, reversedSignature, signature.length(), hPubKey, NULL, 0)) {
            DWORD dw = GetLastError();
            qDebug() << "CryptVerifySignature" << dw;
            setErrMsgFromGetLastError(dw);
        } else {
            verified = true;
        }

        delete[] reversedSignature;
    } else {
        verified = true;
    }

    (void)CryptDestroyHash(hHash);
    (void)CryptDestroyKey(hPubKey);
    (void)CryptReleaseContext(hCryptProv, 0);
    (void)LocalFree(publicKeyInfo);

    return verified;
}

QByteArray QGlitter::dsaSign(QIODevice &, const QByteArray &, const QString &)
{
    s_qglitterErrorMessage = "Not implemented";
    return QByteArray();
}
