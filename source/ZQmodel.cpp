	// ZQmodel.cpp
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../include/ZQmodel.h"

#ifdef CUDA_RT

d_ZQmodel_c ZQmodel::generate_model() {
	d_ZQmodel_c r{};
	r.v_count = this->vCount;
	r.t_count = this->tCount;

	std::vector<ZQ_BVHbase> bases;

	for (ulong_t i = 0; i < this->tCount / BVH_TRI_COUNT; i++) {
		bases.push_back(ZQ_BVHbase{});

		bases.back().min = this->vPositions[0];
		bases.back().max = this->vPositions[0];
		for (ulong_t j = 0; j < BVH_TRI_COUNT; j++) {
			ulong_t idx = i * BVH_TRI_COUNT + j;
			if (idx < this->tCount) {
				bases.back().tri_idxs[j] = (uint_t)idx;
				bases.back().tri_count = (ushort_t)j;

				tri_t t = this->tIndices[idx];

				bases.back().min = glm::min(bases.back().min, glm::min(this->vPositions[t.x], glm::min(this->vPositions[t.y], this->vPositions[t.z])));
				bases.back().max = glm::max(bases.back().max, glm::max(this->vPositions[t.x], glm::max(this->vPositions[t.y], this->vPositions[t.z])));

			}
		}
	}

	std::vector<ZQ_BVHnode> nodes;
	ulong_t count = 0;
	for (ulong_t remaining = bases.size() / 2; remaining > 0; remaining = remaining / 2) {
		for (ulong_t i = 0; i < remaining; i++) {
			if (count == 0) {
				ZQ_BVHnode r{};
				r.next_base = true;

				r.next_a = i * 2;
				r.next_b = i * 2 + 1;


				r.min = glm::min(bases[i * 2].min, bases[i * 2 + 1].min);
				r.max = glm::max(bases[i * 2].max, bases[i * 2 + 1].max);
				nodes.push_back(r);
			}
			else {
				if (remaining > 1) {
					ZQ_BVHnode r{};
					r.next_base = false;

					r.min = glm::min(nodes[i * 2].min, nodes[i * 2 + 1].min);
					r.max = glm::max(nodes[i * 2].max, nodes[i * 2 + 1].max);

					r.next_a = i * 2;
					r.next_b = i * 2 + 1;

					nodes.push_back(r);
				}
				else if (remaining == 1) {
					ZQ_BVHnode r{};
					r.next_base = false;

					r.min = nodes[i * 2].min;
					r.max = nodes[i * 2].max;

					r.next_a = i * 2;
					r.next_b = i * 2 + 1;

					nodes.push_back(r);
				}
			}
		}
		count++;
	}

	cudaMalloc((void**)&r.bvh_bases, sizeof(ZQ_BVHbase) * bases.size());
	cudaMalloc((void**)&r.bvh_nodes, sizeof(ZQ_BVHnode) * nodes.size());

	cudaMalloc((void**)&r.vPos, sizeof(vec3_t) * this->vCount);
	cudaMalloc((void**)&r.vNorms, sizeof(vec3_t) * this->vCount);
	cudaMalloc((void**)&r.vUVs, sizeof(vec2_t) * this->vCount);
	cudaMalloc((void**)&r.tNorms, sizeof(vec3_t) * this->tCount);
	cudaMalloc((void**)&r.tIdxs, sizeof(tri_t) * this->tCount);
	cudaMalloc((void**)&r.tShow, sizeof(bool) * this->tCount);

	cudaMemcpy(r.vPos, this->vPositions.data(), sizeof(vec3_t) * this->vCount, cudaMemcpyHostToDevice);
	cudaMemcpy(r.vNorms, this->vNormals.data(), sizeof(vec3_t) * this->vCount, cudaMemcpyHostToDevice);
	cudaMemcpy(r.vUVs, this->vUVs.data(), sizeof(vec2_t) * this->vCount, cudaMemcpyHostToDevice);

	cudaMemcpy(r.tNorms, this->tNormals.data(), sizeof(vec3_t) * this->tCount, cudaMemcpyHostToDevice);
	cudaMemcpy(r.tIdxs, this->tIndices.data(), sizeof(tri_t) * this->tCount, cudaMemcpyHostToDevice);

	cudaMemcpy(r.bvh_bases, bases.data(), sizeof(ZQ_BVHbase) * bases.size(), cudaMemcpyHostToDevice);
	cudaMemcpy(r.bvh_nodes, nodes.data(), sizeof(ZQ_BVHnode) * nodes.size(), cudaMemcpyHostToDevice);
	cudaDeviceSynchronize();

	r.aabb_min = this->aabb[0];
	r.aabb_max = this->aabb[1];

	return r;
}

