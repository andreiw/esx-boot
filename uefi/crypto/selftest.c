/*******************************************************************************
 * Copyright (c) 2020-2021 VMware, Inc.  All rights reserved.
 * SPDX-License-Identifier: GPL-2.0
 ******************************************************************************/

/*
 *  selftest.c --
 *
 *      Power-on self test of the algorithms exported by the crypto module.
 */

#include <sys/stat.h>
#include <bootlib.h>
#include <efiutils.h>

#include <sha256.h>
#include <sha512.h>
#include <rsa.h>
#include <protocol/MbedTls.h>

#include "crypto.h"


#define SHA256_DIGEST_LENGTH (256 / 8)
#define SHA512_DIGEST_LENGTH (512 / 8)
#define MAX_DIGEST_LENGTH SHA512_DIGEST_LENGTH

/*
 * HMAC test data.
 */
typedef struct {
   const char *testid;
   mbedtls_md_type_t digest;
   const uint8_t *key;
   unsigned key_len;
   const uint8_t *data;
   unsigned data_len;
   const uint8_t *expected_hash;
   unsigned hash_len;
} HMACTestData;

/*
 * 191a940b09f5756f15ba4b67dbc9741bbe2d04661ca5d2acada7dd145a1b286f
 * b2bbebfd54757072a3ad3346133cbfafc5e0d87b8412094e21b06cf1433be269
 */
static const uint8_t hmac_key1[] = {
   0x19, 0x1a, 0x94, 0x0b, 0x09, 0xf5, 0x75, 0x6f, 0x15, 0xba, 0x4b, 0x67,
   0xdb, 0xc9, 0x74, 0x1b, 0xbe, 0x2d, 0x04, 0x66, 0x1c, 0xa5, 0xd2, 0xac,
   0xad, 0xa7, 0xdd, 0x14, 0x5a, 0x1b, 0x28, 0x6f, 0xb2, 0xbb, 0xeb, 0xfd,
   0x54, 0x75, 0x70, 0x72, 0xa3, 0xad, 0x33, 0x46, 0x13, 0x3c, 0xbf, 0xaf,
   0xc5, 0xe0, 0xd8, 0x7b, 0x84, 0x12, 0x09, 0x4e, 0x21, 0xb0, 0x6c, 0xf1,
   0x43, 0x3b, 0xe2, 0x69
};

static const uint8_t hmac_data1[] =
   "the quick brown fox jumps over the lazy dog";

static const uint8_t hmac_hash1[] = {
#if FORCE_HMAC_FAIL
   0x42, // incorrect value to force a self-test failure
#else
   0xcb,
#endif
         0xe7, 0x85, 0x72, 0xf8, 0xad, 0x79, 0x95, 0xd3, 0x41, 0x66, 0x3b,
   0x48, 0xf0, 0xc7, 0xe6, 0xd4, 0x22, 0x05, 0xd0, 0xb2, 0xb3, 0x49, 0xf5,
   0xcb, 0x32, 0x57, 0x53, 0xbc, 0xab, 0x7f, 0xdc, 0x82, 0xfd, 0xb5, 0xfc,
   0x3c, 0xe5, 0xe8, 0x4c, 0xb1, 0xdc, 0x13, 0x8b, 0xd7, 0xf5, 0x4a, 0x3d,
   0x57, 0xe3, 0x7b, 0x09, 0x6c, 0xfc, 0xba, 0x9c, 0x8c, 0xc6, 0x16, 0xc5,
   0x51, 0xbd, 0x33, 0x38
};

static const HMACTestData hmacTestData[] = {
   {
      "hmac1",
      MBEDTLS_MD_SHA512,
      hmac_key1,
      sizeof(hmac_key1),
      hmac_data1,
      sizeof(hmac_data1) - 1, //omit null terminator
      hmac_hash1,
      sizeof(hmac_hash1)
   },
};

/*-- hmac_test -----------------------------------------------------------------
 *
 *     HMAC power-on self-test.  Hashes string(s) and verifies the expected
 *     result.
 *
 * Results
 *      Exits with an error upon failure.
 *----------------------------------------------------------------------------*/
void hmac_test(void)
{
   unsigned i;

   for (i = 0; i < ARRAYSIZE(hmacTestData); i++) {
      const HMACTestData *htd = &hmacTestData[i];
      int errcode;
      uint8_t hash[MAX_DIGEST_LENGTH];

      errcode = mbedtls->HmacRet(htd->digest, htd->key, htd->key_len,
                                 htd->data, htd->data_len, hash);
      if (errcode != 0) {
         failure("mbedtls->HmacRet error");
      }

      if (memcmp(hash, htd->expected_hash, htd->hash_len) != 0) {
         failure(htd->testid);
      }
#if FORCE_STACK_SMASH
      strcpy((char*)&hash[MAX_DIGEST_LENGTH], "force stack smash");
#endif
   }
}


/*
 * RSA signature verification test data.
 */
