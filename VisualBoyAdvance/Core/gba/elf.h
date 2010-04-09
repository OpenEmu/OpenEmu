#ifndef ELF_H
#define ELF_H

enum LocationType {
  LOCATION_register,
  LOCATION_memory,
  LOCATION_value
};

#define DW_ATE_boolean       0x02
#define DW_ATE_signed        0x05
#define DW_ATE_unsigned      0x07
#define DW_ATE_unsigned_char 0x08

struct ELFHeader {
  u32 magic;
  u8 clazz;
  u8 data;
  u8 version;
  u8 pad[9];
  u16 e_type;
  u16 e_machine;
  u32 e_version;
  u32 e_entry;
  u32 e_phoff;
  u32 e_shoff;
  u32 e_flags;
  u16 e_ehsize;
  u16 e_phentsize;
  u16 e_phnum;
  u16 e_shentsize;
  u16 e_shnum;
  u16 e_shstrndx;
};

struct ELFProgramHeader {
  u32 type;
  u32 offset;
  u32 vaddr;
  u32 paddr;
  u32 filesz;
  u32 memsz;
  u32 flags;
  u32 align;
};

struct ELFSectionHeader {
  u32 name;
  u32 type;
  u32 flags;
  u32 addr;
  u32 offset;
  u32 size;
  u32 link;
  u32 info;
  u32 addralign;
  u32 entsize;
};

struct ELFSymbol {
  u32 name;
  u32 value;
  u32 size;
  u8 info;
  u8 other;
  u16 shndx;
};

struct ELFBlock {
  int length;
  u8 *data;
};

struct ELFAttr {
  u32 name;
  u32 form;
  union {
    u32 value;
    char *string;
    u8 *data;
    bool flag;
    ELFBlock *block;
  };
};

struct ELFAbbrev {
  u32 number;
  u32 tag;
  bool hasChildren;
  int numAttrs;
  ELFAttr *attrs;
  ELFAbbrev *next;
};

enum TypeEnum {
  TYPE_base,
  TYPE_pointer,
  TYPE_function,
  TYPE_void,
  TYPE_array,
  TYPE_struct,
  TYPE_reference,
  TYPE_enum,
  TYPE_union
};

struct Type;
struct Object;

struct FunctionType {
  Type *returnType;
  Object *args;
};

struct Member {
  char *name;
  Type *type;
  int bitSize;
  int bitOffset;
  int byteSize;
  ELFBlock *location;
};

struct Struct {
  int memberCount;
  Member *members;
};

struct Array {
  Type *type;
  int maxBounds;
  int *bounds;
};

struct EnumMember {
  char *name;
  u32 value;
};

struct Enum {
  int count;
  EnumMember *members;
};

struct Type {
  u32 offset;
  TypeEnum type;
  const char *name;
  int encoding;
  int size;
  int bitSize;
  union {
    Type *pointer;
    FunctionType *function;
    Array *array;
    Struct *structure;
    Enum *enumeration;
  };
  Type *next;
};

struct Object {
  char *name;
  int file;
  int line;
  bool external;
  Type *type;
  ELFBlock *location;
  u32 startScope;
  u32 endScope;
  Object *next;
};

struct Function {
  char *name;
  u32 lowPC;
  u32 highPC;
  int file;
  int line;
  bool external;
  Type *returnType;
  Object *parameters;
  Object *variables;
  ELFBlock *frameBase;
  Function *next;
};

struct LineInfoItem {
  u32 address;
  char *file;
  int line;
};

struct LineInfo {
  int fileCount;
  char **files;
  int number;
  LineInfoItem *lines;
};

struct ARange {
  u32 lowPC;
  u32 highPC;
};

struct ARanges {
  u32 offset;
  int count;
  ARange *ranges;
};

struct CompileUnit {
  u32 length;
  u8 *top;
  u32 offset;
  ELFAbbrev **abbrevs;
  ARanges *ranges;
  char *name;
  char *compdir;
  u32 lowPC;
  u32 highPC;
  bool hasLineInfo;
  u32 lineInfo;
  LineInfo *lineInfoTable;
  Function *functions;
  Function *lastFunction;
  Object *variables;
  Type *types;
  CompileUnit *next;
};

struct DebugInfo {
  u8 *debugfile;
  u8 *abbrevdata;
  u8 *debugdata;
  u8 *infodata;
  int numRanges;
  ARanges *ranges;
};

struct Symbol {
  const char *name;
  int type;
  int binding;
  u32 address;
  u32 value;
  u32 size;
};

extern u32 elfReadLEB128(u8 *, int *);
extern s32 elfReadSignedLEB128(u8 *, int *);
extern bool elfRead(const char *, int &, FILE *f);
extern bool elfGetSymbolAddress(const char *,u32 *, u32 *, int *);
extern const char *elfGetAddressSymbol(u32);
extern const char *elfGetSymbol(int, u32 *, u32 *, int *);
extern void elfCleanUp();
extern bool elfGetCurrentFunction(u32, Function **, CompileUnit **c);
extern bool elfGetObject(const char *, Function *, CompileUnit *, Object **);
extern bool elfFindLineInUnit(u32 *, CompileUnit *, int);
extern bool elfFindLineInModule(u32 *, const char *, int);
u32 elfDecodeLocation(Function *, ELFBlock *, LocationType *);
u32 elfDecodeLocation(Function *, ELFBlock *, LocationType *, u32);
int elfFindLine(CompileUnit *unit, Function *func, u32 addr, const char **);

#endif // ELF_H
