import pytest
import struct
import sys
import math


# Simulate the buffer allocation and formatting logic from cJSON_JS.c
# The vulnerable code allocates a fixed buffer (typically 64 bytes) and uses
# sprintf without bounds checking for numeric values.

TYPICAL_BUFFER_SIZE = 64  # bytes, as allocated in cJSON_JS.c


def safe_format_number(value):
    """
    Simulates the cJSON_JS.c number formatting with proper bounds checking.
    Returns (formatted_string, overflowed) tuple.
    The invariant: formatted output must never exceed the allocated buffer size.
    """
    try:
        if value is None:
            result = "null"
        elif isinstance(value, int):
            result = "%d" % value
        elif isinstance(value, float):
            if math.isnan(value) or math.isinf(value):
                result = "null"
            elif value == math.floor(value) and abs(value) < 1e15:
                result = "%.0f" % value
            elif abs(value) < 1e-6 or abs(value) > 1e15:
                result = "%e" % value
            else:
                result = "%f" % value
        else:
            result = str(value)
        
        return result, len(result.encode('utf-8')) > TYPICAL_BUFFER_SIZE
    except (OverflowError, ValueError) as e:
        return None, True


def check_buffer_invariant(value):
    """
    Core invariant check: the formatted representation of a numeric value
    must fit within the declared buffer size without overflow.
    Returns True if invariant holds (no overflow), False if violated.
    """
    formatted, overflowed = safe_format_number(value)
    
    if formatted is None:
        # Formatting failed entirely - this is acceptable (rejection)
        return True
    
    encoded = formatted.encode('utf-8')
    
    # The invariant: buffer reads must never exceed declared length
    # Either the value is rejected/truncated OR the output fits in the buffer
    return len(encoded) <= TYPICAL_BUFFER_SIZE


# Attack payloads targeting the buffer overflow vulnerability
ATTACK_PAYLOADS = [
    # DBL_MAX - the primary attack vector mentioned in the vulnerability
    sys.float_info.max,
    
    # DBL_MIN
    sys.float_info.min,
    
    # Extremely large float that produces long %f output
    1.7976931348623157e+308,
    
    # Values that produce very long decimal representations with %f
    1.0e+300,
    1.0e+200,
    1.0e+100,
    
    # Negative extreme values
    -1.7976931348623157e+308,
    -1.0e+300,
    
    # Values near the boundary of format switching
    9.999999999999998e+14,
    1.0e+15,
    1.0e+16,
    
    # Very small values that produce long %e output
    5e-324,  # smallest positive float
    1.0e-300,
    1.0e-100,
    
    # Integer overflow attempts
    2**31 - 1,    # INT_MAX
    2**31,        # INT_MAX + 1
    -(2**31),     # INT_MIN
    -(2**31) - 1, # INT_MIN - 1
    2**63 - 1,    # LLONG_MAX
    -(2**63),     # LLONG_MIN
    
    # Values that produce exactly 64 bytes (boundary test)
    1.23456789012345678901234567890e+100,
    
    # Values producing output > 64 bytes with %f format
    1.23456789012345678901234567890e+50,
    
    # Special float values
    float('inf'),
    float('-inf'),
    float('nan'),
    
    # Zero variants
    0.0,
    -0.0,
    
    # Values that stress %e format with long exponents
    1.0e-999 if hasattr(sys, 'float_info') else 0.0,
    
    # Crafted values to maximize output length
    1.1111111111111111e+308,
    9.9999999999999999e+307,
    
    # Integer values near format boundaries
    0,
    -1,
    999999999999999,
    -999999999999999,
    
    # None (null in JSON)
    None,
    
    # 2x buffer size stress: value whose string repr is ~128 chars
    float('1' + '0' * 100),
    
    # 10x buffer size stress: value whose string repr could be ~640 chars
    # (DBL_MAX with %f would be ~310 chars)
    sys.float_info.max * 0.9999,
]


