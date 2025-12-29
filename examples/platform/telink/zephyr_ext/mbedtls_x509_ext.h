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

#ifndef MBEDTLS_X509_EXT_H
#define MBEDTLS_X509_EXT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <mbedtls/ecp.h>

int ec_keys_bin_to_pem(mbedtls_ecp_group_id ecp_group_id,
	const uint8_t *pri, size_t pri_len, uint8_t *pem_buf, size_t pem_len);

int cert_der_to_pem(const uint8_t *der, size_t der_len, uint8_t *pem_buf, size_t pem_len);

#ifdef __cplusplus
}
#endif

#endif /* MBEDTLS_X509_EXT_H */
