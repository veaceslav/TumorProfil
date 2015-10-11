#include "aesutils.h"
#include <aes.h>
#include <modes.h>
#include <filters.h>
#include <hex.h>
#include <sha.h>

#include <QString>

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
    decoder.Put((byte*)stdAesKey.data(), 32*2 );
    decoder.MessageEnd();
    word64 size = decoder.MaxRetrievable();
    char *decodedKey = new char[size];
    decoder.Get((byte *)decodedKey, size);
    // Generate Cipher, Key, and CBC
    byte key[ AES::MAX_KEYLENGTH ], iv[ AES::BLOCKSIZE ];
    StringSource( reinterpret_cast<const char *>(decodedKey), true,
                  new HashFilter(*(new SHA256), new ArraySink(key, AES::MAX_KEYLENGTH)) );
    memset( iv, 0x00, AES::BLOCKSIZE );
    CBC_Mode<AES>::Encryption Encryptor( key, sizeof(key), iv );
    StringSource( plain, true, new StreamTransformationFilter( Encryptor,
                  new HexEncoder(new StringSink( ciphertext ) ) ) );
    return QString::fromStdString(ciphertext);
}

QString AesUtils::decrypt(QString message, QString aesKey)
{
    string plain;
    string encrypted = message.toStdString();
    // Hex decode symmetric key:
    HexDecoder decoder;
    string stdAesKey = aesKey.toStdString();
    decoder.Put( (byte *)stdAesKey.data(),32*2 );
    decoder.MessageEnd();
    word64 size = decoder.MaxRetrievable();
    char *decodedKey = new char[size];
    decoder.Get((byte *)decodedKey, size);
    // Generate Cipher, Key, and CBC
    byte key[ AES::MAX_KEYLENGTH ], iv[ AES::BLOCKSIZE ];
    StringSource( reinterpret_cast<const char *>(decodedKey), true,
                  new HashFilter(*(new SHA256), new ArraySink(key, AES::MAX_KEYLENGTH)) );
    memset( iv, 0x00, AES::BLOCKSIZE );
    try {
        CBC_Mode<AES>::Decryption Decryptor
        ( key, sizeof(key), iv );
        StringSource( encrypted, true,
                      new HexDecoder(new StreamTransformationFilter( Decryptor,
                                     new StringSink( plain ) ) ) );
    }
    catch (Exception &e) { // ...
    }
    catch (...) { // ...
    }
    return QString::fromStdString(plain);
}

