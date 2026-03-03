#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <memory.h>
#include <stdlib.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef int64_t i64;
typedef double f64;

typedef enum PrimitiveType {
    PRIMITIVE_TYPE_U8,
    PRIMITIVE_TYPE_U16,
    PRIMITIVE_TYPE_U32,
    PRIMITIVE_TYPE_U64,
    PRIMITIVE_TYPE_I64,
    PRIMITIVE_TYPE_F64,
} PrimitiveType;

static const char magic[4] = "BZC2";

typedef struct Header {
    u32 flags;
    u16 major;
    u16 minor;
    u16 section_count;
} Header;

typedef struct SectionHeader {
    u32 offset;
    u32 size;
    u32 reserved;
    u16 flags;
    u16 id;
} SectionHeader;

#define SECTION_FLAG_REQUIRED 1
#define SECTION_FLAG_OPTIONAL 2

typedef enum SectionID {
    SECTION_ID_MODULE_TABLE,
    SECTION_ID_CONST_POOL,
    SECTION_ID_FUNCTION_TABLE,
    SECTION_ID_CODE_SECTION,
    SECTION_ID_EXPORT_TABLE,
    SECTION_ID_IMPORT_TABLE,
    SECTION_ID_METADATA,
    SECTION_ID_ENUM_END,
} SectionID;

typedef struct String {
    char *data;
    u32 length;
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

typedef struct ListString {
    String *strings;
    u64 n_strings;
} ListString;

typedef struct Function {
    String* param_names;
    String name;
    u32 n_param_names;
    u32 module_id;
    u32 id;
    u32 code_offset;
    u32 code_length;
    u16 register_count;
} Function;

typedef struct BuiltIn {

} BuiltIn;

typedef struct ModuleNamespace {

} ModuleNamespace;

typedef struct TypeRef {

} TypeRef;

typedef union ValueData {
    i64 integer;
    f64 floating;
    String string;
    bool boolean;
    Result result;
    Record record;
    ListString list_string;
    Function function;
    BuiltIn builtin;
    ModuleNamespace module_namespace;
    TypeRef type_ref;
} ValueData;

typedef struct Type {
    String* field_names;
    String type_name;
    u32 n_fields;
    u8 exported;
} Type;

typedef struct Module {
    String path;
    Type *types;
    u32 n_types;
    u32 init_func;
    u32 id;
} Module;

typedef struct TypeDef {
    String name;
    String *field_names;
    u32 n_field_names;
    bool exported;
} TypeDef;

typedef enum ConstantType {
    CONSTANT_TYPE_NULL,
    CONSTANT_TYPE_INTEGER,
    CONSTANT_TYPE_FLOATING,
    CONSTANT_TYPE_STRING,
    CONSTANT_TYPE_BOOLEAN,
    CONSTANT_TYPE_ENUM_END,
} ConstantType;

typedef union ConstantData {
    i64 integer;
    f64 floating;
    String string;
    bool boolean;
} ConstantData;

typedef struct Constant {
    ConstantData data;
    ConstantType type;
} Constant;

typedef struct Instruction {

} Instruction;

typedef struct Export {
    String name;
    u32 module_id;
    u32 target_index;
    u8 kind;
} Export;

typedef struct Import {
    String alias;
    u32 module_id;
    u32 target_module_id;
} Import;

#define PERMISSION_FLAG_READ 1
#define PERMISSION_FLAG_WRITE 2
#define PERMISSION_FLAG_NETWORK 4
#define PERMISSION_FLAG_PROCESS 8
#define PERMISSION_FLAG_GIT 16

typedef struct Metadata {
    String project_config_path;
    u32 entry_module_id;
    u32 permission_defaults;
} Metadata;

typedef struct Program {
    Module *modules;
    Constant *constants;
    Function *functions;
    Instruction *code;
    Export *exports;
    Import *imports;
    Metadata metadata;
    u32 n_modules, n_constants;
    u32 n_functions, n_code;
    u32 n_exports, n_imports;
    u32 flags;
} Program;

#define INSTRUCTION_TYPE_MACRO\
    X(NOP) X(LOAD_CONST) X(MOVE) X(LOAD_NAME) X(STORE_NAME) X(DEFINE_FUNC)\
    X(DEFINE_TYPE) X(IMPORT_ALIAS) X(MODULE_SYMBOL) X(ADD) X(SUB) X(MUL) X(DIV)\
    X(MOD) X(EQ) X(NE) X(LT) X(LE) X(GT) X(GE) X(NEG) X(NOT) X(JUMP) X(JUMP_IF_FALSE)\
    X(CALL) X(CALL_METHOD) X(SAY) X(RETURN) X(ENTER_SCOPE) X(EXIT_SCOPE) X(SET_LAST)

#define X(name) INSTRUCTION_TYPE_##name,
typedef enum InstructionType {
    INSTRUCTION_TYPE_MACRO
} InstructionType;
#undef X

#if defined(_MSC_VER)
#define INLINE __forceinline
#else
#define INLINE static inline __attribute((always_inline))
#endif

#if defined(__GNUC__) || defined(__clang__)
#define UNLIKELY(expr) __builtin_expect(!!(expr), 0)
#define LIKELY(expr)   __builtin_expect(!!(expr), 1)
#else
#define UNLIKELY(expr) (expr)
#define LIKELY(expr)   (expr)
#endif

INLINE bool is_little_endian() {
    unsigned int i = 1;
    char *c = (char *)&i;

    return *c == 1;
}

#define MAKE_SWAP_NONSPECIFIC(type, method)\
    INLINE type swap_##type(type v) {\
        if (!is_little_endian()) return v;\
        return method;\
    }

