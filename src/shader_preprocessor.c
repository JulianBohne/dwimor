#include "shader_preprocessor.h"

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <assert.h>

typedef struct {
    char* text;
    size_t text_length;
    size_t cursor;
    bool is_eof;
} TextIterator;

#ifndef MIN
#define MIN(a, b) (a < b ? a : b)
#endif // MIN

#ifndef MAX
#define MAX(a, b) (a > b ? a : b)
#endif // MIN

// Eat n characters
void TextIterator_ChompN(TextIterator* it, size_t n) {
    it->cursor = MIN(it->cursor + n, it->text_length);
    it->is_eof = it->cursor == it->text_length;
}

// Returns the amount of whitespace characters skipped
size_t TextIterator_SkipAfterWhitespace(TextIterator* it) {
    size_t n = 0;
    while (!it->is_eof && isspace(it->text[it->cursor])) {
        TextIterator_ChompN(it, 1);
        ++n;
    }
    return n;
}

bool TextIterator_HasNext(TextIterator* it, const char* string) {
    size_t string_length = strlen(string);

    if (it->text_length - it->cursor < string_length) return false;

    return strncmp(&it->text[it->cursor], string, string_length) == 0;
}

void TextIterator_SkipLine(TextIterator* it) {
    while (!it->is_eof && it->text[it->cursor] != '\n') {
        TextIterator_ChompN(it, 1);
    }
    TextIterator_ChompN(it, 1); // Remove potential new line
}

// Skip comments and whitespace
void TextIterator_SkipAfterWhitespaceAndComments(TextIterator* it) {
    for (;;) {
        TextIterator_SkipAfterWhitespace(it);
        if (TextIterator_HasNext(it, "//")) {
            TextIterator_SkipLine(it);
            continue;
        }
        if (TextIterator_HasNext(it, "/*")) {
            TextIterator_ChompN(it, 2); // Remove "/*"
            while (!it->is_eof && !TextIterator_HasNext(it, "*/")) TextIterator_ChompN(it, 1);
            TextIterator_ChompN(it, 2); // Remove potential "*/"
            continue;
        }
        break;
    }
}

// Returns true if match was found
// Only searches in the text, not comments
// Should only be used with small strings
// Important: search_string can't start with whitespace and can't contain "//" "/*" or "*/"
bool TextIterator_SkipUntilTextMatch(TextIterator* it, const char* search_string) {
    size_t search_string_length = strlen(search_string);

    
    while (!it->is_eof && !TextIterator_HasNext(it, search_string)) {
        TextIterator_SkipAfterWhitespaceAndComments(it);

        if (TextIterator_HasNext(it, search_string)) break;

        TextIterator_ChompN(it, 1);
    }

    if (it->is_eof) return false;
    else return true;
}

static inline size_t next_pow2(size_t x) {
    return x == 1 ? 1 : 1 << (64 - __builtin_clzll(x - 1));
}

typedef struct {
    char* string;  // '\0' terminated string
    size_t length; // Length without '\0' terminator
    size_t capacity;
} StringBuilder;

void StringBuilder_AppendSized(StringBuilder* sb, const char* string, size_t length) {
    size_t combined_length_with_null_terminator = sb->length + length + 1;
    if (sb->length + length + 1 > sb->capacity) {
        sb->string = RL_REALLOC(sb->string, MAX(next_pow2(combined_length_with_null_terminator), 128));
        assert(sb->string != NULL && "https://www.amazon.com/Corsair-VENGEANCE-3200MHz-Compatible-Computer/dp/B07RW6Z692");
    }
    memcpy(sb->string + sb->length, string, length);
    sb->length = combined_length_with_null_terminator - 1;
    sb->string[sb->length] = '\0';
}

void StringBuilder_AppendNullTerminated(StringBuilder* sb, const char* string) {
    size_t length = strlen(string);
    StringBuilder_AppendSized(sb, string, length);
}

// Crop string from the right until the length is met (length has to be smaller or equal to sb->length)
void StringBuilder_Crop(StringBuilder* sb, size_t length) {
    assert(sb->length >= length && "Cannot crop a string to a larger size");
    sb->length = length;
    sb->string[length] = '\0';
}

