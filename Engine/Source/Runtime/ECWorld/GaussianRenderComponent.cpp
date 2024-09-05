#include "GaussianRenderComponent.h"
#include "GaussianUtil\DataView.h"
#include "Log\Log.h"
#include "Rendering/Utility/VertexLayoutUtility.h"
#include "Scene/Types.h"
#include "Scene/VertexFormat.h"

#include <bgfx/bgfx.h>
#include <unordered_map>

namespace engine
{

void GaussianRenderComponent::Build()
{
	const static std::array<float, 8> vertices =
	{
		-2.0f, -2.0f,
		2.0f, -2.0f,
		2.0f, 2.0f,
		-2.0f, 2.0f,
	};
	const static std::array<uint16_t, 6> indices = {
		0, 1, 2, 0, 2, 3,
	};

	bgfx::VertexLayout vertexLayout;
	vertexLayout.begin().add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float).end();
	m_vertexBufferHandle = bgfx::createVertexBuffer(bgfx::makeRef(vertices.data(), uint32_t(vertices.size() * sizeof(float))), vertexLayout).idx;
	m_indexBufferHandle = bgfx::createIndexBuffer(bgfx::makeRef(indices.data(), uint32_t(indices.size() * sizeof(uint16_t)))).idx;
}

void GaussianRenderComponent::ProcessingPlyBuffer()
{
	std::string header(reinterpret_cast<const char*>(m_readBuffer.data()), 1024 * 10);
	const std::string header_end = "end_header\n";
	const size_t header_end_index = header.find(header_end);
	if (header_end_index == std::string::npos)
	{
		throw std::runtime_error("Unable to read .ply file header");
	}

	const std::string vertexCountStr = header.substr(header.find("element vertex ") + 15, header_end_index - header.find("element vertex ") - 15);
	const int vertexCount = std::stoi(vertexCountStr);
	m_vertextCount = vertexCount;
	CD_ERROR("Vertex Count: ");
	CD_ERROR(vertexCount);

	size_t  row_offset = 0;
	std::unordered_map<std::string, size_t> offsets;
	std::unordered_map<std::string, std::string> types;

	const std::unordered_map<std::string, std::string> TYPE_MAP = {
		{"double", "getFloat64"},
		{"int", "getInt32"},
		{"uint", "getUint32"},
		{"float", "getFloat32"},
		{"short", "getInt16"},
		{"ushort", "getUint16"},
		{"uchar", "getUint8"}
	};

	std::string delimiter = "\n";
	size_t pos = 0;
	while ((pos = header.find(delimiter)) != std::string::npos)
	{
		std::string line = header.substr(0, pos);
		header.erase(0, pos + delimiter.length());

		if (line.find("property ") == 0)
		{
			std::istringstream iss(line);
			std::string property, type, name;

			iss >> property >> type >> name;
			std::string arrayType = TYPE_MAP.count(type) ? TYPE_MAP.at(type) : "getInt8";

			types[name] = arrayType;
			offsets[name] = row_offset;
			CD_ERROR(name);
			CD_ERROR(types[name]);
			CD_ERROR(offsets[name]);	
			std::string number_str;
			for (char c : arrayType)
			{
				if (isdigit(c))
				{
					number_str += c;
				}
			}

			if (!number_str.empty())
			{
				row_offset += std::stoi(number_str) / 8;
			}
		}
	}

	CD_ERROR("all row_offset: ");
	CD_ERROR(row_offset);

	std::vector<float> sizeList(vertexCount);
	std::vector<size_t> sizeIndex(vertexCount);
	DataView dataView(m_readBuffer, header_end_index + header_end.length());

	for (size_t row = 0; row < vertexCount; ++row)
	{
		sizeIndex[row] = row;
		if (types.find("scale_0") == types.end()) continue;

		float scale_0 = dataView.get<float>(row * row_offset + offsets["scale_0"]);
		float scale_1 = dataView.get<float>(row * row_offset + offsets["scale_1"]);
		float scale_2 = dataView.get<float>(row * row_offset + offsets["scale_2"]);
		float opacity = dataView.get<float>(row * row_offset + offsets["opacity"]);

		float size = std::exp(scale_0) * std::exp(scale_1) * std::exp(scale_2);
		float opacity_value = 1 / (1 + std::exp(-opacity));
		sizeList[row] = size * opacity_value;
	}

	std::sort(sizeIndex.begin(), sizeIndex.end(), [&sizeList](size_t a, size_t b){
		return sizeList[a] > sizeList[b];
		});


	// 6*4 + 4 + 4 = 8*4
// XYZ - Position (Float32)
// XYZ - Scale (Float32)
// RGBA - colors (uint8)
// IJKL - quaternion/rot (uint8)

	const size_t rowLength = 3 * 4 + 3 * 4 + 4 + 4;
	std::vector<std::byte> buffer(rowLength * vertexCount);

	for (size_t j = 0; j < vertexCount; ++j)
	{
		size_t row = sizeIndex[j];

		std::byte* basePtr = &buffer[j * rowLength];

		float position[3] = {
				dataView.get<float>(row * row_offset + offsets["x"]),
				dataView.get<float>(row * row_offset + offsets["y"]),
				dataView.get<float>(row * row_offset + offsets["z"])
		};

		float opacity = dataView.get<float>(row * row_offset + offsets["opacity"]);

		uint8_t rgba[4]{};
		const float SH_C0 = 0.28209479177387814f;
		if (types.find("f_dc_0") != types.end())
		{

			float f_dc_0 = dataView.get<float>(row * row_offset + offsets["f_dc_0"]);
			float f_dc_1 = dataView.get<float>(row * row_offset + offsets["f_dc_1"]);
			float f_dc_2 = dataView.get<float>(row * row_offset + offsets["f_dc_2"]);

			rgba[0] = static_cast<uint8_t>(std::round((0.5f + SH_C0 * f_dc_0) * 255.0f));
			rgba[1] = static_cast<uint8_t>(std::round((0.5f + SH_C0 * f_dc_1) * 255.0f));
			rgba[2] = static_cast<uint8_t>(std::round((0.5f + SH_C0 * f_dc_2) * 255.0f));
		}
		else
		{
			float red = dataView.get<float>(row * row_offset + offsets["red"]);
			float green = dataView.get<float>(row * row_offset + offsets["green"]);
			float blue = dataView.get<float>(row * row_offset + offsets["blue"]);
			rgba[0] = static_cast<uint8_t>(red);
			rgba[1] = static_cast<uint8_t>(green);
			rgba[2] = static_cast<uint8_t>(blue);
		}

		if (types.find("opacity") != types.end())
		{
			rgba[3] = static_cast<uint8_t>(std::round((1 / (1 + std::exp(-opacity))) * 255.0f));
		}
		else
		{
			rgba[3] = static_cast<uint8_t>(255.0f);
		}

		float scales[3]{};
		uint8_t rot[4]{};
		if (types.find("scale_0") != types.end())
		{
			float rot_0 = dataView.get<float>(row * row_offset + offsets["rot_0"]);
			float rot_1 = dataView.get<float>(row * row_offset + offsets["rot_1"]);
			float rot_2 = dataView.get<float>(row * row_offset + offsets["rot_2"]);
			float rot_3 = dataView.get<float>(row * row_offset + offsets["rot_3"]);

			float qlen = std::sqrt(
				rot_0 * rot_0 +
				rot_1 * rot_1 +
				rot_2 * rot_2 +
				rot_3 * rot_3
			);

			rot[0] = static_cast<uint8_t>(std::round((rot_0 / qlen) * 128.0f + 128.0f));
			rot[1] = static_cast<uint8_t>(std::round((rot_1 / qlen) * 128.0f + 128.0f));
			rot[2] = static_cast<uint8_t>(std::round((rot_2 / qlen) * 128.0f + 128.0f));
			rot[3] = static_cast<uint8_t>(std::round((rot_3 / qlen) * 128.0f + 128.0f));

			scales[0] = std::exp(dataView.get<float>(row * row_offset + offsets["scale_0"]));
			scales[1] = std::exp(dataView.get<float>(row * row_offset + offsets["scale_1"]));
			scales[2] = std::exp(dataView.get<float>(row * row_offset + offsets["scale_2"]));
		}
		else
		{
			scales[0] = 0.01f;
			scales[1] = 0.01f;
			scales[2] = 0.01f;

			rot[0] = static_cast<uint8_t>(255.0f );
			rot[1] = static_cast<uint8_t>(0.0f);
			rot[2] = static_cast<uint8_t>(0.0f);
			rot[3] = static_cast<uint8_t>(0.0f);
		}		
		std::memcpy(basePtr, position, sizeof(position));
		std::memcpy(basePtr + 4 * 3, scales, sizeof(scales));
		std::memcpy(basePtr + 4 * 3 + 4 * 3, rgba, sizeof(rgba));
		std::memcpy(basePtr + 4 * 3 + 4 * 3 + 4, rot, sizeof(rot));
	}
	m_gausianAttributesBuffer = std::move(buffer);

	CD_ERROR("ReadBuffer Over");
}

