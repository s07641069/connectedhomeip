/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
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

#include <x509Credentials.h>
#include <mbedtls_x509_ext.h>

#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/CHIPCert.h>

using namespace chip;
using namespace chip::Credentials;

int X509Credentials::GetKEY(uint8_t *pem_buf, size_t pem_len)
{
	int result = -1;
	DeviceAttestationCredentialsProvider * default_provider =
		GetDeviceAttestationCredentialsProvider();

	if (default_provider)
	{
		uint8_t pri_key_buf[32];
		MutableByteSpan pri_key_span(pri_key_buf);
		CHIP_ERROR err = default_provider->GetDeviceAttestationCertPrivateKey(pri_key_span);

		if (err == CHIP_NO_ERROR)
		{
			result = ec_keys_bin_to_pem(MBEDTLS_ECP_DP_SECP256R1,
				pri_key_span.data(), pri_key_span.size(), pem_buf, pem_len);
		}
	}
	return result;
}

int X509Credentials::GetDAC(uint8_t *pem_buf, size_t pem_len)
{
	int result = -1;
	DeviceAttestationCredentialsProvider * default_provider =
		GetDeviceAttestationCredentialsProvider();

	if (default_provider)
	{
		uint8_t der_cert_buf[kMaxDERCertLength];
		MutableByteSpan der_cert_span(der_cert_buf);
		CHIP_ERROR err = default_provider->GetDeviceAttestationCert(der_cert_span);

		if (err == CHIP_NO_ERROR)
		{
			result = cert_der_to_pem(der_cert_span.data(), der_cert_span.size(), pem_buf, pem_len);
		}
	}
	return result;
}

int X509Credentials::GetPAI(uint8_t *pem_buf, size_t pem_len)
{
	int result = -1;
	DeviceAttestationCredentialsProvider * default_provider =
		GetDeviceAttestationCredentialsProvider();

	if (default_provider)
	{
		uint8_t der_cert_buf[kMaxDERCertLength];
		MutableByteSpan der_cert_span(der_cert_buf);
		CHIP_ERROR err = default_provider->GetProductAttestationIntermediateCert(der_cert_span);

		if (err == CHIP_NO_ERROR)
		{
			result = cert_der_to_pem(der_cert_span.data(), der_cert_span.size(), pem_buf, pem_len);
		}
	}
	return result;
}
