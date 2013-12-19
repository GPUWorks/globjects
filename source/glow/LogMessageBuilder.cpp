#include <cassert>

#include <glow/logging.h>
#include <glow/Object.h>
#include <glow/Buffer.h>
#include <glow/FrameBufferObject.h>
#include <glow/Program.h>
#include <glow/Query.h>
#include <glow/RenderBufferObject.h>
#include <glow/Sampler.h>
#include <glow/Shader.h>
#include <glow/Texture.h>
#include <glow/TransformFeedback.h>
#include <glow/VertexArrayObject.h>
#include <glow/Version.h>
#include <glow/AbstractUniform.h>
#include <glow/ObjectVisitor.h>

#include <glow/LogMessageBuilder.h>

namespace {

using namespace glow;

class TypeGetter : public glow::ObjectVisitor
{
public:
    static std::string getType(Object * object)
    {
        return TypeGetter().typeFor(object);
    }

    std::string typeFor(Object * object)
    {
        m_typeName = "glow::Object";
        visit(object);
        return m_typeName;
    }

    virtual void visitBuffer(Buffer * buffer) override
    {
        m_typeName = "Buffer";
    }

    virtual void visitFrameBufferObject(FrameBufferObject * fbo) override
    {
        m_typeName = "FrameBufferObject";
    }

    virtual void visitProgram(Program * program) override
    {
        m_typeName = "Program";
    }

    virtual void visitQuery(Query * query) override
    {
        m_typeName = "Query";
    }

    virtual void visitRenderBufferObject(RenderBufferObject * rbo) override
    {
        m_typeName = "RenderBufferObject";
    }

    virtual void visitSampler(Sampler * sampler) override
    {
        m_typeName = "Sampler";
    }

    virtual void visitShader(Shader * shader) override
    {
        m_typeName = "Shader";
    }

    virtual void visitTexture(Texture * texture) override
    {
        m_typeName = "Texture";
    }

    virtual void visitTransformFeedback(TransformFeedback * transformfeedback) override
    {
        m_typeName = "TransformFeedback";
    }

    virtual void visitVertexArrayObject(VertexArrayObject * vao) override
    {
        m_typeName = "VertexArrayObject";
    }
protected:
    std::string m_typeName;
};

}