#endif

const ulong_t MAX_BONE_CHILDREN = 8;

void ZQmodel::load_data(const tinygltf::Model m) {
	this->aabb[0] = vec3_t(0.0f);
	this->aabb[1] = vec3_t(0.0f);
	for (const auto& mesh : m.meshes) {
		for (const auto& prim : mesh.primitives) {
			const auto& posAccIdx = prim.attributes.find("POSITION");
			if (posAccIdx == prim.attributes.end()) {
				continue;
			}

			const tinygltf::Accessor& posAcc = m.accessors[posAccIdx->second];
			const tinygltf::BufferView& posBuffView = m.bufferViews[posAcc.bufferView];
			const tinygltf::Buffer& posBuff = m.buffers[posBuffView.buffer];

			const size_t posByteOffset = posBuffView.byteOffset + posAcc.byteOffset;
			const unsigned char* posBuffPtr = posBuff.data.data() + posBuffView.byteOffset + posAcc.byteOffset;

			if (posAcc.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT || posAcc.type == TINYGLTF_TYPE_VEC3) {
				const float* positions = reinterpret_cast<const float*>(posBuffPtr);
				for (size_t i = 0; i < posAcc.count / 3; i++) {
					float x = positions[i * 3 + 0];
					float y = positions[i * 3 + 1];
					float z = positions[i * 3 + 2];

					vec3_t p{ x, y, z };
					if (i == 0) {
						this->aabb[0] = p;
						this->aabb[1] = p;
					}
					else {
						if (x < this->aabb[0].x) {
							this->aabb[0].x = x;
						}
						else if (x > this->aabb[1].x) {
							this->aabb[1].x = x;
						}
						if (y < this->aabb[0].y) {
							this->aabb[0].y = y;
						}
						else if (y > this->aabb[1].y) {
							this->aabb[1].y = y;
						}
						if (z < this->aabb[0].z) {
							this->aabb[0].z = z;
						}
						else if (z > this->aabb[1].z) {
							this->aabb[1].z = z;
						}
					}
					this->vPositions.push_back(p);
#ifdef DEBUG
					std::cout << "pos " << i << " = " << x << ", " << y << ", " << z << std::endl;
#endif
				}
			}
			this->vCount = this->vPositions.size();	

#ifdef DEBUG
			std::cout << "AABB = { ( " << this->aabb[0].x << ", " << this->aabb[0].y << ", " << this->aabb[0].z << " ), ( " << this->aabb[1].x << ", " << this->aabb[1].y << ", " << this->aabb[1].z << ") }" << std::endl;
#endif

			const auto& normAccIdx = prim.attributes.find("NORMAL");
			if (normAccIdx == prim.attributes.end()) {
#ifdef DEBUG
				std::cout << "Normals empty." << std::endl;
#endif
				continue;
			}

			const tinygltf::Accessor& normAcc = m.accessors[normAccIdx->second];
			const tinygltf::BufferView& normBuffView = m.bufferViews[normAcc.bufferView];
			const tinygltf::Buffer& normBuff = m.buffers[normBuffView.buffer];

			const size_t normByteOffset = normBuffView.byteOffset + normAcc.byteOffset;
			const unsigned char* normBuffPtr = normBuff.data.data() + normBuffView.byteOffset + normAcc.byteOffset;

			if (normAcc.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT || normAcc.type == TINYGLTF_TYPE_VEC3) {
				const float* normals = reinterpret_cast<const float*>(normBuffPtr);
				for (size_t i = 0; i < normAcc.count / 3; i++) {
					float x = normals[i * 3 + 0];
					float y = normals[i * 3 + 1];
					float z = normals[i * 3 + 2];

					vec3_t p{ x, y, z };
					this->vNormals.push_back(p);
#ifdef DEBUG
					std::cout << "norm " << i << " = " << x << ", " << y << ", " << z << std::endl;
#endif
				}
			}

			const auto& uvAccIdx = prim.attributes.find("TEXCOORD_0");
			if (uvAccIdx == prim.attributes.end()) {
#ifdef DEBUG
				std::cout << "UVs empty." << std::endl;
#endif
				continue;
			}

			const tinygltf::Accessor& uvAcc = m.accessors[uvAccIdx->second];
			const tinygltf::BufferView& uvBuffView = m.bufferViews[uvAcc.bufferView];
			const tinygltf::Buffer uvBuff = m.buffers[uvBuffView.buffer];

			const size_t uvByteOffset = uvBuffView.byteOffset + uvAcc.byteOffset;
			const unsigned char* uvBuffPtr = uvBuff.data.data() + uvBuffView.byteOffset + uvAcc.byteOffset;

			if (uvAcc.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT || uvAcc.type == TINYGLTF_TYPE_VEC2) {
				const float* uvs = reinterpret_cast<const float*>(uvBuffPtr);
				for (size_t i = 0; i < uvAcc.count / 2; i++) {
					float x = uvs[i * 2 + 0];
					float y = uvs[i * 2 + 1];

					uv_t p{ x, y };
					this->vUVs.push_back(p);
#ifdef DEBUG
					std::cout << "uv " << i << " = {" << x << ", " << y << " }" << std::endl;
#endif
				}
			}

			if (prim.indices >= 0) {
				const tinygltf::Accessor& triAcc = m.accessors[prim.indices];
				const tinygltf::BufferView& triBuffView = m.bufferViews[triAcc.bufferView];
				const tinygltf::Buffer& triBuff = m.buffers[triBuffView.buffer];
				const unsigned char* triBuffPtr = &triBuff.data[triBuffView.byteOffset + triAcc.byteOffset];

				if (triAcc.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
					const uint16_t* tris = reinterpret_cast<const uint16_t*>(triBuffPtr);

					for (size_t i = 0; i < triAcc.count; i++) {
						uint32_t x = static_cast<uint32_t>(tris[i]);// / (3);
						//uint32_t y = static_cast<uint32_t>(tris[i * 3 + 1]);// / (3);
						//uint32_t z = static_cast<uint32_t>(tris[i * 3 + 2]);// / (3);

						//tri_t t{ x, y, z };
						this->tIndices.push_back(x);
						//this->tIndices.push_back(y);
						//this->tIndices.push_back(z);
#ifdef DEBUG
						//std::cout << "tri " << i << " = {" << x << ", " << y << ", " << z << " }" << std::endl;
#endif

						/*vec3_t xP, yP, zP;
						if (x < this->vCount) {
							xP = this->vPositions[x];
						}
						else {
							xP = this->vPositions[this->vCount - 1];
						}
						if (y < this->vCount) {
							yP = this->vPositions[y];
						}
						else {
							yP = this->vPositions[this->vCount - 1];
						}
						if (z < this->vCount) {
							zP = this->vPositions[z];
						}
						else {
							zP = this->vPositions[this->vCount - 1];
						}

						vec3_t c = glm::cross(xP - yP, zP - yP);

						this->tNormals.push_back(c);*/
					}
				}
				else if (triAcc.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
					const uint32_t* tris = reinterpret_cast<const uint32_t*>(triBuffPtr);

					for (size_t i = 0; i < triAcc.count; i++) {
						uint32_t x = tris[i];// / 3;
						//uint32_t y = tris[i * 3 + 1];// / 3;
						//uint32_t z = tris[i * 3 + 2];// / 3;

						//tri_t t{ x, y, z };
						this->tIndices.push_back(x);
						//this->tIndices.push_back(y);
						//this->tIndices.push_back(z);
#ifdef DEBUG
						//std::cout << "tri " << i << " = {" << x << ", " << y << ", " << z << " }" << std::endl;
#endif
						/*vec3_t xP, yP, zP;
						if (x < this->vCount) {
							xP = this->vPositions[x];
						}
						else {
							xP = this->vPositions[this->vCount - 1];
						}
						if (y < this->vCount) {
							yP = this->vPositions[y];
						}
						else {
							yP = this->vPositions[this->vCount - 1];
						}
						if (z < this->vCount) {
							zP = this->vPositions[z];
						}
						else {
							zP = this->vPositions[this->vCount - 1];
						}

						vec3_t c = glm::cross(xP - yP, zP - yP);

						this->tNormals.push_back(c);*/
					}
				}
				this->tCount = this->tIndices.size();

				for (ulong_t j = 0; j < this->tCount / 3; j++) {

					uint32_t x = this->tIndices[j * 3];
					uint32_t y = this->tIndices[j * 3 + 1];
					uint32_t z = this->tIndices[j * 3 + 2];

					vec3_t xP, yP, zP;
					if (x < this->vCount) {
						xP = this->vPositions[x];
					}
					else {
						xP = this->vPositions[this->vCount - 1];
					}
					if (y < this->vCount) {
						yP = this->vPositions[y];
					}
					else {
						yP = this->vPositions[this->vCount - 1];
					}
					if (z < this->vCount) {
						zP = this->vPositions[z];
					}
					else {
						zP = this->vPositions[this->vCount - 1];
					}

					vec3_t c = glm::cross(xP - yP, zP - yP);

					this->tNormals.push_back(c);
				}
			}
		}
	}
	this->vtPositions = std::vector<vec2_t>(this->vCount);
	this->vtNormals = std::vector<vec3_t>(this->vCount);

	this->tNormalsT = std::vector<vec3_t>(this->tCount / 3);
}