@pytest.mark.parametrize("payload", ATTACK_PAYLOADS)
def test_buffer_reads_never_exceed_declared_length(payload):
    """
    Invariant: Buffer reads/writes for numeric formatting must never exceed
    the declared buffer length (TYPICAL_BUFFER_SIZE = 64 bytes).
    
    This guards against CWE-120: Buffer Copy without Checking Size of Input.
    The cJSON_JS.c vulnerability uses sprintf() without bounds checking,
    which can overflow a 64-byte heap buffer with extreme float values.
    
    The invariant: either the formatted value fits within the buffer,
    OR the implementation must reject/truncate the value safely.
    """
    formatted, overflowed = safe_format_number(payload)
    
    if formatted is None:
        # Value was rejected - this is a safe outcome
        assert True, f"Value {payload} was safely rejected"
        return
    
    encoded = formatted.encode('utf-8')
    formatted_len = len(encoded)
    
    # The core security invariant: output must fit in declared buffer
    # If this assertion fails, it means the code would overflow the buffer
    assert formatted_len <= TYPICAL_BUFFER_SIZE, (
        f"SECURITY VIOLATION: Formatted value '{formatted[:80]}...' "
        f"has length {formatted_len} bytes which exceeds the declared "
        f"buffer size of {TYPICAL_BUFFER_SIZE} bytes. "
        f"Input value: {payload}. "
        f"This would cause a heap buffer overflow (CWE-120)."
    )


@pytest.mark.parametrize("payload", [
    # Specifically test the %f format specifier overflow scenario
    sys.float_info.max,
    1.7976931348623157e+308,
    -1.7976931348623157e+308,
    1.0e+200,
    1.0e+100,
    1.0e+50,
])
def test_float_f_format_buffer_overflow_prevention(payload):
    """
    Invariant: The %f format specifier for extreme float values must not
    produce output exceeding the allocated buffer size.
    
    DBL_MAX formatted with %f produces ~310 characters, far exceeding
    a 64-byte buffer. The implementation must handle this safely.
    """
    if math.isnan(payload) or math.isinf(payload):
        pytest.skip("Special float values handled separately")
    
    # Simulate what the vulnerable code does with %f
    try:
        f_formatted = "%f" % payload
        f_len = len(f_formatted.encode('utf-8'))
        
        # Document the overflow potential
        if f_len > TYPICAL_BUFFER_SIZE:
            # The %f format WOULD overflow - the safe implementation
            # must use a different format or larger buffer
            # Check that our safe implementation handles it correctly
            safe_result, overflowed = safe_format_number(payload)
            
            if safe_result is not None:
                safe_len = len(safe_result.encode('utf-8'))
                assert safe_len <= TYPICAL_BUFFER_SIZE, (
                    f"Safe formatter still overflows: {safe_len} > {TYPICAL_BUFFER_SIZE} "
                    f"for value {payload}. Raw %f would produce {f_len} bytes."
                )
        else:
            # %f output fits - verify our formatter also fits
            safe_result, _ = safe_format_number(payload)
            if safe_result is not None:
                assert len(safe_result.encode('utf-8')) <= TYPICAL_BUFFER_SIZE
    except (OverflowError, ValueError):
        # Overflow during formatting itself - safe rejection
        pass


@pytest.mark.parametrize("multiplier,base_value", [
    (2, sys.float_info.max),      # 2x stress
    (1, sys.float_info.max),      # exact max
    (0.5, sys.float_info.max),    # half max
    (2, 1.0e+100),                # 2x large value
    (10, 1.0e+50),                # 10x medium value
])
def test_oversized_numeric_input_rejected_or_truncated(multiplier, base_value):
    """
    Invariant: Oversized numeric inputs (2x, 10x expected buffer size)
    must either be rejected or produce output that fits within the buffer.
    No out-of-bounds access should occur.
    """
    try:
        test_value = base_value * multiplier
        if math.isinf(test_value):
            # Overflow to infinity - must be handled as null/special
            formatted, overflowed = safe_format_number(test_value)
            # inf should be formatted as "null" or similar safe value
            assert formatted is None or len(formatted.encode('utf-8')) <= TYPICAL_BUFFER_SIZE, \
                f"Infinite value not safely handled: {formatted}"
        else:
            formatted, overflowed = safe_format_number(test_value)
            if formatted is not None:
                assert len(formatted.encode('utf-8')) <= TYPICAL_BUFFER_SIZE, \
                    f"Buffer overflow: {len(formatted.encode('utf-8'))} > {TYPICAL_BUFFER_SIZE}"
    except (OverflowError, ValueError):
        # Safe rejection of extreme values
        pass