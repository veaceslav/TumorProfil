#include "aesutils.h"
#include <aes.h>
#include <modes.h>
#include <filters.h>
#include <hex.h>
#include <sha.h>
#include <pwdbased.h>

#include <QString>
#include <QDebug>

#include "tumoruserconstants.h"

using namespace std;
using namespace CryptoPP;

AesUtils::AesUtils()
{

}

QString AesUtils::encrypt(QString message, QString aesKey)
{
    string plain = message.toStdString();
    string ciphertext;
    // Hex decode symmetric key:
    HexDecoder decoder;
    string stdAesKey = aesKey.toStdString();
//    qDebug() << "Encrypt " << message;

    // putting full 64 hexes make decryption to fail sometimes
    // AES 252 bits :D
    decoder.Put((byte*)stdAesKey.data(), AESKEY_LENGTH);
    decoder.MessageEnd();
    word64 size = decoder.MaxRetrievable();
    char decodedKey[size+2];
    memset(decodedKey, 0, size+2);
    decoder.Get((byte *)decodedKey, size);
    // Generate Cipher, Key, and CBC
    byte key[ AES::MAX_KEYLENGTH ], iv[ AES::BLOCKSIZE ];
    StringSource( reinterpret_cast<const char *>(decodedKey), true,
                  new HashFilter(*(new SHA256), new ArraySink(key, AES::MAX_KEYLENGTH)) );
    memset( iv, 0x00, AES::BLOCKSIZE );
    CBC_Mode<AES>::Encryption Encryptor( key, AES::MAX_KEYLENGTH, iv );
    StringSource( plain, true, new StreamTransformationFilter( Encryptor,
                  new HexEncoder(new StringSink( ciphertext )) ) );
    return QString::fromStdString(ciphertext);
}

QString AesUtils::decrypt(QString message, QString aesKey)
{
    string plain;
    string encrypted = message.toStdString();

    // Hex decode symmetric key:
    HexDecoder decoder;
    string stdAesKey = aesKey.toStdString();

    decoder.Put( (byte *)stdAesKey.data(), AESKEY_LENGTH);
    decoder.MessageEnd();
    word64 size = decoder.MaxRetrievable();
    char decodedKey[size+2];
    memset(decodedKey, 0, size+2);
    decoder.Get((byte *)decodedKey, size);


    // Generate Cipher, Key, and CBC
    byte key[ AES::MAX_KEYLENGTH ], iv[ AES::BLOCKSIZE ];
    StringSource( reinterpret_cast<const char *>(decodedKey), true,
                  new HashFilter(*(new SHA256), new ArraySink(key, AES::MAX_KEYLENGTH)) );
    memset( iv, 0x00, AES::BLOCKSIZE );
    qDebug() << "DEcoded key" << key << " " << AES::MAX_KEYLENGTH;
    try {
        CBC_Mode<AES>::Decryption Decryptor
        ( key, AES::MAX_KEYLENGTH, iv );
        StringSource( encrypted, true,
                      new HexDecoder(new StreamTransformationFilter( Decryptor,
                                     new StringSink( plain )) ) );
    }
    catch (Exception &e) { // ...
        qDebug() << "Exception while decrypting " << e.GetWhat().data();
    }
    catch (...) { // ...
    }
    return QString::fromStdString(plain);
}

QString AesUtils::deriveKey(QString password, QString salt)
{

    byte derived_key[AESKEY_LENGTH];

    memset(derived_key,0, AESKEY_LENGTH);
    byte purpose = 2;
    const byte* passwordData = (byte*)password.toLocal8Bit().constData();

    const byte* saltData = (byte*)salt.toLocal8Bit().constData();
    PKCS5_PBKDF2_HMAC<SHA256> pbkdf;

    pbkdf.DeriveKey (derived_key,AESKEY_LENGTH,
            purpose,
            passwordData,
            strlen((char*)passwordData),
            saltData,
            strlen((char*)saltData),
            PBKDF_ITERATIONS,
            PBKDF_TIME
        );

    return QString(QByteArray((const char*)derived_key, AESKEY_LENGTH));
}

QString AesUtils::encryptMasterKey(QString password, QString filling, QString masterKey)
{
    QString myAesKey = password + filling;
    myAesKey.truncate(AESKEY_LENGTH);

    qDebug() << "MyAESkey for encryption: " << myAesKey << " " << myAesKey.size();
    return AesUtils::encrypt(masterKey, myAesKey);
}

QString AesUtils::decryptMasterKey(QString password, QString filling, QString masterHash)
{
    QString myAesKey = password + filling;
    myAesKey.truncate(AESKEY_LENGTH);

    qDebug() << "MyAESkey for decryption: " << myAesKey << " " << myAesKey.size();
    return AesUtils::decrypt(masterHash, myAesKey);
}