typedef struct {
   const char *testid;
   unsigned modulo;
   const char *modulus;
   uint64_t exponent;
   mbedtls_md_type_t digest;
   bool result;
   const uint8_t *data;
   unsigned dataLen;
   const uint8_t *sig;
} RSASignVerifyData;

/*
 * Test case taken from
 * https://github.com/usnistgov/ACVP/blob/master/artifacts/acvp_sub_rsa.txt
 * Uses 2048-bit key with SHA2-256.  Verification should return true.
 */

static const uint8_t data1176[] = {
   0x95, 0x12, 0x3c, 0x8d, 0x1b, 0x23, 0x65, 0x40, 0xb8, 0x69, 0x76, 0xa1, 0x1c,
   0xea, 0x31, 0xf8, 0xbd, 0x4e, 0x6c, 0x54, 0xc2, 0x35, 0x14, 0x7d, 0x20, 0xce,
   0x72, 0x2b, 0x03, 0xa6, 0xad, 0x75, 0x6f, 0xbd, 0x91, 0x8c, 0x27, 0xdf, 0x8e,
   0xa9, 0xce, 0x31, 0x04, 0x44, 0x4c, 0x0b, 0xbe, 0x87, 0x73, 0x05, 0xbc, 0x02,
   0xe3, 0x55, 0x35, 0xa0, 0x2a, 0x58, 0xdc, 0xda, 0x30, 0x6e, 0x63, 0x2a, 0xd3,
   0x0b, 0x3d, 0xc3, 0xce, 0x0b, 0xa9, 0x7f, 0xdf, 0x46, 0xec, 0x19, 0x29, 0x65,
   0xdd, 0x9c, 0xd7, 0xf4, 0xa7, 0x1b, 0x02, 0xb8, 0xcb, 0xa3, 0xd4, 0x42, 0x64,
   0x6e, 0xee, 0xc4, 0xaf, 0x59, 0x08, 0x24, 0xca, 0x98, 0xd7, 0x4f, 0xbc, 0xa9,
   0x34, 0xd0, 0xb6, 0x86, 0x7a, 0xa1, 0x99, 0x1f, 0x30, 0x40, 0xb7, 0x07, 0xe8,
   0x06, 0xde, 0x6e, 0x66, 0xb5, 0x93, 0x4f, 0x05, 0x50, 0x9b, 0xea
};

static const uint8_t sig1176[] = {
#if FORCE_SIGVER_FAIL
   0x42, // incorrect value to force a self-test failure
#else
   0x51,
#endif
         0x26, 0x5d, 0x96, 0xf1, 0x1a, 0xb3, 0x38, 0x76, 0x28, 0x91, 0xcb, 0x29,
   0xbf, 0x3f, 0x1d, 0x2b, 0x33, 0x05, 0x10, 0x70, 0x63, 0xf5, 0xf3, 0x24, 0x5a,
   0xf3, 0x76, 0xdf, 0xcc, 0x70, 0x27, 0xd3, 0x93, 0x65, 0xde, 0x70, 0xa3, 0x1d,
   0xb0, 0x5e, 0x9e, 0x10, 0xeb, 0x61, 0x48, 0xcb, 0x7f, 0x64, 0x25, 0xf0, 0xc9,
   0x3c, 0x4f, 0xb0, 0xe2, 0x29, 0x1a, 0xdb, 0xd2, 0x2c, 0x77, 0x65, 0x6a, 0xfc,
   0x19, 0x68, 0x58, 0xa1, 0x1e, 0x1c, 0x67, 0x0d, 0x9e, 0xeb, 0x59, 0x26, 0x13,
   0xe6, 0x9e, 0xb4, 0xf3, 0xaa, 0x50, 0x17, 0x30, 0x74, 0x3a, 0xc4, 0x46, 0x44,
   0x86, 0xc7, 0xae, 0x68, 0xfd, 0x50, 0x9e, 0x89, 0x6f, 0x63, 0x88, 0x4e, 0x94,
   0x24, 0xf6, 0x9c, 0x1c, 0x53, 0x97, 0x95, 0x9f, 0x1e, 0x52, 0xa3, 0x68, 0x66,
   0x7a, 0x59, 0x8a, 0x1f, 0xc9, 0x01, 0x25, 0x27, 0x3d, 0x93, 0x41, 0x29, 0x5d,
   0x2f, 0x8e, 0x1c, 0xc4, 0x96, 0x9b, 0xf2, 0x28, 0xc8, 0x60, 0xe0, 0x7a, 0x35,
   0x46, 0xbe, 0x2e, 0xed, 0xa1, 0xcd, 0xe4, 0x8e, 0xe9, 0x4d, 0x06, 0x28, 0x01,
   0xfe, 0x66, 0x6e, 0x4a, 0x7a, 0xe8, 0xcb, 0x9c, 0xd7, 0x92, 0x62, 0xc0, 0x17,
   0xb0, 0x81, 0xaf, 0x87, 0x4f, 0xf0, 0x04, 0x53, 0xca, 0x43, 0xe3, 0x4e, 0xfd,
   0xb4, 0x3f, 0xff, 0xb0, 0xbb, 0x42, 0xa4, 0xe2, 0xd3, 0x2a, 0x5e, 0x5c, 0xc9,
   0xe8, 0x54, 0x6a, 0x22, 0x1f, 0xe9, 0x30, 0x25, 0x0e, 0x5f, 0x53, 0x33, 0xe0,
   0xef, 0xe5, 0x8f, 0xfe, 0xbf, 0x19, 0x36, 0x9a, 0x3b, 0x8a, 0xe5, 0xa6, 0x7f,
   0x6a, 0x04, 0x8b, 0xc9, 0xef, 0x91, 0x5b, 0xda, 0x25, 0x16, 0x07, 0x29, 0xb5,
   0x08, 0x66, 0x7a, 0xda, 0x84, 0xa0, 0xc2, 0x7e, 0x7e, 0x26, 0xcf, 0x2a, 0xbc,
   0xa4, 0x13, 0xe5, 0xe4, 0x69, 0x3f, 0x4a, 0x94, 0x05
};

