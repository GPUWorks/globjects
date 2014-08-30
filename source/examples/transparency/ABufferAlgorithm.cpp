#include "ABufferAlgorithm.h"

#include <glbinding/gl/gl.h>

#include <globjects/Program.h>
#include <globjects/Framebuffer.h>
#include <globjects/Texture.h>
#include <globjects/Renderbuffer.h>
#include <globjects/Buffer.h>
#include <globjects/globjects.h>
#include <globjects/NamedString.h>

#include <globjects-base/File.h>
#include <globjects-utils/Camera.h>
#include <globjects-utils/ScreenAlignedQuad.h>
#include <globjects-utils/globjects-utils.h>

namespace {

struct ABufferEntry {
    glm::vec4 color;
    float z;
    int next;
};

struct Head {
    int startIndex;
    int size;

    Head()
    : startIndex(-1)
    , size(0) {
    }
};

const int ABUFFER_SIZE = 8;

} // anonymous namespace

void ABufferAlgorithm::initialize(const std::string & transparencyShaderFilePath, glo::Shader *vertexShader, glo::Shader *geometryShader) {
    glo::NamedString::create("/transparency/abuffer_definitions", "const int ABUFFER_SIZE = " + std::to_string(ABUFFER_SIZE) + ";");
    glo::NamedString::create("/transparency/abuffer.glsl", new glo::File(transparencyShaderFilePath + "abuffer.glsl"));

    m_program = new glo::Program();
	m_program->attach(glo::Shader::fromFile(gl::GL_FRAGMENT_SHADER, transparencyShaderFilePath +  "abuffer.frag"));
	m_program->attach(vertexShader);
	if (geometryShader != nullptr) m_program->attach(geometryShader);

    m_opaqueBuffer = createColorTex();
    m_depthBuffer = new glo::Renderbuffer();

    m_renderFbo = new glo::Framebuffer();
    m_renderFbo->attachTexture(gl::GL_COLOR_ATTACHMENT0, m_opaqueBuffer.get());
    m_renderFbo->attachRenderBuffer(gl::GL_DEPTH_ATTACHMENT, m_depthBuffer.get());
    m_renderFbo->setDrawBuffer(gl::GL_COLOR_ATTACHMENT0);

    m_linkedListBuffer = new glo::Buffer();
    m_linkedListBuffer->setName("A Buffer Linked Lists");

    m_headBuffer = new glo::Buffer();
    m_headBuffer->setName("A Buffer Heads");

    m_counter = new glo::Buffer();
    m_counter->setName("A Buffer Counter");

	m_quad = new gloutils::ScreenAlignedQuad(glo::Shader::fromFile(gl::GL_FRAGMENT_SHADER, transparencyShaderFilePath +  "abuffer_post.frag"));

    m_colorBuffer = createColorTex();
    m_postFbo = new glo::Framebuffer;
    m_postFbo->attachTexture(gl::GL_COLOR_ATTACHMENT0, m_colorBuffer.get());
    m_postFbo->setDrawBuffer(gl::GL_COLOR_ATTACHMENT0);
}

void ABufferAlgorithm::draw(const DrawFunction& drawFunction, gloutils::Camera* camera, int width, int height) {
    m_renderFbo->bind(gl::GL_FRAMEBUFFER);
    m_renderFbo->clear(gl::GL_DEPTH_BUFFER_BIT);
    m_renderFbo->clearBuffer(gl::GL_COLOR, 0, glm::vec4(1.0f, 1.0f, 1.0f, std::numeric_limits<float>::max()));

    // reset head buffer & counter
    static glm::ivec2 initialHead(-1, 0);
    static int initialCounter = 0;
    m_headBuffer->setData(static_cast<gl::GLsizei>(width * height * sizeof(Head)), nullptr, gl::GL_DYNAMIC_DRAW);
    m_headBuffer->clearData(gl::GL_RG32I, gl::GL_RG, gl::GL_INT, &initialHead);
    m_counter->setData(static_cast<gl::GLsizei>(sizeof(int)), nullptr, gl::GL_DYNAMIC_DRAW);
    m_counter->clearData(gl::GL_R32I, gl::GL_RED, gl::GL_UNSIGNED_INT, &initialCounter);

    // bind buffers
    m_linkedListBuffer->bindBase(gl::GL_SHADER_STORAGE_BUFFER, 0);
    m_headBuffer->bindBase(gl::GL_SHADER_STORAGE_BUFFER, 1);
    m_counter->bindBase(gl::GL_ATOMIC_COUNTER_BUFFER, 0);

    m_program->setUniform("viewprojectionmatrix", camera->viewProjection());
    m_program->setUniform("normalmatrix", camera->normal());
    m_program->setUniform("screenSize", glm::ivec2(width, height));
    m_program->use();

    drawFunction(m_program);

    m_renderFbo->unbind(gl::GL_FRAMEBUFFER);

    gl::glMemoryBarrier(gl::GL_SHADER_STORAGE_BARRIER_BIT);


    m_postFbo->bind(gl::GL_FRAMEBUFFER);
    m_postFbo->clear(gl::GL_COLOR_BUFFER_BIT);

    m_opaqueBuffer->bindActive(gl::GL_TEXTURE0);

    m_quad->program()->setUniform("screenSize", glm::ivec2(width, height));
    m_quad->program()->setUniform("opaqueBuffer", 0);
    m_quad->draw();

    m_opaqueBuffer->unbindActive(gl::GL_TEXTURE0);

    m_postFbo->unbind(gl::GL_FRAMEBUFFER);
}

void ABufferAlgorithm::resize(int width, int height) {
    int depthBits = glo::Framebuffer::defaultFBO()->getAttachmentParameter(gl::GL_DEPTH, gl::GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE);
    m_opaqueBuffer->image2D(0, gl::GL_RGBA32F, width, height, 0, gl::GL_RGBA, gl::GL_FLOAT, nullptr);
    m_depthBuffer->storage(depthBits == 16 ? gl::GL_DEPTH_COMPONENT16 : gl::GL_DEPTH_COMPONENT, width, height);
    m_linkedListBuffer->setData(static_cast<gl::GLsizei>(width * height * ABUFFER_SIZE * sizeof(ABufferEntry)), nullptr, gl::GL_DYNAMIC_DRAW);
    m_colorBuffer->image2D(0, gl::GL_RGBA32F, width, height, 0, gl::GL_RGBA, gl::GL_FLOAT, nullptr);
}

glo::Texture* ABufferAlgorithm::getOutput()
{
    return m_colorBuffer;
}
