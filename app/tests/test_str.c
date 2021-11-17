#include "common.h"

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "util/str.h"

static void test_strncpy_simple(void) {
    char s[] = "xxxxxxxxxx";
    size_t w = sc_strncpy(s, "abcdef", sizeof(s));

    // returns strlen of copied string
    assert(w == 6);

    // is nul-terminated
    assert(s[6] == '\0');

    // does not write useless bytes
    assert(s[7] == 'x');

    // copies the content as expected
    assert(!strcmp("abcdef", s));
}

static void test_strncpy_just_fit(void) {
    char s[] = "xxxxxx";
    size_t w = sc_strncpy(s, "abcdef", sizeof(s));

    // returns strlen of copied string
    assert(w == 6);

    // is nul-terminated
    assert(s[6] == '\0');

    // copies the content as expected
    assert(!strcmp("abcdef", s));
}

static void test_strncpy_truncated(void) {
    char s[] = "xxx";
    size_t w = sc_strncpy(s, "abcdef", sizeof(s));

    // returns 'n' (sizeof(s))
    assert(w == 4);

    // is nul-terminated
    assert(s[3] == '\0');

    // copies the content as expected
    assert(!strncmp("abcdef", s, 3));
}

static void test_join_simple(void) {
    const char *const tokens[] = { "abc", "de", "fghi", NULL };
    char s[] = "xxxxxxxxxxxxxx";
    size_t w = sc_str_join(s, tokens, ' ', sizeof(s));

    // returns strlen of concatenation
    assert(w == 11);

    // is nul-terminated
    assert(s[11] == '\0');

    // does not write useless bytes
    assert(s[12] == 'x');

    // copies the content as expected
    assert(!strcmp("abc de fghi", s));
}

static void test_join_just_fit(void) {
    const char *const tokens[] = { "abc", "de", "fghi", NULL };
    char s[] = "xxxxxxxxxxx";
    size_t w = sc_str_join(s, tokens, ' ', sizeof(s));

    // returns strlen of concatenation
    assert(w == 11);

    // is nul-terminated
    assert(s[11] == '\0');

    // copies the content as expected
    assert(!strcmp("abc de fghi", s));
}

static void test_join_truncated_in_token(void) {
    const char *const tokens[] = { "abc", "de", "fghi", NULL };
    char s[] = "xxxxx";
    size_t w = sc_str_join(s, tokens, ' ', sizeof(s));

    // returns 'n' (sizeof(s))
    assert(w == 6);

    // is nul-terminated
    assert(s[5] == '\0');

    // copies the content as expected
    assert(!strcmp("abc d", s));
}

static void test_join_truncated_before_sep(void) {
    const char *const tokens[] = { "abc", "de", "fghi", NULL };
    char s[] = "xxxxxx";
    size_t w = sc_str_join(s, tokens, ' ', sizeof(s));

    // returns 'n' (sizeof(s))
    assert(w == 7);

    // is nul-terminated
    assert(s[6] == '\0');

    // copies the content as expected
    assert(!strcmp("abc de", s));
}

static void test_join_truncated_after_sep(void) {
    const char *const tokens[] = { "abc", "de", "fghi", NULL };
    char s[] = "xxxxxxx";
    size_t w = sc_str_join(s, tokens, ' ', sizeof(s));

    // returns 'n' (sizeof(s))
    assert(w == 8);

    // is nul-terminated
    assert(s[7] == '\0');

    // copies the content as expected
    assert(!strcmp("abc de ", s));
}

static void test_quote(void) {
    const char *s = "abcde";
    char *out = sc_str_quote(s);

    // add '"' at the beginning and the end
    assert(!strcmp("\"abcde\"", out));

    free(out);
}

static void test_utf8_truncate(void) {
    const char *s = "aÉbÔc";
    assert(strlen(s) == 7); // É and Ô are 2 bytes-wide

    size_t count;

    count = sc_str_utf8_truncation_index(s, 1);
    assert(count == 1);

    count = sc_str_utf8_truncation_index(s, 2);
    assert(count == 1); // É is 2 bytes-wide

    count = sc_str_utf8_truncation_index(s, 3);
    assert(count == 3);

    count = sc_str_utf8_truncation_index(s, 4);
    assert(count == 4);

    count = sc_str_utf8_truncation_index(s, 5);
    assert(count == 4); // Ô is 2 bytes-wide

    count = sc_str_utf8_truncation_index(s, 6);
    assert(count == 6);

    count = sc_str_utf8_truncation_index(s, 7);
    assert(count == 7);

    count = sc_str_utf8_truncation_index(s, 8);
    assert(count == 7); // no more chars
}

static void test_parse_integer(void) {
    long value;
    bool ok = sc_str_parse_integer("1234", &value);
    assert(ok);
    assert(value == 1234);

    ok = sc_str_parse_integer("-1234", &value);
    assert(ok);
    assert(value == -1234);

    ok = sc_str_parse_integer("1234k", &value);
    assert(!ok);

    ok = sc_str_parse_integer("123456789876543212345678987654321", &value);
    assert(!ok); // out-of-range
}

