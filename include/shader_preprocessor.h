#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include <raylib.h>

// Load a file and resolve #include directives (ignores comments)
// Note: File paths should use forward slashes
char* LoadAndPreprocessFileText(char* file_name);

// Load a shader and resolve #include directives
// Note: File paths should use forward slashes
Shader LoadAndPreProcessFragmentShader(char* file_name);

#endif // PREPROCESSOR_H
