/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include "mbedtls_x509_ext.h"
#include <mbedtls/pk.h>
#include <mbedtls/pem.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>

int ec_keys_bin_to_pem(mbedtls_ecp_group_id ecp_group_id,
	const uint8_t *pri, size_t pri_len, uint8_t *pem_buf, size_t pem_len)
{
	int result;
	mbedtls_pk_context pk;
	mbedtls_entropy_context entropy;
	mbedtls_ctr_drbg_context ctr_drbg;

	mbedtls_pk_init(&pk);
	mbedtls_entropy_init(&entropy);
	mbedtls_ctr_drbg_init(&ctr_drbg);
	do {
		result = mbedtls_pk_setup(&pk, mbedtls_pk_info_from_type(MBEDTLS_PK_ECKEY));
		if (result)
		{
			break;
		}
		result = mbedtls_ecp_group_load(&mbedtls_pk_ec(pk)->private_grp, ecp_group_id);
		if (result)
		{
			break;
		}
		result = mbedtls_mpi_read_binary(&mbedtls_pk_ec(pk)->private_d, pri, pri_len);
		if (result)
		{
			break;
		}
		result = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, NULL, 0);
		if (result)
		{
			break;
		}
		result = mbedtls_ecp_mul(&mbedtls_pk_ec(pk)->private_grp, &mbedtls_pk_ec(pk)->private_Q,
				&mbedtls_pk_ec(pk)->private_d, &mbedtls_pk_ec(pk)->private_grp.G,
				mbedtls_ctr_drbg_random, &ctr_drbg);
		if (result)
		{
			break;
		}
		result = mbedtls_pk_write_key_pem(&pk, pem_buf, pem_len);
	} while (0);
	mbedtls_ctr_drbg_free(&ctr_drbg);
	mbedtls_entropy_free(&entropy);
	mbedtls_pk_free(&pk);
	return result;
}

int cert_der_to_pem(const uint8_t *der, size_t der_len, uint8_t *pem_buf, size_t pem_len)
{
	size_t pem_olen;

	return mbedtls_pem_write_buffer("-----BEGIN CERTIFICATE-----\n",
			"-----END CERTIFICATE-----\n", der, der_len, pem_buf, pem_len, &pem_olen);
}
