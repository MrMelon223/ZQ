	// ZQmodel.cpp
//#define STBI_NO_SIMD
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

/*void ZQskeleton::traverse_skeleton(const tinygltf::Model& m, int idx, int d = 0) {
	const tinygltf::Node& n = m.nodes[idx];

	for (size_t i = 0; i < d; i++) {
#ifdef DEBUG
		if (!n.name.empty()) {
			std::cout << "Skeleton Model " << this->name << " Node [" << idx << "]: " << n.name << std::endl;
		}
		else {
			std::cout << "Skeleton Model " << this->name << " Node [" << idx << "]: Not found!" << std::endl;
		}
#endif
		for (int cIdx : n.children) {
			ZQskeleton::traverse_skeleton(m, cIdx, d + 1);
		}
	}
}

void ZQskeleton::extract_skeleton(const tinygltf::Model& m) {
	if (m.skins.empty()) {
#ifdef DEBUG
		std::cout << "No Skeleton data found!: " << this->name << std::endl;
#endif
		return;
	}

	for (size_t i = 0; i < m.skins.size(); i++) {
		const tinygltf::Skin& s = m.skins[i];
#ifdef DEBUG
		std::cout << "Skeleton #" << i << " Name: " << s.name << std::endl;
#endif

		if (s.skeleton >= 0) {
#ifdef DEBUG
			std::cout << "Skeleton root node: " << s.skeleton << std::endl;
#endif
		}

		if (s.inverseBindMatrices >= 0) {
			const tinygltf::Accessor& acc = m.accessors[s.inverseBindMatrices];
			const tinygltf::BufferView& bView = m.bufferViews[acc.bufferView];
			const tinygltf::Buffer& buff = m.buffers[bView.buffer];

			const float* mtxData = reinterpret_cast<const float*>(&(buff.data[acc.byteOffset + bView.byteOffset]));

#ifdef DEBUG
			std::cout << "Extracted " << acc.count << " Inverse Bind Matrices." << std::endl;
#endif
		}

#ifdef DEBUG
		std::cout << " ** Skeleton : **" << std::endl;
#endif

		std::vector<bool> is_child(s.joints.size(), false);
		for (size_t j = 0; j < s.joints.size(); j++) {
			
			int jNodeIdx = s.joints[j];
			for (int cIdx : m.nodes[jNodeIdx].children) {

				for (size_t k = 0; k < s.joints.size(); k++) {
					if (s.joints[k] == cIdx) {
						is_child[k] = true;
					}
				}
			}
		}

		for (size_t j = 0; j < s.joints.size(); j++) {
			if (!is_child[j]) {
				ZQskeleton::traverse_skeleton(m, s.joints[j], 0);
			}
		}
	}
}

void ZQskeleton::extract_mesh_skin(const tinygltf::Model& m, const tinygltf::Primitive& prim) {

	std::vector<tri_t> triangles;
	if (prim.indices >= 0) {
		const tinygltf::Accessor& idxAcc = m.accessors[prim.indices];
		const tinygltf::BufferView& bView = m.bufferViews[idxAcc.bufferView];
		const tinygltf::Buffer& buff = m.buffers[bView.buffer];

		const unsigned char* dPtr = &(buff.data[idxAcc.byteOffset + bView.byteOffset]);
		size_t stride = idxAcc.ByteStride(bView);

		std::vector<uint_t> rIndices;
		for (size_t i = 0; i < idxAcc.count; i++) {
			uint_t idx = 0;

			if (idxAcc.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
				idx = *reinterpret_cast<const uint_t*>(dPtr + i * stride);
			}
			else if (idxAcc.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
				idx = *reinterpret_cast<const ushort_t*>(dPtr + i * stride);
			}
			else if (idxAcc.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
				idx = *reinterpret_cast<const uchar_t*>(dPtr + i * stride);
			}

			rIndices.push_back(idx);
		}

		for (size_t i = 0; i < rIndices.size(); i += 3) {
			triangles.push_back(tri_t{ rIndices[i], rIndices[i + 1], rIndices[i + 2] });
		}
#ifdef DEBUG
		std::cout << "Extracted " << triangles.size() << " triangles.\n";
#endif
	}
}

void ZQskeleton::load_from(const tinygltf::Model m) {
	
	if (m.skins.empty()) {
#ifdef DEBUG
		std::cout << "Cannot load model skeleton at " << this->path << std::endl;
#endif
		return;
	}

	const tinygltf::Skin& skn = m.skins[0];
	
	size_t bCount = skn.joints.size();



	for (size_t i = 0; i < bCount; i++) {
		int nIdx = skn.joints[i];
		const tinygltf::Node& n = m.nodes[nIdx];

		std::string bName = "";
		if (n.name.empty()) {
#ifdef DEBUG
			std::cout << "Bone name empty!" << std::endl;
#endif
		}
		else {
			bName = n.name;
			this->bone_name_table.insert({ bName, i });

			ZQbone_t bone = {};
			if (!n.translation.empty()) {
#ifdef DEBUG
				std::cout << "Bone #" << i << " position value empty!" << std::endl;
#endif
			}
			else {
				bone.end = vec3_t(static_cast<float>(n.translation[0]), static_cast<float>(n.translation[1]), static_cast<float>(n.translation[2]));
			}

			this->bones.push_back(bone);
#ifdef DEBUG
			std::cout << "Bone name " << bName << " added!" << std::endl;
#endif
		}
	}

}

void ZQskeleton::load_skeleton(std::string path) {
	tinygltf::Model raw_model;
	tinygltf::TinyGLTF loader;

	this->path = path;

	bool r = false;
	std::string error;
	std::string warn;

	r = loader.LoadBinaryFromFile(&raw_model, &error, &warn, this->path);

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
		this->load_from(raw_model);
	}
	else {
#ifdef DEBUG
		std::cout << "Could not find model at: " << this->path << std::endl;
#endif
		return;
	}
}

ZQskeleton::ZQskeleton(std::string path, std::string name) {
	this->name = name;

	this->load_skeleton(name);
}*/