void ZQmodel::debug_print() {

}

d_ZQmodel ZQmodel::to_gpu() {
	d_ZQmodel r{};
	
	int_t err = 0;

	glGenVertexArrays(1, &r.vao);

	GLuint* buffs = new GLuint[6];
	glCreateBuffers(6, buffs);

	glBindVertexArray(r.vao);

	r.vPos = buffs[0];
	r.vNorms = buffs[1];
	r.vUVs = buffs[2];
	r.v_count = this->vCount;

	r.tNorms = buffs[3];
	r.tIdxs = buffs[4];
	r.tShow = buffs[5];
	r.t_count = this->tCount;

	glBindBuffer(GL_ARRAY_BUFFER, r.vPos);
	glBufferData(GL_ARRAY_BUFFER, this->vPositions.size() * sizeof(vec3_t), this->vPositions.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3_t), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, r.vNorms);
	glBufferData(GL_ARRAY_BUFFER, this->vNormals.size() * sizeof(vec3_t), this->vNormals.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vec3_t), (void*)0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, r.vUVs);
	glBufferData(GL_ARRAY_BUFFER, this->vUVs.size() * sizeof(vec2_t), this->vUVs.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vec2_t), (void*)0);
	glEnableVertexAttribArray(2);
	
	glBindBuffer(GL_ARRAY_BUFFER, r.tNorms);
	glBufferData(GL_ARRAY_BUFFER, this->tNormals.size() * sizeof(vec3_t), this->tNormals.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(vec3_t), (void*)0);
	glEnableVertexAttribArray(3);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r.tIdxs);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->tIndices.size() * sizeof(uint_t), this->tIndices.data(), GL_STATIC_DRAW);
	//glVertexAttribIPointer(4, 3, GL_UNSIGNED_INT, sizeof(tri_t), (void*)0);
	//glEnableVertexAttribArray(4);

	/*glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r.tShow);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, r.t_count * sizeof(bool), this->tIndices.data(), GL_STATIC_DRAW);
	glVertexAttribIPointer(5, 1, GL_BYTE, sizeof(bool), (void*)0);
	glEnableVertexAttribArray(5);*/
	glBindVertexArray(0);

	return r;
}

ZQmodel::ZQmodel(std::string path, std::string name, bool is_bin) {

	tinygltf::Model raw_model;
	tinygltf::TinyGLTF loader;

	this->filepath = path;
	this->name = name;

	bool r = false;
	std::string error;
	std::string warn;

	if (is_bin) {
		r = loader.LoadBinaryFromFile(&raw_model, &error, &warn, this->filepath);
	}
	else {
		r = loader.LoadASCIIFromFile(&raw_model, &error, &warn, this->filepath);
	}

	if (r) {
		if (!error.empty()) {
#ifdef DEBUG
			std::cout << error << std::endl;
#endif
			return;
		}
		if (!warn.empty()) {
#ifdef DEBUG
			std::cout << warn << std::endl;
#endif
			return;
		}
		this->load_data(raw_model);
	}
	else {
#ifdef DEBUG
		std::cout << "Could not find model at: " << this->filepath << std::endl;
#endif
		return;
	}
}