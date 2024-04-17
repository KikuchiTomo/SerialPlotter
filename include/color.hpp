#ifndef S_COLOR_HPP__
#define S_COLOR_HPP__

#include <stdio.h>
#include <stdlib.h>

namespace Serial {
    namespace Appearance {
        class TextStyle {
           private:
            char *sequence_;
           public:
            TextStyle(const char* __sequence) {
                sequence_ = (char*)malloc(sizeof(char) * 32); 
                snprintf(sequence_, 8,"%s", __sequence);
            }

            ~TextStyle() { free(sequence_); }

            void set(FILE* __fp = stdout){ fprintf(__fp, "%s", sequence_); }
        };

        namespace ForegroundColor {
            static TextStyle Red("\x1b[31m");
            static TextStyle Blue("\x1b[34m");
            static TextStyle Green("\x1b[32m");
            static TextStyle Yellow("\x1b[33m");
            static TextStyle Magenta("\x1b[35m");
            static TextStyle Cyan("\x1b[36m");
            static TextStyle Default("\x1b[39m");
        }; // namespace ForegroundColor

        namespace BackgroundColor {
            static TextStyle Red("\x1b[41m");
            static TextStyle Blue("\x1b[44m");
            static TextStyle Green("\x1b[42m");
            static TextStyle Yellow("\x1b[43m");
            static TextStyle Magenta("\x1b[45m");
            static TextStyle Cyan("\x1b[46m");
            static TextStyle Default("\x1b[49m");
        };  // namespace BackgroundColor

        namespace Decoration {
            static TextStyle Underline("\x1b[4m");
            static TextStyle Bold("\x1b[1m");
            static TextStyle Reverse("\x1b[7m");
            static TextStyle Default("\x1b[0m");
        }; // namespace Decoration
    }; // namespace Appearance
}; // namespace Serial
#endif