static void test_parse_integers(void) {
    long values[5];

    size_t count = sc_str_parse_integers("1234", ':', 5, values);
    assert(count == 1);
    assert(values[0] == 1234);

    count = sc_str_parse_integers("1234:5678", ':', 5, values);
    assert(count == 2);
    assert(values[0] == 1234);
    assert(values[1] == 5678);

    count = sc_str_parse_integers("1234:5678", ':', 2, values);
    assert(count == 2);
    assert(values[0] == 1234);
    assert(values[1] == 5678);

    count = sc_str_parse_integers("1234:-5678", ':', 2, values);
    assert(count == 2);
    assert(values[0] == 1234);
    assert(values[1] == -5678);

    count = sc_str_parse_integers("1:2:3:4:5", ':', 5, values);
    assert(count == 5);
    assert(values[0] == 1);
    assert(values[1] == 2);
    assert(values[2] == 3);
    assert(values[3] == 4);
    assert(values[4] == 5);

    count = sc_str_parse_integers("1234:5678", ':', 1, values);
    assert(count == 0); // max_items == 1

    count = sc_str_parse_integers("1:2:3:4:5", ':', 3, values);
    assert(count == 0); // max_items == 3

    count = sc_str_parse_integers(":1234", ':', 5, values);
    assert(count == 0); // invalid

    count = sc_str_parse_integers("1234:", ':', 5, values);
    assert(count == 0); // invalid

    count = sc_str_parse_integers("1234:", ':', 1, values);
    assert(count == 0); // invalid, even when max_items == 1

    count = sc_str_parse_integers("1234::5678", ':', 5, values);
    assert(count == 0); // invalid
}

static void test_parse_integer_with_suffix(void) {
    long value;
    bool ok = sc_str_parse_integer_with_suffix("1234", &value);
    assert(ok);
    assert(value == 1234);

    ok = sc_str_parse_integer_with_suffix("-1234", &value);
    assert(ok);
    assert(value == -1234);

    ok = sc_str_parse_integer_with_suffix("1234k", &value);
    assert(ok);
    assert(value == 1234000);

    ok = sc_str_parse_integer_with_suffix("1234m", &value);
    assert(ok);
    assert(value == 1234000000);

    ok = sc_str_parse_integer_with_suffix("-1234k", &value);
    assert(ok);
    assert(value == -1234000);

    ok = sc_str_parse_integer_with_suffix("-1234m", &value);
    assert(ok);
    assert(value == -1234000000);

    ok = sc_str_parse_integer_with_suffix("123456789876543212345678987654321", &value);
    assert(!ok); // out-of-range

    char buf[32];

    sprintf(buf, "%ldk", LONG_MAX / 2000);
    ok = sc_str_parse_integer_with_suffix(buf, &value);
    assert(ok);
    assert(value == LONG_MAX / 2000 * 1000);

    sprintf(buf, "%ldm", LONG_MAX / 2000);
    ok = sc_str_parse_integer_with_suffix(buf, &value);
    assert(!ok);

    sprintf(buf, "%ldk", LONG_MIN / 2000);
    ok = sc_str_parse_integer_with_suffix(buf, &value);
    assert(ok);
    assert(value == LONG_MIN / 2000 * 1000);

    sprintf(buf, "%ldm", LONG_MIN / 2000);
    ok = sc_str_parse_integer_with_suffix(buf, &value);
    assert(!ok);
}

static void test_strlist_contains(void) {
    assert(sc_str_list_contains("a,bc,def", ',', "bc"));
    assert(!sc_str_list_contains("a,bc,def", ',', "b"));
    assert(sc_str_list_contains("", ',', ""));
    assert(sc_str_list_contains("abc,", ',', ""));
    assert(sc_str_list_contains(",abc", ',', ""));
    assert(sc_str_list_contains("abc,,def", ',', ""));
    assert(!sc_str_list_contains("abc", ',', ""));
    assert(sc_str_list_contains(",,|x", '|', ",,"));
    assert(sc_str_list_contains("xyz", '\0', "xyz"));
}

static void test_wrap_lines(void) {
    const char *s = "This is a text to test line wrapping. The lines must be "
                    "wrapped at a space or a line break.\n"
                    "\n"
                    "This rectangle must remains a rectangle because it is "
                    "drawn in lines having lengths lower than the specified "
                    "number of columns:\n"
                    "  +----+\n"
                    "  |    |\n"
                    "  +----+\n";

    //                     |----      1    1    2  2|
    //                     |0    5    0    5    0  3| <-- 24 columns
    const char *expected = "    This is a text to\n"
                           "    test line wrapping.\n"
                           "    The lines must be\n"
                           "    wrapped at a space\n"
                           "    or a line break.\n"
                           "    \n"
                           "    This rectangle must\n"
                           "    remains a rectangle\n"
                           "    because it is drawn\n"
                           "    in lines having\n"
                           "    lengths lower than\n"
                           "    the specified number\n"
                           "    of columns:\n"
                           "      +----+\n"
                           "      |    |\n"
                           "      +----+\n";

    char *formatted = sc_str_wrap_lines(s, 24, 4);
    assert(formatted);

    assert(!strcmp(formatted, expected));

    free(formatted);
}

static void test_truncate_first_line(void) {
    char s[] = "hello\nworld\n!";
    size_t len = sc_str_truncate_first_line(s, sizeof(s));

    assert(len == 5);
    assert(!strcmp("hello", s));
}

int main(int argc, char *argv[]) {
    (void) argc;
    (void) argv;

    test_strncpy_simple();
    test_strncpy_just_fit();
    test_strncpy_truncated();
    test_join_simple();
    test_join_just_fit();
    test_join_truncated_in_token();
    test_join_truncated_before_sep();
    test_join_truncated_after_sep();
    test_quote();
    test_utf8_truncate();
    test_parse_integer();
    test_parse_integers();
    test_parse_integer_with_suffix();
    test_strlist_contains();
    test_wrap_lines();
    test_truncate_first_line();
    return 0;
}