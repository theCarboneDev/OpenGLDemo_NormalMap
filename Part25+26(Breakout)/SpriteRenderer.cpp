#include "SpriteRenderer.h"

SpriteRenderer::SpriteRenderer(Shader s) {
	this->shader = s;
	this->initRenderData();
}
SpriteRenderer::~SpriteRenderer() {
	glDeleteVertexArrays(1, &this->quadVAO);
}

void SpriteRenderer::drawSprite(Texture2D &texture, glm::vec2 position, glm::vec2 size, float rotate, glm::vec3 color) {
	this->shader.Use();
	glm::mat4 model = glm::mat4(1.0);
	model = glm::translate(model, glm::vec3(position, 1.0));

	model = glm::translate(model, glm::vec3(0.5f * size.x, 0.5f * size.y, 0.f));
	model = glm::rotate(model, glm::radians(rotate), glm::vec3(0.f, 0.f, 1.f));
	model = glm::translate(model, glm::vec3(-0.5f * size.x, -0.5f * size.y, 0.f));

	model = glm::scale(model, glm::vec3(size, 1.0));

	glActiveTexture(GL_TEXTURE0);
	texture.Bind();

	this->shader.SetMatrix4("model", model);
	this->shader.SetVector3f("spriteColor", color);

	glBindVertexArray(this->quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

void SpriteRenderer::initRenderData() {
	unsigned int VBO;
	float vertex[] = {
		0.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 1.f,
		1.f, 1.f, 1.f, 1.f,

		0.f, 0.f, 0.f, 0.f,
		1.f, 0.f, 1.f, 0.f,
		1.f, 1.f, 1.f, 1.f,
	};

	glGenVertexArrays(1, &this->quadVAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(this->quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex), &vertex, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}