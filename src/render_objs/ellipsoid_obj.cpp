#include "ellipsoid_obj.h"

RENDERABLE_BEGIN
EllipsoidObj::EllipsoidObj()
{
	SetUpData();
	SetUpShader();
}

std::shared_ptr<AABB> EllipsoidObj::GetAABB()
{
	return m_aabb_obj;
}

void EllipsoidObj::DrawObj(const std::unordered_map<std::string, std::any>& uniform)
{
	m_shader->Use();

	auto projection = std::any_cast<glm::mat4>(uniform.at("projection"));
	auto view = std::any_cast<glm::mat4>(uniform.at("view"));
	auto model = std::any_cast<glm::mat4>(uniform.at("model"));

	m_shader->Use();
	m_shader->SetMat4("projection", projection);
	m_shader->SetMat4("view", view);
	m_shader->SetMat4("model", model);
	RenderObject::Draw();
}

void EllipsoidObj::ImGuiCallback()
{
	bool changed = false;
	changed |= ImGui::SliderInt("number", &m_number, 0, 100000);
	changed |= ImGui::SliderFloat3("scale", &m_scale[0], 0, 2);
	changed |= ImGui::SliderFloat4("rotation", &m_rotation[0], -2, 2);

	if (changed) SetUpData();
}

void EllipsoidObj::SetUpData()
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::normal_distribution<> d(0.0, 1.0);
	m_aabb_obj = std::make_shared<AABB>();

	std::vector<float> M{
		1.0f - 2.0f * (m_rotation[2] * m_rotation[2] + m_rotation[3] * m_rotation[3]),
		2.0f * (m_rotation[1] * m_rotation[2] + m_rotation[0] * m_rotation[3]),
		2.0f * (m_rotation[1] * m_rotation[3] - m_rotation[0] * m_rotation[2]),

		2.0f * (m_rotation[1] * m_rotation[2] - m_rotation[0] * m_rotation[3]),
		1.0f - 2.0f * (m_rotation[1] * m_rotation[1] + m_rotation[3] * m_rotation[3]),
		2.0f * (m_rotation[2] * m_rotation[3] + m_rotation[0] * m_rotation[1]),

		2.0f * (m_rotation[1] * m_rotation[3] + m_rotation[0] * m_rotation[2]),
		2.0f * (m_rotation[2] * m_rotation[3] - m_rotation[0] * m_rotation[1]),
		1.0f - 2.0f * (m_rotation[1] * m_rotation[1] + m_rotation[2] * m_rotation[2]),
	};

	for (size_t i = 0; i < M.size(); i++) {
		M[i] *= m_scale[std::floor(i / 3)];
	}

	std::vector<float> s{
		M[0] * M[0] + M[3] * M[3] + M[6] * M[6],
		M[0] * M[1] + M[3] * M[4] + M[6] * M[7],
		M[0] * M[2] + M[3] * M[5] + M[6] * M[8],
		M[1] * M[1] + M[4] * M[4] + M[7] * M[7],
		M[1] * M[2] + M[4] * M[5] + M[7] * M[8],
		M[2] * M[2] + M[5] * M[5] + M[8] * M[8],
	};

	Eigen::Matrix3f Sigma(3, 3);
	Sigma << s[0], s[1], s[2],
		s[1], s[3], s[4],
		s[2], s[4], s[5];


	Eigen::SelfAdjointEigenSolver<Eigen::Matrix3f> eigenSolver(Sigma);
	if (eigenSolver.info() != Eigen::Success) {
		std::cerr << "Eigenvalue decomposition failed!" << std::endl;
	}
	Eigen::Vector3f eigenvalues = eigenSolver.eigenvalues();     // 获取特征值
	Eigen::Matrix3f eigenvectors = eigenSolver.eigenvectors();     // 获取特征向量

	std::vector<int> argsort{ 0, 1, 2 };
	std::sort(argsort.begin(), argsort.end(), [&](const int i1, const int i2) {
		return m_scale[i1] < m_scale[i2];
		});

	auto lltOfSigma = Sigma.llt();
	std::vector<glm::vec3>vertices(m_number);

	glm::vec3 value;
	std::vector<glm::vec3> vector(3);
	for (size_t i = 0; i < 3; i++) {
		value[argsort[i]] = eigenvalues[i];
		for (size_t j = 0; j < 3; j++) {
			vector[argsort[i]][j] = eigenvectors.col(i)[j];
		}
	}

	Eigen::Vector3f center(0.0f, 0.0f, 0.0f);
	for (int i = 0; i < m_number; i++) {
		Eigen::VectorXf z(3);
		z(0) = d(gen);
		z(1) = d(gen);
		z(2) = d(gen);
		Eigen::VectorXf y = lltOfSigma.matrixL() * z + center;
		float norm_y = y.norm();
		glm::vec3 radius = glm::sqrt(value) / norm_y;
		vertices[i] = glm::vec3(y[0] * radius[0], y[1] * radius[1], y[2] * radius[2]);
	}

	m_aabb_obj->reset();
	glm::vec3 center1(0.0f, 0.0f, 0.0f);
	glm::vec3 offsetPoint;
	for (int i = 0; i < 3; i++) {
		float sqrt_eigenvalue = std::sqrt(value[i]);
		glm::vec3 displacement = sqrt_eigenvalue * vector[i];
		for (int j = -1; j <= 1; j += 2) {
			offsetPoint = center1 + static_cast<float>(j) * displacement;
			m_aabb_obj->expand({ offsetPoint[0], offsetPoint[1], offsetPoint[2] });
		}
	}


	std::vector<VertexInfo> vertex_info = std::vector<VertexInfo>{
		{"aPos", 0, 3, GL_FLOAT, GL_FALSE, 1, 0},
	};

	SetMesh(&vertices, &vertex_info);
	SetPrimitive(GL_POINTS);
}

void EllipsoidObj::SetUpShader()
{
	m_shader = std::make_unique<Shader>("./shader/sphere_vs.glsl", "./shader/sphere_fs.glsl");
}

RENDERABLE_END