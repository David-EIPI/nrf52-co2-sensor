/*
 * Minimal nrf_crypto compatibility for unsigned in-application DFU.
 *
 * Nordic's DFU validation module uses nrf_crypto for two separate jobs:
 * SHA-256 firmware hash verification and ECDSA signature verification. This
 * application disables signed updates, so the large ECC/ECDSA backends are not
 * linked. The SHA-256 adapter below keeps the SDK init-packet hash check
 * working through the much smaller sha256.c module.
 */

#include <stddef.h>
#include <string.h>

#include "nrf_crypto.h"
#include "nrf_crypto_shared.h"
#include "sha256.h"

const uint8_t pk[64] = {0};

const nrf_crypto_hash_info_t g_nrf_crypto_hash_sha256_info =
{
    .digest_size = NRF_CRYPTO_HASH_SIZE_SHA256,
    .hash_mode   = NRF_CRYPTO_HASH_MODE_SHA256,
};

const nrf_crypto_ecc_curve_info_t g_nrf_crypto_ecc_secp256r1_curve_info =
{
    0
};

ret_code_t nrf_crypto_init(void)
{
    return NRF_SUCCESS;
}

ret_code_t nrf_crypto_hash_calculate(nrf_crypto_hash_context_t    * const p_context,
                                     nrf_crypto_hash_info_t       const * p_info,
                                     uint8_t                      const * p_data,
                                     size_t                               data_size,
                                     uint8_t                            * p_digest,
                                     size_t                       * const p_digest_size)
{
    ret_code_t       err_code;
    sha256_context_t sha256_ctx;

    UNUSED_PARAMETER(p_context);

    if ((p_info != &g_nrf_crypto_hash_sha256_info) ||
        (p_data == NULL) ||
        (p_digest == NULL) ||
        (p_digest_size == NULL) ||
        (*p_digest_size < NRF_CRYPTO_HASH_SIZE_SHA256))
    {
        return NRF_ERROR_INVALID_PARAM;
    }

    err_code = sha256_init(&sha256_ctx);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    err_code = sha256_update(&sha256_ctx, p_data, data_size);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    err_code = sha256_final(&sha256_ctx, p_digest, false);
    if (err_code == NRF_SUCCESS)
    {
        *p_digest_size = NRF_CRYPTO_HASH_SIZE_SHA256;
    }

    return err_code;
}

ret_code_t nrf_crypto_ecc_public_key_from_raw(
    nrf_crypto_ecc_curve_info_t const * p_curve_info,
    nrf_crypto_ecc_public_key_t       * p_public_key,
    uint8_t                     const * p_raw_data,
    size_t                              raw_data_size)
{
    UNUSED_PARAMETER(p_curve_info);
    UNUSED_PARAMETER(p_public_key);
    UNUSED_PARAMETER(p_raw_data);
    UNUSED_PARAMETER(raw_data_size);

    return NRF_ERROR_NOT_SUPPORTED;
}

ret_code_t nrf_crypto_ecdsa_verify(
    nrf_crypto_ecdsa_verify_context_t       * p_context,
    nrf_crypto_ecc_public_key_t       const * p_public_key,
    uint8_t                           const * p_hash,
    size_t                                    hash_size,
    uint8_t                           const * p_signature,
    size_t                                    signature_size)
{
    UNUSED_PARAMETER(p_context);
    UNUSED_PARAMETER(p_public_key);
    UNUSED_PARAMETER(p_hash);
    UNUSED_PARAMETER(hash_size);
    UNUSED_PARAMETER(p_signature);
    UNUSED_PARAMETER(signature_size);

    return NRF_ERROR_NOT_SUPPORTED;
}

void nrf_crypto_internal_swap_endian(uint8_t * p_out, uint8_t const * p_in, size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        p_out[i] = p_in[size - 1 - i];
    }
}

void nrf_crypto_internal_double_swap_endian(uint8_t * p_out, uint8_t const * p_in, size_t part_size)
{
    nrf_crypto_internal_swap_endian(p_out, p_in, part_size);
    nrf_crypto_internal_swap_endian(&p_out[part_size], &p_in[part_size], part_size);
}

void nrf_crypto_internal_double_swap_endian_in_place(uint8_t * p_buffer, size_t part_size)
{
    uint8_t tmp[NRF_CRYPTO_ECDSA_SECP256R1_SIGNATURE_SIZE];

    if ((part_size * 2) > sizeof(tmp))
    {
        return;
    }

    nrf_crypto_internal_double_swap_endian(tmp, p_buffer, part_size);
    memcpy(p_buffer, tmp, part_size * 2);
}
