#define OUTPUT_LINE_MAX 1024

typedef struct ConvLetter {
        char win1251;
        int unicode;
} Letter;
 
 
static Letter g_letters[] = {
        {0x82, 0x201A}, // SINGLE LOW-9 QUOTATION MARK
        {0x83, 0x0453}, // CYRILLIC SMALL LETTER GJE
        {0x84, 0x201E}, // DOUBLE LOW-9 QUOTATION MARK
        {0x85, 0x2026}, // HORIZONTAL ELLIPSIS
        {0x86, 0x2020}, // DAGGER
        {0x87, 0x2021}, // DOUBLE DAGGER
        {0x88, 0x20AC}, // EURO SIGN
        {0x89, 0x2030}, // PER MILLE SIGN
        {0x8A, 0x0409}, // CYRILLIC CAPITAL LETTER LJE
        {0x8B, 0x2039}, // SINGLE LEFT-POINTING ANGLE QUOTATION MARK
        {0x8C, 0x040A}, // CYRILLIC CAPITAL LETTER NJE
        {0x8D, 0x040C}, // CYRILLIC CAPITAL LETTER KJE
        {0x8E, 0x040B}, // CYRILLIC CAPITAL LETTER TSHE
        {0x8F, 0x040F}, // CYRILLIC CAPITAL LETTER DZHE
        {0x90, 0x0452}, // CYRILLIC SMALL LETTER DJE
        {0x91, 0x2018}, // LEFT SINGLE QUOTATION MARK
        {0x92, 0x2019}, // RIGHT SINGLE QUOTATION MARK
        {0x93, 0x201C}, // LEFT DOUBLE QUOTATION MARK
        {0x94, 0x201D}, // RIGHT DOUBLE QUOTATION MARK
        {0x95, 0x2022}, // BULLET
        {0x96, 0x2013}, // EN DASH
        {0x97, 0x2014}, // EM DASH
        {0x99, 0x2122}, // TRADE MARK SIGN
        {0x9A, 0x0459}, // CYRILLIC SMALL LETTER LJE
        {0x9B, 0x203A}, // SINGLE RIGHT-POINTING ANGLE QUOTATION MARK
        {0x9C, 0x045A}, // CYRILLIC SMALL LETTER NJE
        {0x9D, 0x045C}, // CYRILLIC SMALL LETTER KJE
        {0x9E, 0x045B}, // CYRILLIC SMALL LETTER TSHE
        {0x9F, 0x045F}, // CYRILLIC SMALL LETTER DZHE
        {0xA0, 0x00A0}, // NO-BREAK SPACE
        {0xA1, 0x040E}, // CYRILLIC CAPITAL LETTER SHORT U
        {0xA2, 0x045E}, // CYRILLIC SMALL LETTER SHORT U
        {0xA3, 0x0408}, // CYRILLIC CAPITAL LETTER JE
        {0xA4, 0x00A4}, // CURRENCY SIGN
        {0xA5, 0x0490}, // CYRILLIC CAPITAL LETTER GHE WITH UPTURN
        {0xA6, 0x00A6}, // BROKEN BAR
        {0xA7, 0x00A7}, // SECTION SIGN
        {0xA8, 0x0401}, // CYRILLIC CAPITAL LETTER IO
        {0xA9, 0x00A9}, // COPYRIGHT SIGN
        {0xAA, 0x0404}, // CYRILLIC CAPITAL LETTER UKRAINIAN IE
        {0xAB, 0x00AB}, // LEFT-POINTING DOUBLE ANGLE QUOTATION MARK
        {0xAC, 0x00AC}, // NOT SIGN
        {0xAD, 0x00AD}, // SOFT HYPHEN
        {0xAE, 0x00AE}, // REGISTERED SIGN
        {0xAF, 0x0407}, // CYRILLIC CAPITAL LETTER YI
        {0xB0, 0x00B0}, // DEGREE SIGN
        {0xB1, 0x00B1}, // PLUS-MINUS SIGN
        {0xB2, 0x0406}, // CYRILLIC CAPITAL LETTER BYELORUSSIAN-UKRAINIAN I
        {0xB3, 0x0456}, // CYRILLIC SMALL LETTER BYELORUSSIAN-UKRAINIAN I
        {0xB4, 0x0491}, // CYRILLIC SMALL LETTER GHE WITH UPTURN
        {0xB5, 0x00B5}, // MICRO SIGN
        {0xB6, 0x00B6}, // PILCROW SIGN
        {0xB7, 0x00B7}, // MIDDLE DOT
        {0xB8, 0x0451}, // CYRILLIC SMALL LETTER IO
        {0xB9, 0x2116}, // NUMERO SIGN
        {0xBA, 0x0454}, // CYRILLIC SMALL LETTER UKRAINIAN IE
        {0xBB, 0x00BB}, // RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK
        {0xBC, 0x0458}, // CYRILLIC SMALL LETTER JE
        {0xBD, 0x0405}, // CYRILLIC CAPITAL LETTER DZE
        {0xBE, 0x0455}, // CYRILLIC SMALL LETTER DZE
        {0xBF, 0x0457} // CYRILLIC SMALL LETTER YI
};
 