// Note: File paths should use forward slashes
bool LoadAndPreprocessFileText_(StringBuilder* sb, char* file_name) {
    StringBuilder_AppendNullTerminated(sb, "\n// #################### BEGIN ");
    StringBuilder_AppendNullTerminated(sb, file_name);
    StringBuilder_AppendNullTerminated(sb, " ####################\n");

    char* unprocessed = LoadFileText(file_name);

    if (unprocessed == NULL) {
        TraceLog(LOG_ERROR, "Could not load text file \"%s\"", file_name);
        return false;
    }

    size_t unprocessed_length = strlen(unprocessed);

    size_t directory_length_with_slash;
    StringBuilder included_file_path = { 0 };
    {
        // Directory without '/' at the end
        const char* static_directory = GetDirectoryPath(file_name);
        size_t directory_length = strlen(static_directory);
        StringBuilder_AppendSized(&included_file_path, static_directory, directory_length);
    }
    StringBuilder_AppendSized(&included_file_path, "/", 1);
    directory_length_with_slash = included_file_path.length;

    
    TextIterator it = {
        .text = unprocessed,
        .text_length = unprocessed_length,
        .is_eof = unprocessed_length == 0
    };
    
    while (!it.is_eof) {
        size_t start_cursor = it.cursor;

        StringBuilder_Crop(&included_file_path, directory_length_with_slash);

        if (TextIterator_SkipUntilTextMatch(&it, "#include")) {
            StringBuilder_AppendSized(sb, &it.text[start_cursor], it.cursor - start_cursor);

            TextIterator_ChompN(&it, strlen("#include"));

            TextIterator_SkipAfterWhitespace(&it);
            if (it.is_eof) {
                TraceLog(LOG_ERROR, "Unexpected EOF in \"%s\" after #include", file_name);
                UnloadFileText(unprocessed);
                return false;
            }
            if (it.text[it.cursor] != '"') {
                TraceLog(LOG_ERROR, "Expected '\"' after #include in \"%s\", but got %c", file_name, it.text[it.cursor]);
                UnloadFileText(unprocessed);
                return false;
            }
            TextIterator_ChompN(&it, 1);
            if (it.is_eof) {
                TraceLog(LOG_ERROR, "Unexpected EOF in \"%s\" after #include \"", file_name);
                UnloadFileText(unprocessed);
                return false;
            }
            size_t start_include_name = it.cursor;
            if (!TextIterator_SkipUntilTextMatch(&it, "\"")) {
                TraceLog(LOG_ERROR, "Unexpected EOF in \"%s\" after #include \"...", file_name);
                UnloadFileText(unprocessed);
                return false;
            }
            StringBuilder_AppendSized(&included_file_path, &it.text[start_include_name], it.cursor - start_include_name);
            
            TextIterator_ChompN(&it, 1);
            
            if (!LoadAndPreprocessFileText_(sb, included_file_path.string)) {
                UnloadFileText(unprocessed);
                return false;
            }

        } else {
            StringBuilder_AppendSized(sb, &it.text[start_cursor], it.cursor - start_cursor);
        }
    }

    UnloadFileText(unprocessed);

    StringBuilder_AppendSized(sb, "\n", 1);
    StringBuilder_AppendNullTerminated(sb, "// #################### END ");
    StringBuilder_AppendNullTerminated(sb, file_name);
    StringBuilder_AppendNullTerminated(sb, " ####################\n");

    return true;
}

// Note: File paths should use forward slashes
char* LoadAndPreprocessFileText(char* file_name) {
    StringBuilder sb = { 0 };
    if (!LoadAndPreprocessFileText_(&sb, file_name)) return NULL;
    return sb.string;
}

// Load a shader and resolve #include directives
// Note: File paths should use forward slashes
Shader LoadAndPreProcessFragmentShader(char* file_name) {
    Shader shader = { 0 };

    char* shader_str = LoadAndPreprocessFileText(file_name);

    if (shader_str == NULL) {
        TraceLog(LOG_WARNING, "SHADER: Shader file provided are not valid, using default shader");
    } else {
        static char processed_file[1024];
        snprintf(processed_file, sizeof(processed_file), "./tmp/processed_shaders/%s", file_name);
        MakeDirectory(GetDirectoryPath(processed_file));
        SaveFileText(processed_file, shader_str);
    }

    shader = LoadShaderFromMemory(NULL, shader_str);

    RL_FREE(shader_str);
    
    return shader;
}

