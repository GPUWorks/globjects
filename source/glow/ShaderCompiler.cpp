#include "ShaderCompiler.h"

#include <sstream>
#include <algorithm>
#include <cctype>

#include <GL/glew.h>

#include <glow/Error.h>
#include <glow/logging.h>
#include <glow/Shader.h>
#include <glow/StringSource.h>
#include <glow/NamedStrings.h>
#include <glow/Version.h>

namespace {
    // From http://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring
    inline std::string trim(const std::string &s)
    {
       auto wsfront=std::find_if_not(s.begin(),s.end(),[](int c){return std::isspace(c);});
       auto wsback=std::find_if_not(s.rbegin(),s.rend(),[](int c){return std::isspace(c);}).base();
       return (wsback<=wsfront ? std::string() : std::string(wsfront,wsback));
    }

    inline bool contains(const std::string& string, const std::string& search)
    {
        return string.find(search) != std::string::npos;
    }
}

namespace glow {

ShaderCompiler::ShaderCompiler(Shader* shader)
: m_shader(shader)
{
}

ShaderCompiler::~ShaderCompiler()
{
}

bool ShaderCompiler::compile(Shader* shader)
{
    return ShaderCompiler(shader).compile();
}

bool ShaderCompiler::compile()
{
    if (m_shader->source())
    {
        std::string source = m_shader->source()->string();

        m_includes.clear();

        if (glCompileShaderIncludeARB && Version::current() >= Version(3, 2))
        {
            const char * sourcePointer = source.c_str();

            glShaderSource(m_shader->id(), 1, &sourcePointer, 0);
            CheckGLError();
        }
        else
        {
            std::string completeSource = resolveIncludes(source);

            const char * sourcePointer = completeSource.c_str();

            glShaderSource(m_shader->id(), 1, &sourcePointer, 0);
            CheckGLError();
        }
    }

    if (glCompileShaderIncludeARB && Version::current() >= Version(3, 2))
    {
        glCompileShaderIncludeARB(m_shader->id(), 0, nullptr, nullptr);
        CheckGLError();
    }
    else
    {
        glCompileShader(m_shader->id());
        CheckGLError();
    }


    bool compiled = checkCompileStatus();

    m_shader->m_compiled = compiled;

    if (compiled)
        m_shader->changed();

    return compiled;
}

bool ShaderCompiler::checkCompileStatus()
{
    GLint status = 0;

    glGetShaderiv(m_shader->id(), GL_COMPILE_STATUS, &status);
    CheckGLError();

    if (GL_FALSE == status)
    {
        critical()
            << "Compiler error:" << std::endl
            << m_shader->shaderString() << std::endl
            << m_shader->infoLog();

        return false;
    }

    return true;
}

std::string ShaderCompiler::resolveIncludes(const std::string& source)
{
    std::istringstream sourcestream(source);
    std::stringstream destinationstream;

    do
    {
        std::string line;

        std::getline(sourcestream, line);

        std::string trimmedLine = trim(line);

        if (trimmedLine.size() >= 0)
        {
            if (trimmedLine[0] == '#')
            {
                if (contains(trimmedLine, "extension"))
                {
                    // #extension GL_ARB_shading_language_include : require
                    if (contains(trimmedLine, "GL_ARB_shading_language_include"))
                    {
                        // drop line
                    }
                    else
                    {
                        destinationstream << line << '\n';
                    }
                }
                else if (contains(trimmedLine, "include"))
                {
                    size_t leftBracketPosition = trimmedLine.find('<');
                    size_t rightBracketPosition = trimmedLine.find('>');

                    if (leftBracketPosition == std::string::npos || rightBracketPosition == std::string::npos)
                    {
                        glow::warning() << "Malformed #include";
                    }
                    else
                    {
                        std::string include = trimmedLine.substr(leftBracketPosition+1, rightBracketPosition - leftBracketPosition - 1);

                        if (include.size() == 0 || include[0] != '/')
                        {
                            glow::warning() << "Malformed #include";
                        }
                        else
                        {
                            if (m_includes.count(include) == 0)
                            {
                                m_includes.insert(include);
                                destinationstream << resolveIncludes(NamedStrings::namedString(include));
                            }
                        }
                    }
                }
                else
                {
                    // other macro
                    destinationstream << line << '\n';
                }
            }
            else
            {
                // normal line
                destinationstream << line << '\n';
            }
        }
    }
    while (sourcestream.good());

    return destinationstream.str();
}

} // namespace glow