int convert_utf8_to_windows1251(const char* utf8, char* windows1251, size_t n)
{
        int i = 0;
        int j = 0;
        for(; i < (int)n && utf8[i] != 0; ++i) {
                char prefix = utf8[i];
                char suffix = utf8[i+1];
                if ((prefix & 0x80) == 0) {
                        windows1251[j] = (char)prefix;
                        ++j;
                } else if ((~prefix) & 0x20) {
                        int first5bit = prefix & 0x1F;
                        first5bit <<= 6;
                        int sec6bit = suffix & 0x3F;
                        int unicode_char = first5bit + sec6bit;
 
                        if ( unicode_char >= 0x410 && unicode_char <= 0x44F ) {
                                windows1251[j] = (char)(unicode_char - 0x350);
                        } else if (unicode_char >= 0x80 && unicode_char <= 0xFF) {
                                windows1251[j] = (char)(unicode_char);
                        } else if (unicode_char >= 0x402 && unicode_char <= 0x403) {
                                windows1251[j] = (char)(unicode_char - 0x382);
                        } else {
                                int count = sizeof(g_letters) / sizeof(Letter);
                                for (int k = 0; k < count; ++k) {
                                        if (unicode_char == g_letters[k].unicode) {
                                                windows1251[j] = g_letters[k].win1251;
                                                goto NEXT_LETTER;
                                        }
                                }
                                // can't convert this char
                                return 0;
                        }
NEXT_LETTER:
                        ++i;
                        ++j;
                } else {
                        // can't convert this chars
                        return 0;
                }
        }
        windows1251[j] = 0;
        return 1;
}

string to_cp1251(string input) {
	char output[OUTPUT_LINE_MAX] = {0};
	convert_utf8_to_windows1251(input.c_str(), output, input.size());
	return output;
}

void cp1251_to_utf8(char *out, const char *in) {
    static const char table[] = {                 
        "\320\202 \320\203 \342\200\232\321\223 \342\200\236\342\200\246\342\200\240\342\200\241"
        "\342\202\254\342\200\260\320\211 \342\200\271\320\212 \320\214 \320\213 \320\217 "      
        "\321\222 \342\200\230\342\200\231\342\200\234\342\200\235\342\200\242\342\200\223\342\200\224"
        "   \342\204\242\321\231 \342\200\272\321\232 \321\234 \321\233 \321\237 "                     
        "\302\240 \320\216 \321\236 \320\210 \302\244 \322\220 \302\246 \302\247 "                     
        "\320\201 \302\251 \320\204 \302\253 \302\254 \302\255 \302\256 \320\207 "                     
        "\302\260 \302\261 \320\206 \321\226 \322\221 \302\265 \302\266 \302\267 "
        "\321\221 \342\204\226\321\224 \302\273 \321\230 \320\205 \321\225 \321\227 "
        "\320\220 \320\221 \320\222 \320\223 \320\224 \320\225 \320\226 \320\227 "
        "\320\230 \320\231 \320\232 \320\233 \320\234 \320\235 \320\236 \320\237 "
        "\320\240 \320\241 \320\242 \320\243 \320\244 \320\245 \320\246 \320\247 "
        "\320\250 \320\251 \320\252 \320\253 \320\254 \320\255 \320\256 \320\257 "
        "\320\260 \320\261 \320\262 \320\263 \320\264 \320\265 \320\266 \320\267 "
        "\320\270 \320\271 \320\272 \320\273 \320\274 \320\275 \320\276 \320\277 "
        "\321\200 \321\201 \321\202 \321\203 \321\204 \321\205 \321\206 \321\207 "
        "\321\210 \321\211 \321\212 \321\213 \321\214 \321\215 \321\216 \321\217 "
    };
    while (*in)
        if (*in & 0x80) {
            const char *p = &table[3 * (0x7f & *in++)];
            if (*p == ' ')
                continue;
            *out++ = *p++;
            *out++ = *p++;
            if (*p == ' ')
                continue;
            *out++ = *p++;
        }
        else
            *out++ = *in++;
    *out = 0;
}

string to_utf8(string input) {
	string output;
	int c;
    for (unsigned i = 0; i < input.size(); i++) {
        char buf[4], in[2] = {0, 0};
        c = input[i];
        *in = c;
        cp1251_to_utf8(buf, in);
        output += buf;
    }
    return output;
}