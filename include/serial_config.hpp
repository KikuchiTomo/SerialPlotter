#ifndef S_SERIAL_CONFIG_HPP__
#define S_SERIAL_CONFIG_HPP__

namespace Serial {
enum DataLength { Len5, Len6, Len7, Len8 };
enum Parity { None, Odd, Even };
enum StopBit { One, Two };

typedef struct {
    DataLength len;
    Parity parity;
    StopBit stop;
} format_t;

enum Format {
    S8N1,
    S8N2,
    S8O1,
    S8O2,
    S8E1,
    S8E2,
    S7N1,
    S7N2,
    S7O1,
    S7O2,
    S7E1,
    S7E2,
    S6N1,
    S6N2,
    S6O1,
    S6O2,
    S6E1,
    S6E2,
    S5N1,
    S5N2,
    S5O1,
    S5O2,
    S5E1,
    S5E2
};

format_t FormatType(Format __format) {
    switch (__format) {
    case S8N1:
        return {Len8, None, One};
    case S8N2:
        return {Len8, None, Two};
    case S8O1:
        return {Len8, Odd, One};
    case S8O2:
        return {Len8, Odd, Two};
    case S8E1:
        return {Len8, Even, One};
    case S8E2:
        return {Len8, Even, Two};
    case S7N1:
        return {Len7, None, One};
    case S7N2:
        return {Len7, None, Two};
    case S7O1:
        return {Len7, Odd, One};
    case S7O2:
        return {Len7, Odd, Two};
    case S7E1:
        return {Len7, Even, One};
    case S7E2:
        return {Len7, Even, Two};
    case S6N1:
        return {Len6, None, One};
    case S6N2:
        return {Len6, None, Two};
    case S6O1:
        return {Len6, Odd, One};
    case S6O2:
        return {Len6, Odd, Two};
    case S6E1:
        return {Len6, Even, One};
    case S6E2:
        return {Len6, Even, Two};
    case S5N1:
        return {Len5, None, One};
    case S5N2:
        return {Len5, None, Two};
    case S5O1:
        return {Len5, Odd, One};
    case S5O2:
        return {Len5, Odd, Two};
    case S5E1:
        return {Len5, Even, One};
    case S5E2:
        return {Len5, Even, Two};
    }
}
};  // namespace Serial

#endif