#if defined(__GNUC__) || defined(__clang__)
#define MAKE_SWAP_FUNCTION(type, gnu, msvc, fallback) MAKE_SWAP_NONSPECIFIC(type, gnu)
#elif defined(_MSC_VER)
#define MAKE_SWAP_FUNCTION(type, gnu, msvc, fallback) MAKE_SWAP_NONSPECIFIC(type, msvc)
#else
#define MAKE_SWAP_FUNCTION(type, gnu, msvc, fallback) MAKE_SWAP_NONSPECIFIC(type, fallback)
#endif

MAKE_SWAP_FUNCTION(u16, __builtin_bswap16(v), _byteswap_ushort(v), (v >> 8) | (v << 8));

MAKE_SWAP_FUNCTION(u32, __builtin_bswap32(v), _byteswap_ulong(v),
    ((v & 0xFF000000) >> 24) |
    ((v & 0x00FF0000) >> 8) |
    ((v & 0x0000FF00) << 8) |
    ((v & 0x000000FF) << 24));

MAKE_SWAP_FUNCTION(u64, __builtin_bswap64(v), _byteswap_uint64(v),
    ((v & 0xFF00000000000000) >> 56) |
    ((v & 0x00FF000000000000) >> 40) |
    ((v & 0x0000FF0000000000) >> 24) |
    ((v & 0x000000FF00000000) >> 8) |
    ((v & 0x00000000FF000000) << 8) |
    ((v & 0x0000000000FF0000) << 24) |
    ((v & 0x000000000000FF00) << 40) |
    ((v & 0x00000000000000FF) << 56));

