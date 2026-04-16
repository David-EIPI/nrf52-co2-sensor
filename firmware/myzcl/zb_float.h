#ifndef ZB_FLOAT_H__
#define ZB_FLOAT_H__

/*  Wrapper type to simulate floating point type that is used by Concentration Measurement cluster.
    Below is the function to convert integer data to binary representation of single precision float
    compatible with ZCL standard types.
 */
typedef struct {
    uint32_t v;
} zb_float32_t;


/**@brief Convert integer measurement value in PPM units (parts per million) to
  single precision floating point fractional value (that is the output value is the input x 1e-6) */
inline static
void concentration_ppm_to_float(const uint32_t ppm, zb_float32_t *out)
{
/*  IEEE754 float is represented as series of powers of 2. Therefore, to scale the input PPM value
    by 1e-6  we need to switch decimal fraction to power of 2 fraction.
    To do that we first multiply the PPM value by 2^20 = 1048576 and divide by 1_000_000.
    Then we subtract 20 from the exponent part in the floating point representatation.
    In the actual calculation below, we scale down both 1_000_000 and 2^20 by 2^6 to allow for larger
    PPM values without 32-bit overflow. The largest PPM value that can be safely converted
    is therefore 2^18 = 262144.
*/
    uint32_t a = (ppm * (1 << 14)) / 15625;

    int32_t upper_bit = a ? 31 - __builtin_clz(a) : 0;
/* Exponent part of the IEE754 float */
    uint32_t x = a ? (0x80 ^ (unsigned char)(upper_bit - 20 - 1)) : 0;
/* Mantissa */
    uint32_t m = a ? (a & ((1 << upper_bit) - 1)) : 0;
/* Build final output value */
    out->v = (x << 23) | (m << (23 - upper_bit));
}

/**@brief Convert integer measurement value to single precision floating point integer value. */
inline static
void int32_to_float(const int32_t val, zb_float32_t *out)
{
    int32_t a = (val * (1 << 14)) / 15625;
    int sign = a < 0;
    if (sign) a = -a;

    int32_t upper_bit = a ? 31 - __builtin_clz(a) : 0;
/* Exponent part of the IEE754 float */
    uint32_t x = a ? (0x80 ^ (unsigned char)(upper_bit - 1)) : 0;
/* Mantissa */
    uint32_t m = a ? (a & ((1 << upper_bit) - 1)) : 0;
/* Build final output value */
    out->v = (x << 23) | (m << (23 - upper_bit)) | (sign << 31);
}

/**@brief Convert integer floating point to signed integer type (fractional part is discarded). */
inline static
void float_to_int32(const zb_float32_t *val, int32_t *out)
{
    uint32_t a = val->v;

    int sign = a & (1<<31);

    uint32_t x = (((a >> 23) & 0xff) ^ 0x80) + 1;
    if (x > 31) {
        *out = sign + (0x80000000u - 1u);
        return;
    }
    uint32_t m = a & ((1 << 23) - 1);
    int32_t upper_bit = m ? 31 - __builtin_clz(m) : 0;

    if (0 == m) {
        if (0 == x) {
            *out = 0;
            return;
        } else {
            m = 1;
        }
    } else {
        m |= (1 << (upper_bit + 1));
    }

    if (x > upper_bit) {
        *out = m << (x - upper_bit - 1);
    } else {
        *out = m >> (upper_bit - x + 1);
    }

    if (sign)
        *out = - *out;
}


#endif
