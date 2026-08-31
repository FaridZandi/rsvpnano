#include "text/RsvpTokenizer.h"

#include "text/UnicodeText.h"
#include "text/Utf8Text.h"

namespace RsvpText {

    namespace Detail {

        bool isWordBoundary(char c) {
            return static_cast<uint8_t>(c) <= ' ';
        }

        bool dashAt(std::string_view text, size_t index, uint32_t& codepoint, size_t& length) {
            if (index >= text.length()) {
                return false;
            }
            std::string_view remaining = text.substr(index);
            const size_t available = remaining.size();
            if (!Utf8Text::next(remaining, codepoint) || !UnicodeText::isDash(codepoint)) {
                return false;
            }
            length = available - remaining.size();
            return length > 0;
        }

        bool isInlineWordDash(std::string_view text, size_t index, size_t length) {
            if (index == 0 || index + length >= text.length()) {
                return false;
            }
            std::string_view before = text.substr(0, index);
            before.remove_prefix(Utf8Text::lastCodepointStart(before));
            std::string_view after = text.substr(index + length);
            uint32_t previous = 0;
            uint32_t next = 0;
            if (!Utf8Text::next(before, previous) || !Utf8Text::next(after, next)) {
                return false;
            }
            // A neighbouring dash means this one ends a run, not a compound.
            if (UnicodeText::isDash(previous) || UnicodeText::isDash(next)) {
                return false;
            }
            return UnicodeText::isWordCharacter(previous) && UnicodeText::isWordCharacter(next);
        }

        size_t closingPunctuationEnd(std::string_view text, size_t index) {
            while (index < text.length()) {
                std::string_view remaining = text.substr(index);
                const size_t available = remaining.size();
                uint32_t codepoint = 0;
                if (!Utf8Text::next(remaining, codepoint) || !UnicodeText::isClosingPunctuation(codepoint)) {
                    break;
                }
                index += available - remaining.size();
            }
            return index;
        }

        bool isDashToken(std::string_view token) {
            uint32_t codepoint = 0;
            if (!Utf8Text::next(token, codepoint) || !UnicodeText::isDash(codepoint)) {
                return false;
            }
            while (Utf8Text::next(token, codepoint)) {
                if (!UnicodeText::isClosingPunctuation(codepoint)) {
                    return false;
                }
            }
            return true;
        }

        bool endsCjkPhrase(uint32_t codepoint) {
            switch (codepoint) {
            case ',':
            case ';':
            case ':':
            case '!':
            case '?':
            case 0x3001U: // IDEOGRAPHIC COMMA
            case 0x3002U: // IDEOGRAPHIC FULL STOP
            case 0xFF01U: // FULLWIDTH EXCLAMATION MARK
            case 0xFF0CU: // FULLWIDTH COMMA
            case 0xFF1AU: // FULLWIDTH COLON
            case 0xFF1BU: // FULLWIDTH SEMICOLON
            case 0xFF1FU: // FULLWIDTH QUESTION MARK
                return true;
            default:
                return false;
            }
        }

    } // namespace Detail

    bool hasReadableText(std::string_view text) {
        uint32_t codepoint = 0;
        while (Utf8Text::next(text, codepoint)) {
            if (UnicodeText::isWordCharacter(codepoint))
                return true;
        }
        return false;
    }

} // namespace RsvpText