namespace glow 
{

LogMessageBuilder::LogMessageBuilder(LogMessage::Level level, AbstractLogHandler * handler)
: std::stringstream()
, m_level(level)
, m_handler(handler)
{
}

LogMessageBuilder::LogMessageBuilder(const LogMessageBuilder& builder)
: m_level(builder.m_level)
, m_handler(builder.m_handler)
{
	str(builder.str());
}

LogMessageBuilder::~LogMessageBuilder()
{
	if (m_handler)
		m_handler->handle(LogMessage(m_level, str()));
}

LogMessageBuilder& LogMessageBuilder::operator<<(const char * c)
{
    assert(c != nullptr);

	write(c, std::strlen(c));
	return *this;
}

LogMessageBuilder& LogMessageBuilder::operator<<(const std::string & str)
{
	write(str.c_str(), str.length());
	return *this;
}

LogMessageBuilder& LogMessageBuilder::operator<<(bool b)
{
	*this << (b ? "true" : "false");
	return *this;
}

LogMessageBuilder& LogMessageBuilder::operator<<(char c)
{
	std::stringstream::operator<<(c);
	return *this;
}

LogMessageBuilder& LogMessageBuilder::operator<<(int i)
{
	std::stringstream::operator<<(i);
	return *this;
}

LogMessageBuilder& LogMessageBuilder::operator<<(float f)
{
	std::stringstream::operator<<(f);
	return *this;
}

LogMessageBuilder& LogMessageBuilder::operator<<(double d)
{
	std::stringstream::operator<<(d);
	return *this;
}

LogMessageBuilder& LogMessageBuilder::operator<<(long double d)
{
	std::stringstream::operator<<(d);
	return *this;
}

LogMessageBuilder& LogMessageBuilder::operator<<(unsigned u)
{
	std::stringstream::operator<<(u);
	return *this;
}

LogMessageBuilder& LogMessageBuilder::operator<<(long l)
{
	std::stringstream::operator<<(l);
	return *this;
}

LogMessageBuilder& LogMessageBuilder::operator<<(long long l)
{
    std::stringstream::operator<<(l);
    return *this;
}

LogMessageBuilder& LogMessageBuilder::operator<<(unsigned long ul)
{
	std::stringstream::operator<<(ul);
	return *this;
}

LogMessageBuilder& LogMessageBuilder::operator<<(unsigned char uc)
{
	std::stringstream::operator<<(uc);
	return *this;
}

LogMessageBuilder& LogMessageBuilder::operator<<(void * pointer)
{
	std::stringstream::operator<<(pointer);
	return *this;
}

LogMessageBuilder& LogMessageBuilder::operator<<(std::ostream & (*manipulator)(std::ostream &))
{
	std::stringstream::operator<<(manipulator);
	return *this;
}

LogMessageBuilder& LogMessageBuilder::operator<<(const glm::vec2 & v)
{
    *this << "vec2(" << v.x << "," << v.y << ")";
    return *this;
}

LogMessageBuilder& LogMessageBuilder::operator<<(const glm::vec3 & v)
{
    *this << "vec3(" << v.x << "," << v.y << "," << v.z << ")";
    return *this;
}

LogMessageBuilder& LogMessageBuilder::operator<<(const glm::vec4 & v)
{
    *this << "vec4(" << v.x << "," << v.y << "," << v.z << "," << v.w << ")";
    return *this;
}

LogMessageBuilder& LogMessageBuilder::operator<<(const glm::ivec2 & v)
{
	*this << "ivec2(" << v.x << "," << v.y << ")";
	return *this;
}

LogMessageBuilder& LogMessageBuilder::operator<<(const glm::ivec3 & v)
{
	*this << "ivec3(" << v.x << "," << v.y << "," << v.z << ")";
	return *this;
}

LogMessageBuilder& LogMessageBuilder::operator<<(const glm::ivec4 & v)
{
	*this << "ivec4(" << v.x << "," << v.y << "," << v.z << "," << v.w << ")";
	return *this;
}

LogMessageBuilder& LogMessageBuilder::operator<<(const glm::mat2 & m)
{
	*this
		<< "mat2("
		<< "(" << m[0][0] << ", " << m[0][1] << "), "
		<< "(" << m[1][0] << ", " << m[1][1] << ")"
		<< ")";
	return *this;
}

LogMessageBuilder& LogMessageBuilder::operator<<(const glm::mat3 & m)
{
	*this
		<< "mat3("
		<< "(" << m[0][0] << ", " << m[0][1] << ", " << m[0][2] << "), "
		<< "(" << m[1][0] << ", " << m[1][1] << ", " << m[1][2] <<"), "
		<< "(" << m[2][0] << ", " << m[2][1] << ", " << m[2][2] <<")"
		<< ")";
	return *this;
}

LogMessageBuilder& LogMessageBuilder::operator<<(const glm::mat4 & m)
{
	*this
		<< "mat4("
		<< "(" << m[0][0] << ", " << m[0][1] << ", " << m[0][2] << ", " << m[0][3] << "), "
		<< "(" << m[1][0] << ", " << m[1][1] << ", " << m[1][2] << ", " << m[1][3] <<"), "
		<< "(" << m[2][0] << ", " << m[2][1] << ", " << m[2][2] << ", " << m[2][3] <<"), "
		<< "(" << m[3][0] << ", " << m[3][1] << ", " << m[3][2] << ", " << m[3][3] <<")"
		<< ")";
	return *this;
}

LogMessageBuilder& LogMessageBuilder::operator<<(Object* object)
{
    assert(object != nullptr);

    *this << TypeGetter::getType(object) << " (" << object->id() << ")";

    return *this;
}

LogMessageBuilder& LogMessageBuilder::operator<<(Buffer* object)
{
    return *this<<static_cast<Object*>(object);
}

LogMessageBuilder& LogMessageBuilder::operator<<(FrameBufferObject* object)
{
    return *this<<static_cast<Object*>(object);
}

LogMessageBuilder& LogMessageBuilder::operator<<(Program* object)
{
    return *this<<static_cast<Object*>(object);
}

LogMessageBuilder& LogMessageBuilder::operator<<(Query* object)
{
    return *this<<static_cast<Object*>(object);
}

LogMessageBuilder& LogMessageBuilder::operator<<(RenderBufferObject* object)
{
    return *this<<static_cast<Object*>(object);
}

LogMessageBuilder& LogMessageBuilder::operator<<(Sampler* object)
{
    return *this<<static_cast<Object*>(object);
}

LogMessageBuilder& LogMessageBuilder::operator<<(Shader* object)
{
    return *this<<static_cast<Object*>(object);
}

LogMessageBuilder& LogMessageBuilder::operator<<(Texture* object)
{
    return *this<<static_cast<Object*>(object);
}

LogMessageBuilder& LogMessageBuilder::operator<<(TransformFeedback* object)
{
    return *this<<static_cast<Object*>(object);
}

LogMessageBuilder& LogMessageBuilder::operator<<(VertexArrayObject* object)
{
    return *this<<static_cast<Object*>(object);
}

LogMessageBuilder& LogMessageBuilder::operator<<(AbstractUniform* uniform)
{
    assert(uniform != nullptr);

    *this << "Uniform " << uniform->name();

    return *this;
}

LogMessageBuilder& LogMessageBuilder::operator<<(const Version& version)
{
    *this << "Version " << version.toString();

    return *this;
}

} // namespace glow