void ZQmodel::load_data(const tinygltf::Model m) {
	this->aabb[0] = vec3_t(std::numeric_limits<float>::max());
	this->aabb[1] = vec3_t(-std::numeric_limits<float>::max());

	this->vCount = 0;
	this->tCount = 0;

	size_t vertex_offset = 0;
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

			if (posAcc.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT && posAcc.type == TINYGLTF_TYPE_VEC3) {
				const float* positions = reinterpret_cast<const float*>(posBuffPtr);
				for (size_t i = 0; i < posAcc.count; i++) {
					float x = positions[i * 3 + 0];
					float y = positions[i * 3 + 1];
					float z = positions[i * 3 + 2];

					vec3_t p{ x, y, z };
					if (i == 0) {
						this->aabb[0] = p;
						this->aabb[1] = p;
					}
					else {
						/*if (x < this->aabb[0].x) {
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
							this->aabb[1].z = z;*/
						this->aabb[0].x = std::min(this->aabb[0].x, x);
						this->aabb[0].y = std::min(this->aabb[0].y, y);
						this->aabb[0].z = std::min(this->aabb[0].z, z);

						this->aabb[1].x = std::max(this->aabb[1].x, x);
						this->aabb[1].y = std::max(this->aabb[1].y, y);
						this->aabb[1].z = std::max(this->aabb[1].z, z);
					}
					this->vPositions.push_back(p);
#ifdef DEBUG_MODEL_TELE
					std::cout << "pos " << i << " = " << x << ", " << y << ", " << z << std::endl;
#endif
				}
			}
			this->vCount += this->vPositions.size();	

#ifdef DEBUG_MODEL_TELE
			std::cout << "AABB = { ( " << this->aabb[0].x << ", " << this->aabb[0].y << ", " << this->aabb[0].z << " ), ( " << this->aabb[1].x << ", " << this->aabb[1].y << ", " << this->aabb[1].z << ") }" << std::endl;
#endif

			const auto& normAccIdx = prim.attributes.find("NORMAL");
			if (normAccIdx == prim.attributes.end()) {
#ifdef DEBUG_MODEL_TELE
				std::cout << "Normals empty." << std::endl;
#endif
				continue;
			}

			const tinygltf::Accessor& normAcc = m.accessors[normAccIdx->second];
			const tinygltf::BufferView& normBuffView = m.bufferViews[normAcc.bufferView];
			const tinygltf::Buffer& normBuff = m.buffers[normBuffView.buffer];

			const size_t normByteOffset = normBuffView.byteOffset + normAcc.byteOffset;
			const unsigned char* normBuffPtr = normBuff.data.data() + normBuffView.byteOffset + normAcc.byteOffset;

			if (normAcc.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT && normAcc.type == TINYGLTF_TYPE_VEC3) {
				const float* normals = reinterpret_cast<const float*>(normBuffPtr);
				for (size_t i = 0; i < normAcc.count / 3; i++) {
					float x = normals[i * 3 + 0];
					float y = normals[i * 3 + 1];
					float z = normals[i * 3 + 2];

					vec3_t p{ x, y, z };
					this->vNormals.push_back(p);
#ifdef DEBUG_MODEL_TELE
					std::cout << "norm " << i << " = " << x << ", " << y << ", " << z << std::endl;
#endif
				}
			}

			const auto& uvAccIdx = prim.attributes.find("TEXCOORD_0");
			if (uvAccIdx == prim.attributes.end()) {
#ifdef DEBUG_MODEL_TELE
				std::cout << "UVs empty." << std::endl;
#endif
				continue;
			}

			const tinygltf::Accessor& uvAcc = m.accessors[uvAccIdx->second];
			const tinygltf::BufferView& uvBuffView = m.bufferViews[uvAcc.bufferView];
			const tinygltf::Buffer& uvBuff = m.buffers[uvBuffView.buffer];

			const size_t uvByteOffset = uvBuffView.byteOffset + uvAcc.byteOffset;
			const unsigned char* uvBuffPtr = uvBuff.data.data() + uvBuffView.byteOffset + uvAcc.byteOffset;

			if (uvAcc.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT && uvAcc.type == TINYGLTF_TYPE_VEC2) {
				const float* uvs = reinterpret_cast<const float*>(uvBuffPtr);
				for (size_t i = 0; i < uvAcc.count / 2; i++) {
					float x = uvs[i * 2 + 0];
					float y = uvs[i * 2 + 1];

					uv_t p{ x, y };
					this->vUVs.push_back(p);
#ifdef DEBUG_MODEL_TELE
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

#ifdef DEBUG
					std::cout << "16-bit uint used!" << std::endl;
#endif

					for (size_t i = 0; i < triAcc.count; i++) {
						uint32_t x = static_cast<uint32_t>(tris[i]);// / (3);
						//uint32_t y = static_cast<uint32_t>(tris[i * 3 + 1]);// / (3);
						//uint32_t z = static_cast<uint32_t>(tris[i * 3 + 2]);// / (3);

						//tri_t t{ x, y, z };
						this->tIndices.push_back(x);
						//this->tIndices.push_back(y);
						//this->tIndices.push_back(z);
#ifdef DEBUG_MODEL_TELE
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

#ifdef DEBUG
					std::cout << "32-bit uint used!" << std::endl;
#endif

					for (size_t i = 0; i < triAcc.count; i++) {
						uint32_t x = tris[i];// / 3;
						//uint32_t y = tris[i * 3 + 1];// / 3;
						//uint32_t z = tris[i * 3 + 2];// / 3;

						//tri_t t{ x, y, z };
						this->tIndices.push_back(x);
						//this->tIndices.push_back(y);
						//this->tIndices.push_back(z);
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
				this->tCount += this->tIndices.size() / 3;
				ulong_t local_tCount = this->tIndices.size() / 3;
				for (ulong_t j = 0; j < local_tCount; j++) {

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
			vertex_offset += posAcc.count;
		}
	}
	this->vtPositions = std::vector<vec2_t>(this->vCount);
	this->vtNormals = std::vector<vec3_t>(this->vCount);

	this->tNormalsT = std::vector<vec3_t>(this->tCount);

	std::vector<vec4_t> vWeights;
	std::vector<uvec4_t> vJoints;

	std::vector<gltfSkinData> temp_skin_data;

	for (size_t j = 0; j < m.meshes.size(); j++) {
		if (!m.meshes[j].primitives.empty()) {
			for (size_t k = 0; k < m.meshes[j].primitives.size(); k++) {
				const tinygltf::Primitive& prim = m.meshes[j].primitives[k];

				auto jntAttr = prim.attributes.find("JOINTS_0");
				auto wghtAttr = prim.attributes.find("WEIGHTS_0");

				if (jntAttr != prim.attributes.end() && wghtAttr != prim.attributes.end()) {
					const tinygltf::Accessor& jAcc = m.accessors[jntAttr->second];
					const tinygltf::BufferView& jView = m.bufferViews[jAcc.bufferView];
					const tinygltf::Buffer& jBuff = m.buffers[jView.buffer];
					const uchar_t* jPtr = jBuff.data.data() + jView.byteOffset + jAcc.byteOffset;

					const tinygltf::Accessor& wAcc = m.accessors[wghtAttr->second];
					const tinygltf::BufferView& wView = m.bufferViews[wAcc.bufferView];
					const tinygltf::Buffer& wBuff = m.buffers[wView.buffer];
					const uchar_t* wPtr = wBuff.data.data() + wView.byteOffset + wAcc.byteOffset;

					temp_skin_data.resize(jAcc.count);

					for (size_t v = 0; v < jAcc.count; v++) {
						if (jAcc.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
							const uint8_t* jnt = reinterpret_cast<const uint8_t*>(jPtr + v * jView.byteStride);

							temp_skin_data[v].joints[0] = jnt[0];
							temp_skin_data[v].joints[1] = jnt[1];
							temp_skin_data[v].joints[2] = jnt[2];
							temp_skin_data[v].joints[3] = jnt[3];
						}
						else if (jAcc.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
							const uint16_t* jnt = reinterpret_cast<const uint16_t*>(jPtr + v * jView.byteStride);

							temp_skin_data[v].joints[0] = jnt[0];
							temp_skin_data[v].joints[1] = jnt[1];
							temp_skin_data[v].joints[2] = jnt[2];
							temp_skin_data[v].joints[3] = jnt[3];
						}
						else {
							const uint32_t* jnt = reinterpret_cast<const uint32_t*>(jPtr + v * jView.byteStride);

							temp_skin_data[v].joints[0] = jnt[0];
							temp_skin_data[v].joints[1] = jnt[1];
							temp_skin_data[v].joints[2] = jnt[2];
							temp_skin_data[v].joints[3] = jnt[3];
						}

						const float* wght = reinterpret_cast<const float*>(wPtr + v *(wView.byteStride ? wView.byteStride : 16));
						temp_skin_data[v].weights[0] = wght[0];
						temp_skin_data[v].weights[1] = wght[1];
						temp_skin_data[v].weights[2] = wght[2];
						temp_skin_data[v].weights[2] = wght[3];
					}
				}

			}
		}
	}

	if (!m.skins.empty()) {
		for (const tinygltf::Skin& skn : m.skins) {
			for (size_t j = 0; j < skn.joints.size(); j++) {
				
				int nIdx = skn.joints[j];
				const tinygltf::Node nde = m.nodes[nIdx];

				ZQbone_t bone = {};

				bone.name = nde.name.empty() ? "joint_" + std::to_string(j) : nde.name;

				if (nde.translation.size() == 3) {
					bone.end = vec3_t(
						static_cast<float>(nde.translation[0]),
						static_cast<float>(nde.translation[1]),
						static_cast<float>(nde.translation[2])
					);
				}
				else {
					bone.end = vec3_t(0.0f, 0.0f, 0.0f);
				}

				const float wght_threshold = 0.01f;
				for (size_t vIdx = 0; vIdx < temp_skin_data.size(); vIdx++) {
					const gltfSkinData& data = temp_skin_data[vIdx];
					for (uchar_t k = 0; k < 4; k++) {
						
						if (data.joints[k] == j && data.weights[k] > wght_threshold) {
							bone.vertex_idxs.push_back(static_cast<uint_t>(vIdx));
						}
					}
				}

				this->bones.push_back(bone);
				this->bone_name_table.emplace(bone.name, this->bones.size() - 1);

#ifdef DEBUG
				std::cout << "Adding bone \'" << bone.name << "\' at index " << this->bone_name_table[bone.name] << " = { " << bone.end.x << ", " << bone.end.y << ", " << bone.end.z << " } impacting " << bone.vertex_idxs.size() << " vertices." << std::endl;
#endif

				/*if (skn.inverseBindMatrices >= 0) {
					const tinygltf::Accessor& acc = m.accessors[skn.inverseBindMatrices];
					const tinygltf::BufferView& bView = m.bufferViews[acc.bufferView];
					const tinygltf::Buffer& buff = m.buffers[bView.buffer];

					this->inverse_bind_matrices.reserve(acc.count);

					const unsigned char* dPtr = buff.data.data() + bView.byteOffset + acc.byteOffset;
					size_t stride = acc.ByteStride(bView);

					for (size_t i = 0; i < skn.joints.size(); i++) {
						int nIdx = skn.joints[i];
						std::string bName = m.nodes[nIdx].name;
						m.nodes[nIdx].

							this->bone_name_table[bName] = static_cast<size_t>(nIdx);
					}
				}*/
			}
		}
	}

}

void ZQmodel::debug_print() {

}

d_ZQmodel ZQmodel::to_gpu() {
	d_ZQmodel r{};
	
	int_t err = 0;

	glGenVertexArrays(1, &r.vao);

	GLuint* buffs = new GLuint[6];
	glGenBuffers(6, buffs);

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