// Stub implementations for OpenGL functions for demo
#include <GL/gl.h>

// Stub implementations for OpenGL functions
unsigned int glCreateShader(unsigned int type) { return 1; }
void glShaderSource(unsigned int shader, int count, const char** string, const int* length) {}
void glCompileShader(unsigned int shader) {}
void glGetShaderiv(unsigned int shader, unsigned int pname, int* params) { *params = 1; }
void glGetShaderInfoLog(unsigned int shader, int bufSize, int* length, char* infoLog) {}
void glDeleteShader(unsigned int shader) {}
unsigned int glCreateProgram() { return 1; }
void glAttachShader(unsigned int program, unsigned int shader) {}
void glLinkProgram(unsigned int program) {}
void glGetProgramiv(unsigned int program, unsigned int pname, int* params) { *params = 1; }
void glGetProgramInfoLog(unsigned int program, int bufSize, int* length, char* infoLog) {}
void glDeleteProgram(unsigned int program) {}
void glUseProgram(unsigned int program) {}
int glGetUniformLocation(unsigned int program, const char* name) { return 1; }
void glUniform1i(int location, int value) {}
void glUniform1f(int location, float value) {}
void glUniform2fv(int location, int count, const float* value) {}
void glUniform3fv(int location, int count, const float* value) {}
void glUniform4fv(int location, int count, const float* value) {}
void glUniformMatrix2fv(int location, int count, int transpose, const float* value) {}
void glUniformMatrix3fv(int location, int count, int transpose, const float* value) {}
void glUniformMatrix4fv(int location, int count, int transpose, const float* value) {}
void glGenVertexArrays(int n, unsigned int* arrays) { arrays[0] = 1; }
void glBindVertexArray(unsigned int array) {}
void glGenBuffers(int n, unsigned int* buffers) { buffers[0] = 1; }
void glBindBuffer(unsigned int target, unsigned int buffer) {}
void glBufferData(unsigned int target, int size, const void* data, unsigned int usage) {}
void glEnableVertexAttribArray(unsigned int index) {}
void glVertexAttribPointer(unsigned int index, int size, unsigned int type, int normalized, int stride, const void* pointer) {}
void glDeleteVertexArrays(int n, const unsigned int* arrays) {}
void glDeleteBuffers(int n, const unsigned int* buffers) {}
