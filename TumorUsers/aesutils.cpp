#include "aesutils.h"
#include <aes.h>
#include <modes.h>
#include <filters.h>
#include <hex.h>
#include <sha.h>

#include <QString>
#include <QDebug>

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
    decoder.Put((byte*)stdAesKey.data(), AESKEY_LENGTH-1);
    decoder.MessageEnd();
    word64 size = decoder.MaxRetrievable();
    char *decodedKey = new char[size];
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
//    qDebug() << "decrypting" <<  message;

    // putting full 64 hexes make decryption to fail sometimes
    // AES 252 bits :D

    decoder.Put( (byte *)stdAesKey.data(), AESKEY_LENGTH-1);
    decoder.MessageEnd();
    word64 size = decoder.MaxRetrievable();
    char *decodedKey = new char[size];
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

QString AesUtils::encryptMasterKey(QString password, QString filling, QString masterKey)
{
    QString myAesKey = password + filling;
    myAesKey.truncate(AESKEY_LENGTH);

    return AesUtils::encrypt(masterKey, myAesKey);
}

QString AesUtils::decryptMasterKey(QString password, QString filling, QString masterHash)
{
    QString myAesKey = password + filling;
    myAesKey.truncate(AESKEY_LENGTH);
    return AesUtils::decrypt(masterHash, myAesKey);
}
