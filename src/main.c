#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <string.h>

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

typedef struct Section {
    u8 *data;
    u32 start;
    u32 size;
    u32 index;
} Section;

typedef struct SectionHeader {
    u32 offset;
    u32 size;
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
    u32 module_id;
    u32 id;
    u32 code_offset;
    u32 code_length;
    u16 register_count;
    u16 n_param_names;
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
    u32 b, c;
    u16 a;
    u8 flags;
    u8 op;
} Instruction;

typedef enum ExportType {
    EXPORT_TYPE_FUNCTION,
    EXPORT_TYPE_TYPE,
    EXPORT_TYPE_ENUM_END,
} ExportType;

typedef struct Export {
    String name;
    u32 module_id;
    u32 target_index;
    ExportType type;
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
    String config;
    u32 entry_module_id;
    u32 permissions;
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

#ifdef __linux__
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#else
#error unsupported platform
#endif

typedef struct MappedFile {
#ifdef __linux__
    void* memory;
    size_t size;
    int fd;
#endif
} MappedFile;

INLINE MappedFile memory_map_file(const char* path) {
#ifdef __linux__
    MappedFile file = {.fd = open(path, O_RDONLY, 0644)};
    struct stat sb;
    fstat(file.fd, &sb);
    file.size = sb.st_size;
    file.memory = mmap(NULL, file.size, PROT_READ, MAP_SHARED, file.fd, 0);
    return file;
#endif
}

INLINE void memory_unmap_file(MappedFile file) {
#ifdef __linux__
    msync(file.memory, file.size, MS_SYNC);
    munmap(file.memory, file.size);
    close(file.fd);
#endif
}

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
    INLINE int read_##sign##bits(Section *s, sign##bits *value) {\
        if (s->index + sizeof(sign##bits) >= s->size) return 1;\
        if (value != NULL) {\
            memcpy(value, s->data + s->start + s->index, sizeof(sign##bits));\
            *value = swap_u##bits(*value);\
        }\
        s->index += sizeof(sign##bits);\
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

INLINE int parse_header(Section *s, Header *header) {
    u32 start;
    if (!read_u32(s, &start) && start == magic_int()) return 1;
    CHECK(read_u16(s, &header->major));
    CHECK(read_u16(s, &header->minor));
    CHECK(read_u32(s, &header->flags));
    CHECK(read_u16(s, &header->section_count));
    if (header->section_count == 0) return 1;
    CHECK(read_u16(s, NULL));
    return 0;
}

INLINE int parse_section_header(Section *s, SectionHeader *header) {
    CHECK(read_u16(s, &header->id));
    CHECK(read_u16(s, &header->flags));
    CHECK(read_u32(s, &header->offset));
    CHECK(read_u32(s, &header->size));
    if (header->size > 64 * 1024 * 1024) return 1;
    CHECK(read_u32(s, NULL));
    return 0;
}

INLINE int read_string(Section *s, String *string) {
    if (string == NULL) {
        u32 length;
        CHECK(read_u32(s, &length));
        if (s->index + length >= s->size) return 1;
        s->index += length;
    } else {
        CHECK(read_u32(s, &string->length));
        if (s->index + string->length >= s->size) return 1;
        string->data = malloc(string->length * sizeof(char));
        memcpy(string->data, s->data + s->start + s->index, string->length * sizeof(char));
        s->index += string->length;
    }

    return 0;
}

INLINE int get_string(Section *s, String *string) {
    CHECK(read_u32(s, &string->length));
    if (s->index + string->length >= s->size) return 1;
    string->data = (char*)(s->data + s->start + s->index);
    s->index += string->length;
    return 0;
}

INLINE void safe_free(void *ptr) {
    if (ptr != NULL) free(ptr);
}

#define SECTION_PARSER_START(list, type, min_size)\
    program->n_##list = 0, program->list = NULL;\
    CHECK(read_u32(&s, &program->n_##list));\
    if ((program->n_##list * min_size) + s.index > s.size) return 1;\
    program->list = calloc(program->n_##list, sizeof(type));

INLINE int parse_module_table(Section s, Program *program) {
    SECTION_PARSER_START(modules, Module, 16);
    for (u32 i = 0; i < program->n_modules; i++) {
        Module *module = &program->modules[i];
        CHECK(read_u32(&s, &module->id));
        CHECK(read_string(&s, &module->path));
        CHECK(read_u32(&s, &module->init_func));
        CHECK(read_u32(&s, &module->n_types));
        if ((module->n_types * 9) + s.index > s.size) return 1;
        module->types = calloc(module->n_types, sizeof(Type));
        for (u32 j = 0; j < module->n_types; j++) {
            Type *type = &module->types[j];
            CHECK(read_string(&s, &type->type_name));
            CHECK(read_u8(&s, &type->exported));
            CHECK(read_u32(&s, &type->n_fields));
            if ((type->n_fields * 4) + s.index > s.size) return 1;
            type->field_names = calloc(type->n_fields, sizeof(String));
            for (u32 k = 0; k < type->n_fields; k++) CHECK(read_string(&s, &type->field_names[k]));
        }
    }
    if (s.index != s.size - 1) return 1; // possible bug
    return 0;
}

INLINE void free_module_table(Program *program) {
    if (program->constants != NULL) for (u32 i = 0; i < program->n_modules; i++) {
        Module *module = &program->modules[i];
        if (module->types != NULL) for (u32 j = 0; j < module->n_types; j++) {
            Type *type = &module->types[j];
            if (type->field_names != NULL)
                for (u32 k = 0; k < type->n_fields; k++) safe_free(type->field_names[k].data);
            safe_free(type->field_names), safe_free(type->type_name.data);
        }
        safe_free(module->types), safe_free(module->path.data);
    }
    safe_free(program->modules);
}

INLINE int parse_const_pool(Section s, Program *program) {
    SECTION_PARSER_START(constants, Constant, 4);
    for (u32 i = 0; i < program->n_constants; i++) {
        Constant *constant = &program->constants[i];
        u8 kind;
        CHECK(read_u8(&s, &kind));
        if (kind >= CONSTANT_TYPE_ENUM_END) return 1;
        constant->type = kind;
        CHECK(read_u8(&s, NULL));
        CHECK(read_u16(&s, NULL));
        switch (constant->type) {
            case CONSTANT_TYPE_INTEGER:
                CHECK(read_i64(&s, &constant->data.integer));
                break;
            case CONSTANT_TYPE_FLOATING:
                CHECK(read_f64(&s, &constant->data.floating));
                break;
            case CONSTANT_TYPE_STRING:
                CHECK(read_string(&s, &constant->data.string));
                break;
            case CONSTANT_TYPE_BOOLEAN:
                u8 temp;
                CHECK(read_u8(&s, &temp));
                constant->data.boolean = temp == 1;
                break;
            default: break;
        }
    }
    if (s.index != s.size - 1) return 1;
    return 0;
}

INLINE void free_const_pool(Program *program) {
    if (program->constants != NULL) for (u32 i = 0; i < program->n_constants; i++) {
        Constant *constant = &program->constants[i];
        if (constant->type == CONSTANT_TYPE_STRING) safe_free(constant->data.string.data);
    }
    safe_free(program->constants);
}

INLINE int parse_function_table(Section s, Program *program) {
    SECTION_PARSER_START(functions, Function, 24);
    for (u32 i = 0; i < program->n_functions; i++) {
        Function *function = &program->functions[i];
        CHECK(read_u32(&s, &function->id));
        CHECK(read_u32(&s, &function->module_id));
        CHECK(read_string(&s, &function->name));
        CHECK(read_u16(&s, &function->register_count));
        CHECK(read_u16(&s, &function->n_param_names));
        CHECK(read_u32(&s, &function->code_offset));
        CHECK(read_u32(&s, &function->code_length));
        if ((function->n_param_names * 4) + s.index > s.size) return 1;
        function->param_names = calloc(function->n_param_names, sizeof(String));
        for (u32 j = 0; j < function->n_param_names; j++) CHECK(read_string(&s, &function->param_names[j]));
    }
    if (s.index != s.size - 1) return 1;
    return 0;
}

INLINE void free_function_table(Program *program) {
    for (u32 i = 0; i < program->n_functions; i++) {
        Function *function = &program->functions[i];
        for (u32 j = 0; j < function->n_param_names; j++) safe_free(function->param_names[j].data);
        safe_free(function->param_names);
    }
    safe_free(program->functions);
}

INLINE int parse_code_section(Section s, Program *program) {
    SECTION_PARSER_START(code, Instruction, 12);
    for (u32 i = 0; i < program->n_code; i++) {
        Instruction *instruction = &program->code[i];
        CHECK(read_u8(&s, &instruction->op));
        CHECK(read_u8(&s, &instruction->flags));
        CHECK(read_u16(&s, &instruction->a));
        CHECK(read_u32(&s, &instruction->b));
        CHECK(read_u32(&s, &instruction->c));
    }
    if (s.index != s.size - 1) return 1;
    return 0;
}

INLINE int parse_export_table(Section s, Program *program) {
    SECTION_PARSER_START(exports, Export, 16);
    for (u32 i = 0; i < program->n_exports; i++) {
        Export *export = &program->exports[i];
        CHECK(read_u32(&s, &export->module_id));
        u8 temp;
        CHECK(read_u8(&s, &temp));
        if (temp >= EXPORT_TYPE_ENUM_END) return 1;
        export->type = temp;
        CHECK(read_u8(&s, NULL));
        CHECK(read_u16(&s, NULL));
        CHECK(read_string(&s, &export->name));
        CHECK(read_u32(&s, &export->target_index));
    }
    if (s.index != s.size - 1) return 1;
    return 0;
}

INLINE void free_export_table(Program *program) {
    for (u32 i = 0; i < program->n_exports; i++) safe_free(program->exports[i].name.data);
    safe_free(program->exports);
}

INLINE int parse_import_table(Section s, Program *program) {
    SECTION_PARSER_START(imports, Import, 12);
    for (u32 i = 0; i < program->n_imports; i++) {
        Import *import = &program->imports[i];
        CHECK(read_u32(&s, &import->module_id));
        CHECK(read_u32(&s, &import->target_module_id));
        CHECK(read_string(&s, &import->alias));
    }
    if (s.index != s.size - 1) return 1;
    return 0;
}

INLINE void free_import_table(Program *program) {
    for (u32 i = 0; i < program->n_imports; i++) safe_free(program->imports[i].alias.data);
    safe_free(program->imports);
}

INLINE bool string_equal(String lhs, const char *rhs) {
    if (lhs.length != strlen(rhs)) return false;
    if (memcmp(lhs.data, rhs, lhs.length)) return false;
    return true;
}

INLINE int parse_metadata(Section s, Program *program) {
    CHECK(read_u32(&s, &program->metadata.entry_module_id));
    program->metadata.permissions = 0;
    for (u32 i = 0; i < 5; i++) {
        String string;
        CHECK(get_string(&s, &string));
        if (string_equal(string, "allowed")) program->metadata.permissions |= 1 << i;
        else if (!string_equal(string, "blocked")) return 1;
    }
    CHECK(read_string(&s, &program->metadata.config));
    if (s.index != s.size - 1) return 1;
    return 0;
}

int parse_container(MappedFile file) {
    Program program;
    Section s = {.data = file.memory, .size = file.size};

    Header header;
    CHECK(parse_header(&s, &header));

    Section *required[SECTION_ID_ENUM_END] = {0};
    Section *sections = malloc(header.section_count * sizeof(SectionHeader));
    u32 n_sections = 0;

    for (u32 i = 0; i < header.section_count; i++) {
        SectionHeader section;
        if (parse_section_header(&s, &section)) goto error1;
        if (section.offset + section.size >= s.size) goto error1;
        if (section.offset < 16 + (header.section_count * 20)) goto error1;
        for (u32 j = 0; j < n_sections; j++)
            if (section.offset <= sections[j].start + sections[j].size &&
                section.offset + section.size >= sections[j].start) goto error1;
        if (section.id >= SECTION_ID_ENUM_END) {
            if (section.flags & SECTION_FLAG_REQUIRED) goto error1;
            if (!(section.flags & SECTION_FLAG_OPTIONAL)) goto error1;
        } else {
            if (required[section.id] != NULL) goto error1;
            required[section.id] = &sections[n_sections];
        }
        sections[n_sections++] = (Section){.data = s.data, .start = section.offset, .size = section.size};
    }

    for (u32 i = 0; i < SECTION_ID_ENUM_END; i++) if (required[i] == NULL) goto error1;

    if (parse_module_table(*required[SECTION_ID_MODULE_TABLE], &program)) goto error2;
    if (parse_const_pool(*required[SECTION_ID_CONST_POOL], &program)) goto error3;
    if (parse_function_table(*required[SECTION_ID_FUNCTION_TABLE], &program)) goto error4;
    if (parse_code_section(*required[SECTION_ID_CODE_SECTION], &program)) goto error5;
    if (parse_export_table(*required[SECTION_ID_EXPORT_TABLE], &program)) goto error6;
    if (parse_import_table(*required[SECTION_ID_IMPORT_TABLE], &program)) goto error7;
    if (parse_metadata(*required[SECTION_ID_METADATA], &program)) goto error8;

    return 0;

    error8: safe_free(program.metadata.config.data);
    error7: free_import_table(&program);
    error6: free_export_table(&program);
    error5: safe_free(program.code);
    error4: free_function_table(&program);
    error3: free_const_pool(&program);
    error2: free_module_table(&program);
    error1: free(sections);
    return 1;
}

int main(int argc, const char** argv) {
    if (argc != 2) return 1;
    MappedFile file = memory_map_file(argv[1]);
    if (file.memory == NULL) return 1;
    if (parse_container(file)) goto error;
    memory_unmap_file(file);

    return 0;

    error: memory_unmap_file(file);
    return 1;
}