static const RSASignVerifyData rsaSignVerifyData[] = {
   {
      "rsa1176",
      2048,
      "c47abacc2a84d56f3614d92fd62ed36ddde459664b9301dcd1d61781cfcc026b"
      "cb2399bee7e75681a80b7bf500e2d08ceae1c42ec0b707927f2b2fe92ae85208"
      "7d25f1d260cc74905ee5f9b254ed05494a9fe06732c3680992dd6f0dc634568d"
      "11542a705f83ae96d2a49763d5fbb24398edf3702bc94bc168190166492b8671"
      "de874bb9cecb058c6c8344aa8c93754d6effcd44a41ed7de0a9dcd9144437f21"
      "2b18881d042d331a4618a9e630ef9bb66305e4fdf8f0391b3b2313fe549f0189"
      "ff968b92f33c266a4bc2cffc897d1937eeb9e406f5d0eaa7a14782e76af3fce9"
      "8f54ed237b4a04a4159a5f6250a296a902880204e61d891c4da29f2d65f34cbb",
      0x49d2a1,
      MBEDTLS_MD_SHA256,
      true,
      data1176,
      sizeof(data1176),
      sig1176,
   },
};

/*-- rsa_sign_verify_test ------------------------------------------------------
 *
 *     Signature verification test. Computes the hash of the data and
 *     verifies it with the passed signature.
 *
 * Results
 *      Exits with an error upon failure.
 *----------------------------------------------------------------------------*/
void rsa_sign_verify_test(void)
{
   unsigned char md[MAX_DIGEST_LENGTH];
   int errcode;
   const RSASignVerifyData *signVerifyData;
   mbedtls_rsa_context rsa;
   unsigned i;

   for (i = 0; i < ARRAYSIZE(rsaSignVerifyData); i++) {
      signVerifyData = &rsaSignVerifyData[i];
      mbedtls->RsaInit(&rsa, MBEDTLS_RSA_PKCS_V15, MBEDTLS_MD_NONE);
      rsa.len = signVerifyData->modulo / 8;
      errcode = mbedtls->MpiReadString(&rsa.N, 16, signVerifyData->modulus);
      if (!errcode) {
         errcode = mbedtls->MpiLset(&rsa.E, signVerifyData->exponent);
      }
      if (errcode) {
         failure("mbedtls->MpiLset error");
      }

      switch (signVerifyData->digest) {
      case MBEDTLS_MD_SHA256:
         mbedtls->Sha256Ret(signVerifyData->data,
                            signVerifyData->dataLen, md, 0);
         errcode = mbedtls->RsaPkcs1Verify(&rsa, NULL, NULL, MBEDTLS_RSA_PUBLIC,
                                           signVerifyData->digest,
                                           SHA256_DIGEST_LENGTH, md,
                                           signVerifyData->sig);
         break;

      case MBEDTLS_MD_SHA512:
         mbedtls->Sha512Ret(signVerifyData->data,
                            signVerifyData->dataLen, md, 0);
         errcode = mbedtls->RsaPkcs1Verify(&rsa, NULL, NULL, MBEDTLS_RSA_PUBLIC,
                                           signVerifyData->digest,
                                           SHA512_DIGEST_LENGTH, md,
                                           signVerifyData->sig);
         break;

      default:
         failure("unknown digest type in self test data");
      }

      if ((errcode && signVerifyData->result) ||
          (!errcode && !signVerifyData->result)) {
         failure(signVerifyData->testid);
      }
   }
}


/*-- self_test -----------------------------------------------------------------
 *
 *     ESXboot cryptographic module power-on self-tests.
 *
 * Results
 *      Exits with an error upon failure.
 *----------------------------------------------------------------------------*/
void self_test(void)
{
   hmac_test();
   rsa_sign_verify_test();
}
