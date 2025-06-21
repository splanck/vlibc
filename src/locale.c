#include "locale.h"
#include "string.h"

static char current_locale[6] = "C";

static struct lconv c_lconv = {
    .decimal_point = ".",
    .thousands_sep = "",
    .grouping = "",
    .int_curr_symbol = "",
    .currency_symbol = "",
    .mon_decimal_point = ".",
    .mon_thousands_sep = "",
    .mon_grouping = "",
    .positive_sign = "",
    .negative_sign = "",
    .int_frac_digits = 127,
    .frac_digits = 127,
    .p_cs_precedes = 127,
    .p_sep_by_space = 127,
    .n_cs_precedes = 127,
    .n_sep_by_space = 127,
    .p_sign_posn = 127,
    .n_sign_posn = 127,
};

char *setlocale(int category, const char *locale)
{
    (void)category;
    if (!locale)
        return current_locale;

    if (strcmp(locale, "C") == 0 || strcmp(locale, "POSIX") == 0) {
        strncpy(current_locale, locale, sizeof(current_locale) - 1);
        current_locale[sizeof(current_locale) - 1] = '\0';
        return current_locale;
    }

    return NULL;
}

struct lconv *localeconv(void)
{
    return &c_lconv;
}
