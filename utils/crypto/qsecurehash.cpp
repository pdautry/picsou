#include "qsecurehash.h"

#ifndef USE_WIN_CRYPTO_API
#include <gcrypt.h>
#else
#   error   not implemented !
#endif

#include "utils/macro.h"

QSecureHash::~QSecureHash()
{
    gcry_md_close(_hd);
    _hd=NULL;
}

QSecureHash::QSecureHash(HashAlgorithm algo,
                         HashFlag flags,
                         const QSecureMemory key)
{
    uint hash_flags;

    switch(algo) {
        case SHA1: _algo=GCRY_MD_SHA1; break;
        case RMD160: _algo=GCRY_MD_RMD160; break;
        case MD5: _algo=GCRY_MD_MD5; break;
        case MD4: _algo=GCRY_MD_MD4; break;
        case MD2: _algo=GCRY_MD_MD2; break;
        case TIGER: _algo=GCRY_MD_TIGER; break;
        case TIGER1: _algo=GCRY_MD_TIGER1; break;
        case TIGER2: _algo=GCRY_MD_TIGER2; break;
        case HAVAL: _algo=GCRY_MD_HAVAL; break;
        case SHA224: _algo=GCRY_MD_SHA224; break;
        case SHA256: _algo=GCRY_MD_SHA256; break;
        case SHA384: _algo=GCRY_MD_SHA384; break;
        case SHA512: _algo=GCRY_MD_SHA512; break;
        case SHA3_224: _algo=GCRY_MD_SHA3_224; break;
        case SHA3_256: _algo=GCRY_MD_SHA3_256; break;
        case SHA3_384: _algo=GCRY_MD_SHA3_384; break;
        case SHA3_512: _algo=GCRY_MD_SHA3_512; break;
        case SHAKE128: _algo=GCRY_MD_SHAKE128; break;
        case SHAKE256: _algo=GCRY_MD_SHAKE256; break;
        case CRC32: _algo=GCRY_MD_CRC32; break;
        case CRC32_RFC1510: _algo=GCRY_MD_CRC32_RFC1510; break;
        case CRC24_RFC2440: _algo=GCRY_MD_CRC24_RFC2440; break;
        case WHIRLPOOL: _algo=GCRY_MD_WHIRLPOOL; break;
        case GOSTR3411_94: _algo=GCRY_MD_GOSTR3411_94; break;
        case STRIBOG256: _algo=GCRY_MD_STRIBOG256; break;
        case STRIBOG512: _algo=GCRY_MD_STRIBOG512; break;
#ifdef ENABLE_BLAKE__algoRITHM
        case BLAKE2B_512: _algo=GCRY_MD_BLAKE2B_512; break;
        case BLAKE2B_384: _algo=GCRY_MD_BLAKE2B_384; break;
        case BLAKE2B_256: _algo=GCRY_MD_BLAKE2B_256; break;
        case BLAKE2B_160: _algo=GCRY_MD_BLAKE2B_160; break;
        case BLAKE2S_256: _algo=GCRY_MD_BLAKE2S_256; break;
        case BLAKE2S_224: _algo=GCRY_MD_BLAKE2S_224; break;
        case BLAKE2S_160: _algo=GCRY_MD_BLAKE2S_160; break;
        case BLAKE2S_128: _algo=GCRY_MD_BLAKE2S_128; break;
#endif
    }

    hash_flags = 0;
    hash_flags |= (IS_FLAG_SET(flags, SECURE)? GCRY_MD_FLAG_SECURE: 0);
    hash_flags |= (IS_FLAG_SET(flags, HMAC)? GCRY_MD_FLAG_HMAC: 0);
    hash_flags |= (IS_FLAG_SET(flags, BUGEMU1)? GCRY_MD_FLAG_BUGEMU1: 0);

    if(GCRYPT_FAILED(gcry_md_open, &_hd, _algo, hash_flags)) {
        goto end;
    }

    if(IS_FLAG_SET(flags, HMAC)) {
        if(GCRYPT_FAILED(gcry_md_setkey, _hd, key.const_data(), key.length())) {
            goto failed;
        }
    }
    goto end;

failed:
    gcry_md_close(_hd);
    _hd=NULL;
end:
    return;
}

bool QSecureHash::valid() const
{
    return (_hd!=NULL);
}

void QSecureHash::reset()
{
    gcry_md_reset(_hd);
}

void QSecureHash::update(const QSecureMemory data)
{
    gcry_md_write(_hd, data.const_data(), data.length());
}

bool QSecureHash::digest(QSecureMemory digest)
{
    return !GCRYPT_FAILED(gcry_md_extract, _hd, _algo, digest.data(), digest.length());
}