uint16_t floatToHalf(float value)
{
	int32_t f = *reinterpret_cast<int32_t*> (&value);
	uint16_t sign = (f >> 31) & 0x0001;
	uint16_t exp = (f >> 23) & 0x00ff;
	int32_t frac = f & 0x007fffff;
	uint16_t newExp;
	if (exp == 0)
	{
		newExp = 0;
	}
	else if (exp < 113)
	{
		newExp = 0;
		frac |= 0x00800000;
		frac >>= (113 - exp);
		if (frac & 0x01000000)
		{
			newExp = 1;
			frac = 0;
		}
	}
	else if (exp < 142)
	{
		newExp = exp - 112;
	}
	else
	{
		newExp = 31;
		frac = 0;
	}

	return (sign << 15) | (newExp << 10) | (frac >> 13);
}

// Function to pack two half-precision floating point values into a single uint32_t
uint32_t packHalf2x16(float x, float y)
{
	uint16_t halfX = floatToHalf(x);
	uint16_t halfY = floatToHalf(y);
	return (static_cast<uint32_t>(halfX) | (static_cast<uint32_t>(halfY) << 16));
}

void GaussianRenderComponent::GenerateTexture()
{
	if (m_gausianAttributesBuffer.empty()) return;

	const float* f_buffer = reinterpret_cast<const float*>(m_gausianAttributesBuffer.data());
	const uint8_t* u_buffer = reinterpret_cast<const uint8_t*>(m_gausianAttributesBuffer.data());

	size_t  texwidth = 1024 * 2;
	size_t  texheight = static_cast<int>(std::ceil((2 * m_vertextCount) / static_cast<float>(texwidth)));
	m_textureBuffer.resize(texwidth * texheight * 4 * sizeof(uint32_t));

	uint8_t* texdata_c = reinterpret_cast<uint8_t*>(m_textureBuffer.data());
	float* texdata_f = reinterpret_cast<float*>(m_textureBuffer.data());

	for (size_t i = 0; i < m_vertextCount; ++i)
	{
		// x, y, z
		texdata_f[8 * i + 0] = f_buffer[8 * i + 0];
		texdata_f[8 * i + 1] = f_buffer[8 * i + 1];
		texdata_f[8 * i + 2] = f_buffer[8 * i + 2];

		// r, g, b, a
		texdata_c[4 * (8 * i + 7) + 0] = u_buffer[32 * i + 24 + 0];
		texdata_c[4 * (8 * i + 7) + 1] = u_buffer[32 * i + 24 + 1];
		texdata_c[4 * (8 * i + 7) + 2] = u_buffer[32 * i + 24 + 2];
		texdata_c[4 * (8 * i + 7) + 3] = u_buffer[32 * i + 24 + 3];

		// quaternions
		float scale[3] = {
			f_buffer[8 * i + 3 + 0],
			f_buffer[8 * i + 3 + 1],
			f_buffer[8 * i + 3 + 2]
		};

		float rot[4] = {
			(u_buffer[32 * i + 28 + 0] - 128) / 128.0f,
			(u_buffer[32 * i + 28 + 1] - 128) / 128.0f,
			(u_buffer[32 * i + 28 + 2] - 128) / 128.0f,
			(u_buffer[32 * i + 28 + 3] - 128) / 128.0f
		};

		// Compute the matrix product of S and R (M = Scale * Rotation)
		float M[9] = {
			1.0f - 2.0f * (rot[2] * rot[2] + rot[3] * rot[3]),
			2.0f * (rot[1] * rot[2] + rot[0] * rot[3]),
			2.0f * (rot[1] * rot[3] - rot[0] * rot[2]),

			2.0f * (rot[1] * rot[2] - rot[0] * rot[3]),
			1.0f - 2.0f * (rot[1] * rot[1] + rot[3] * rot[3]),
			2.0f * (rot[2] * rot[3] + rot[0] * rot[1]),

			2.0f * (rot[1] * rot[3] + rot[0] * rot[2]),
			2.0f * (rot[2] * rot[3] - rot[0] * rot[1]),
			1.0f - 2.0f * (rot[1] * rot[1] + rot[2] * rot[2])
		};

		for (int j = 0; j < 9; ++j)
		{
			M[j] *= scale[j / 3];
		}

		float sigma[6] = {
			M[0] * M[0] + M[3] * M[3] + M[6] * M[6],
			M[0] * M[1] + M[3] * M[4] + M[6] * M[7],
			M[0] * M[2] + M[3] * M[5] + M[6] * M[8],
			M[1] * M[1] + M[4] * M[4] + M[7] * M[7],
			M[1] * M[2] + M[4] * M[5] + M[7] * M[8],
			M[2] * M[2] + M[5] * M[5] + M[8] * M[8]
		};

		m_textureBuffer[8 * i + 4] = packHalf2x16(4 * sigma[0], 4 * sigma[1]);
		m_textureBuffer[8 * i + 5] = packHalf2x16(4 * sigma[2], 4 * sigma[3]);
		m_textureBuffer[8 * i + 6] = packHalf2x16(4 * sigma[4], 4 * sigma[5]);
	}

	bool hasMips = false;  //if-mip-map
	uint16_t numLayers = 1;
	bgfx::TextureFormat::Enum format = bgfx::TextureFormat::RGBA32U; // fomat

	const bgfx::Memory* mem = bgfx::copy(m_textureBuffer.data(), static_cast<uint32_t>(m_textureBuffer.size()));
	m_textureHandle = bgfx::createTexture2D(static_cast<uint16_t>(texwidth), static_cast<uint16_t>(texheight), hasMips, numLayers, format, 0, mem).idx;
	CD_ERROR("Generate TextureHandle Over");
}


}