#define swap_u8(v) v
#define MAKE_VALUE_READER(sign, bits)\
    INLINE int read_##sign##bits(FILE *file, sign##bits *value) {\
        if (value == NULL) {\
            sign##bits fake;\
            if (fread(&fake, sizeof(sign##bits), 1, file) != sizeof(sign##bits)) return 1;\
        } else if (fread(value, sizeof(sign##bits), 1, file) != sizeof(sign##bits)) return 1;\
        return 0;\
    }

MAKE_VALUE_READER(u, 8);
MAKE_VALUE_READER(u, 16);
MAKE_VALUE_READER(u, 32);
MAKE_VALUE_READER(u, 64);

MAKE_VALUE_READER(i, 8);
MAKE_VALUE_READER(i, 16);
MAKE_VALUE_READER(i, 32);
MAKE_VALUE_READER(i, 64);

MAKE_VALUE_READER(f, 64);

INLINE u32 magic_int() {
    u32 result;
    memcpy(&result, magic, sizeof(u32));
    return result;
}

#define CHECK(code)\
    {int _code = code;\
    if (code != 0) return code;}

INLINE int parse_header(FILE *file, Header *header) {
    u32 start;
    if (!read_u32(file, &start) && start == magic_int()) return 1;
    CHECK(read_u16(file, &header->major));
    CHECK(read_u16(file, &header->minor));
    CHECK(read_u32(file, &header->flags));
    CHECK(read_u16(file, &header->section_count));
    if (header->section_count == 0) return 1;
    CHECK(read_u16(file, NULL));
    return 0;
}

INLINE int parse_section_header(FILE *file, SectionHeader *header) {
    CHECK(read_u16(file, &header->id));
    CHECK(read_u16(file, &header->flags));
    CHECK(read_u32(file, &header->offset));
    CHECK(read_u32(file, &header->size));
    if (header->size > 64 * 1024 * 1024) return 1;
    CHECK(read_u32(file, NULL));
    return 0;
}

int read_string(FILE* file, String *string) {
    CHECK(read_u32(file, &string->length));
    string->data = malloc(string->length * sizeof(char));
    if (fread(string->data, string->length * sizeof(char), 1, file) != string->length * sizeof(char)) {
        free(string->data);
        return 1;
    }
    return 0;
}

INLINE void safe_free(void *ptr) {
    if (ptr != NULL) free(ptr);
}

#define SECTION_PARSER_START(list, type)\
    program->n_##list = 0, program->list = NULL;\
    fseek(file, section.offset, SEEK_SET);\
    CHECK(read_u32(file, &program->n_##list));\
    program->list = calloc(program->n_##list, sizeof(type));

INLINE int parse_module_table(FILE *file, SectionHeader section, Program *program) {
    SECTION_PARSER_START(modules, Module);
    for (u32 i = 0; i < program->n_modules; i++) {
        Module module;
        CHECK(read_u32(file, &module.id));
        CHECK(read_string(file, &module.path));
        CHECK(read_u32(file, &module.init_func));
        CHECK(read_u32(file, &module.n_types));
        module.types = calloc(module.n_types, sizeof(Type));
        for (u32 j = 0; j < module.n_types; j++) {
            Type type;
            CHECK(read_string(file, &type.type_name));
            CHECK(read_u8(file, &type.exported));
            CHECK(read_u32(file, &type.n_fields));
            type.field_names = calloc(type.n_fields, sizeof(String));
            for (u32 k = 0; k < type.n_fields; k++) CHECK(read_string(file, &type.field_names[k]));
            module.types[j] = type;
        }
        program->modules[i] = module;
    }
    if (ftell(file) != section.offset + section.size - 1) return 1; // possible bug
    return 0;
}

INLINE void free_module_table(Program *program) {
    for (u32 i = 0; i < program->n_modules; i++) {
        Module *module = &program->modules[i];
        for (u32 j = 0; j < module->n_types; j++) {
            Type *type = &module->types[j];
            for (u32 k = 0; k < type->n_fields; k++) safe_free(type->field_names[k].data);
            safe_free(type->field_names), safe_free(type->type_name.data);
        }
        safe_free(module->types), safe_free(module->path.data);
    }
    safe_free(program->modules);
}

INLINE int parse_const_pool(FILE *file, SectionHeader section, Program *program) {
    SECTION_PARSER_START(constants, Constant);
    for (u32 i = 0; i < program->n_constants; i++) {
        Constant constant;
        u8 kind;
        CHECK(read_u8(file, &kind));
        if (kind >= CONSTANT_TYPE_ENUM_END) return 1;
        constant.type = kind;
        CHECK(read_u8(file, NULL));
        CHECK(read_u16(file, NULL));
        switch (constant.type) {
            case CONSTANT_TYPE_INTEGER:
                CHECK(read_i64(file, &constant.data.integer));
                break;
            case CONSTANT_TYPE_FLOATING:
                CHECK(read_f64(file, &constant.data.floating));
                break;
            case CONSTANT_TYPE_STRING:
                CHECK(read_string(file, &constant.data.string));
                break;
            case CONSTANT_TYPE_BOOLEAN:
                u8 temp;
                CHECK(read_u8(file, &temp));
                constant.data.boolean = temp != 0;
                break;
            default: break;
        }
        program->constants[i] = constant;
    }
    if (ftell(file) != section.offset + section.size - 1) return 1; // possible bug
    return 0;
}

int parse_container(FILE* file) {
    Program program;

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    Header header;
    CHECK(parse_header(file, &header));

    SectionHeader *required[SECTION_ID_ENUM_END] = {0};
    SectionHeader *sections = malloc(header.section_count * sizeof(SectionHeader));
    u32 n_sections = 0;

    for (u32 i = 0; i < header.section_count; i++) {
        SectionHeader section;
        if (parse_section_header(file, &section) != 0) goto error1;
        if (section.offset + section.size >= size) goto error1;
        if (section.offset < 16 + (header.section_count * 20)) goto error1;
        for (u32 j = 0; j < n_sections; j++)
            if (section.offset <= sections[j].offset + sections[j].size &&
                section.offset + section.size >= sections[j].offset) goto error1;
        if (section.id >= SECTION_ID_ENUM_END) {
            if (section.flags & SECTION_FLAG_REQUIRED) goto error1;
            if (!(section.flags & SECTION_FLAG_OPTIONAL)) goto error1;
        } else {
            if (required[section.id] != NULL) goto error1;
            required[section.id] = &sections[n_sections];
        }
        sections[n_sections++] = section;
    }

    for (u32 i = 0; i < SECTION_ID_ENUM_END; i++) if (required[i] == NULL) goto error1;

    if (parse_module_table(file, *required[SECTION_ID_MODULE_TABLE], &program)) goto error2;

    return 0;

    error2: free_module_table(&program);
    error1: free(sections);
    return 1;
}

int main(int argc, const char** argv) {
    if (argc != 2) return 1;
    FILE* file = fopen(argv[1], "rwb");
    if (file == NULL) return 1;
    if (parse_container(file) != 0) goto error;
    fclose(file);

    return 0;

    error: fclose(file);
    return 1;
}
