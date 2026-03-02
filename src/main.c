#include <stdint.h>
#include <stdbool.h>

typedef int64_t i64;
typedef uint64_t u64;
typedef double f64;

typedef struct String {
    char *data;
    u64 length;
} String;

typedef struct Value Value;

typedef union ResultData {
    Value *ok;
    String *err;
} ResultData;

typedef enum ResultType {
    RESULT_TYPE_OK,
    RESULT_TYPE_ERR,
} ResultType;

typedef struct Result {
    ResultData data;
    ResultType type;
} Result;

typedef struct Record {

} Record;

typedef struct StringList {
    String *strings;
    u64 n_strings;
} StringList;

typedef struct 

typedef union ValueData {
    i64 integer;
    f64 floating;
    String string;
    bool boolean;
    Result result;
    Record record;
    StringList string_list;

} ValueData;