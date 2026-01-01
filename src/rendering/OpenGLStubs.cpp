// OpenGL Stub Implementations for Interstate3D
// These prevent crashes when real OpenGL functions aren't available

#include <GLFW/glfw3.h>
#include <iostream>

// Basic OpenGL function stubs that do nothing but prevent crashes
extern "C" {
    void glGenVertexArrays(GLsizei n, GLuint* arrays) {
        if (arrays) {
            for (GLsizei i = 0; i < n; i++) {
                arrays[i] = i + 1; // Simple fake IDs
            }
        }
    }
    
    void glBindVertexArray(GLuint array) {
        // Do nothing - stub implementation
    }
    
    void glGenBuffers(GLsizei n, GLuint* buffers) {
        if (buffers) {
            for (GLsizei i = 0; i < n; i++) {
                buffers[i] = i + 100; // Simple fake IDs
            }
        }
    }
    
    void glBindBuffer(GLenum target, GLuint buffer) {
        // Do nothing - stub implementation
    }
    
    void glBufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage) {
        // Do nothing - stub implementation
    }
    
    void glEnableVertexAttribArray(GLuint index) {
        // Do nothing - stub implementation
    }
    
    void glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer) {
        // Do nothing - stub implementation
    }
    
    void glDeleteVertexArrays(GLsizei n, const GLuint* arrays) {
        // Do nothing - stub implementation
    }
    
    void glDeleteBuffers(GLsizei n, const GLuint* buffers) {
        // Do nothing - stub implementation
    }
    
    GLuint glCreateShader(GLenum type) {
        return 1; // Return fake shader ID
    }
    
    void glShaderSource(GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length) {
        // Do nothing - stub implementation
    }
    
    void glCompileShader(GLuint shader) {
        // Do nothing - stub implementation
    }
    
    void glGetShaderiv(GLuint shader, GLenum pname, GLint* params) {
        if (params && pname == GL_COMPILE_STATUS) {
            *params = GL_TRUE; // Always report success
        }
    }
    
    void glGetShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog) {
        if (infoLog && bufSize > 0) {
            infoLog[0] = '\0'; // Empty error message
        }
        if (length) *length = 0;
    }
    
    void glDeleteShader(GLuint shader) {
        // Do nothing - stub implementation
    }
    
    GLuint glCreateProgram() {
        return 1; // Return fake program ID
    }
    
    void glAttachShader(GLuint program, GLuint shader) {
        // Do nothing - stub implementation
    }
    
    void glLinkProgram(GLuint program) {
        // Do nothing - stub implementation
    }
    
    void glGetProgramiv(GLuint program, GLenum pname, GLint* params) {
        if (params && pname == GL_LINK_STATUS) {
            *params = GL_TRUE; // Always report success
        }
    }
    
    void glGetProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog) {
        if (infoLog && bufSize > 0) {
            infoLog[0] = '\0'; // Empty error message
        }
        if (length) *length = 0;
    }
    
    void glDeleteProgram(GLuint program) {
        // Do nothing - stub implementation
    }
    
    void glUseProgram(GLuint program) {
        // Do nothing - stub implementation
    }
    
    GLint glGetUniformLocation(GLuint program, const GLchar* name) {
        return 0; // Return fake uniform location
    }
    
    void glUniform1i(GLint location, GLint value) {
        // Do nothing - stub implementation
    }
    
    void glUniform1f(GLint location, GLfloat value) {
        // Do nothing - stub implementation
    }
    
    void glUniform2fv(GLint location, GLsizei count, const GLfloat* value) {
        // Do nothing - stub implementation
    }
    
    void glUniform3fv(GLint location, GLsizei count, const GLfloat* value) {
        // Do nothing - stub implementation
    }
    
    void glUniform4fv(GLint location, GLsizei count, const GLfloat* value) {
        // Do nothing - stub implementation
    }
    
    void glUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
        // Do nothing - stub implementation
    }
    
    void glUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
        // Do nothing - stub implementation
    }
    
    void glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
        // Do nothing - stub implementation
    }
}
