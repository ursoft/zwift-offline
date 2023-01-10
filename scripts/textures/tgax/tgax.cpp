#include "tgax.h"
using namespace std;

const char *g_in_path;
string g_out_path;
void CreateOutputPath(const char* ext) {
	ostringstream file_name;
	file_name << filesystem::current_path().string() << "\\" << filesystem::path(g_in_path).replace_extension(ext).filename().string();
	g_out_path = file_name.str();
}
bool LOADER_IsValidCompAssetHeader(const char *data) {
	return data && data[0] == 'Z' && data[1] == 'H' && data[2] == 'R' && data[3] == 1; //*.ztx
}
size_t ZLIB_Decompress(byte *src, int uncompr_sz, byte *dest) {
	size_t ret = 0;
	z_stream s = {};
	char chunk[16384] = {};
	if (!inflateInit(&s)) {
		do {
			uInt a_in = s.avail_in = min(uInt(uncompr_sz), uInt(sizeof(chunk)));
			if (!a_in)
				break;
			s.next_in = (Bytef *)src;
			do {
				s.avail_out = sizeof(chunk);
				s.next_out = (Bytef *)chunk;
				int iret = inflate(&s, 0);
				if (iret != Z_OK && iret != Z_STREAM_END) {
					inflateEnd(&s);
					return 0;
				}
				int dec_sz = sizeof(chunk) - s.avail_out;
				memcpy(&dest[ret], chunk, dec_sz);
				ret += dec_sz;
			} while (!s.avail_out);
			uncompr_sz -= a_in;
			src += a_in;
		} while (uncompr_sz >= 0);
		inflateEnd(&s);
	}
	return ret;
}

struct Texture
{
	int field_0;
	int field_4;
	uint32_t name;
	int field_C;
	int align;
	uint16_t best_width;
	uint16_t best_height;
	int field_18;
	int field_1C;
	int field_20_5;
	int texture_time;
	int field_28;
	int field_2C;
	int total_bytes;
	char field_34;
	char field_35;
	char field_36_3;
	bool loaded;
	char from_level;
	char field_39_0;
	char to_level;
	char field_3B;
	int field_3C;
} g_Texture; //64 bytes

void glGenTextures(int n, uint32_t *dest) { /* generate texture names */ }
enum GLenum { GL_0 = 0, GL_TEXTURE_1D, GL_TEXTURE_2D = 3553, GL_TEXTURE_MAG_FILTER = 10240, GL_LINEAR = 9729, GL_TEXTURE_MIN_FILTER = 10241, GL_LINEAR_MIPMAP_LINEAR = 9987, GL_TEXTURE_BASE_LEVEL = 33084, GL_TEXTURE_MAX_LEVEL = 33085,
	GL_COMPRESSED_R11_EAC = 0x9270, GL_RGBA = 6408, GL_LUMINANCE = 6409, GL_R8 = 33321, GL_R16UI = 33332, GL_COMPRESSED_RGB8_ETC2 = 37492, GL_COMPRESSED_RGBA8_ETC2_EAC = 37496, GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG = 35840,
	GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG = 35841, GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG = 35842, GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG = 35843, GL_UNPACK_ALIGNMENT = 3317, GL_LUMINANCE_FLOAT32_ATI = 34840,
	GL_COMPRESSED_RGBA_S3TC_DXT1_EXT = 33777, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT = 33779,
	GL_TEXTURE_3D, GL_TEXTURE_1D_ARRAY, GL_TEXTURE_2D_ARRAY, GL_TEXTURE_RECTANGLE, GL_TEXTURE_CUBE_MAP, GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_BUFFER, GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_2D_MULTISAMPLE_ARRAY };
const int OIF_CNT = 13;
int gaGFXtoOGLaligns[OIF_CNT] = { 1, 4, 1, 4, 2, 8, 8, 8, 8, 0 };
GLenum gaGFXtoOGLInternalFormat[OIF_CNT] = { GL_RGBA, GL_RGBA, GL_R8, GL_LUMINANCE_FLOAT32_ATI, GL_R16UI, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, GL_0 };
bool gaIsCompressedInternalFormat[OIF_CNT] = {false, false, false, false, false, true, true , true , true , true , true , true , true };
bool GFX_IsCompressed(int formatIdx) { return gaIsCompressedInternalFormat[formatIdx]; }
void glBindTexture(GLenum target, uint32_t textureName) { /* bind a named texture to a texturing target */ }
void glTexParameteri(GLenum target, GLenum pname, int param) { /* set texture params */ }
void GFXAPI_CreateTexture(int mipMapLevelIdx) {
	glGenTextures(1, &g_Texture.name);
	glBindTexture(GL_TEXTURE_2D, g_Texture.name);
	int filter = GL_LINEAR;
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	if (mipMapLevelIdx) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mipMapLevelIdx - 1);
		filter = GL_LINEAR_MIPMAP_LINEAR;
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
	cerr << "GFXAPI_CreateTexture(" << mipMapLevelIdx << ")\n";
}
void glCompressedTexImage2D(GLenum target, int level, GLenum internalformat, int width, int height, int border, int dataBytes, const void *data) {
    //specify a two-dimensional texture image in a compressed format
	cerr << "glCompressedTexImage2D(level=" << level << ", internalformat=" << internalformat << ", width=" << width << ", height=" << height << ", dataBytes=" << dataBytes << ")\n";
}
void glPixelStorei(GLenum pname, int param) { /* Задает режимы хранения пикселей */ }
void GFXAPI_UpdateTexture(int level, int w, int h, int formatIdx, const void *data, int dataBytes) {
	int align = g_Texture.align;
	glBindTexture(GL_TEXTURE_2D, g_Texture.name);
	if (align < OIF_CNT)
		align = gaGFXtoOGLaligns[align];
	else
		align = 0;
	glPixelStorei(GL_UNPACK_ALIGNMENT, align);

	if (GFX_IsCompressed(formatIdx)) {
		GLenum internalformat = GL_0;
		if (formatIdx < OIF_CNT)
			internalformat = gaGFXtoOGLInternalFormat[formatIdx];
		glCompressedTexImage2D(GL_TEXTURE_2D, level, internalformat, w, h, 0 /* border */, dataBytes, data);
	} else {
		cerr << "GFXAPI_UpdateTexture: not compressed!!!\n";
	}
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
}
int g_nSkipMipCount = 0; //ultra gfx profile, load all qualities
int g_TextureTimeThisFrame = 0;
void GFX_CreateTextureFromTGAX(byte *data, size_t size) {
	const TGAX_HEADER *h = (const TGAX_HEADER *)data;
	uint16_t max_width = h->wWidth;
	uint16_t max_height = h->wHeight;
	int div = 0;
	while ((max_width >> div) > 4) ++div;
	int   nSkipMipCount = g_nSkipMipCount;
	if (div <= 6)
		nSkipMipCount = div + g_nSkipMipCount - 6;
	if (nSkipMipCount < 0)
		nSkipMipCount = 0;
	g_Texture.best_width = max_width >> nSkipMipCount;
	g_Texture.loaded = true;
	g_Texture.texture_time = g_TextureTimeThisFrame;
	g_Texture.best_height = max_height >> nSkipMipCount;
	g_Texture.from_level = div - nSkipMipCount;
	g_Texture.to_level = div - 1;
	g_Texture.align = 0;
	g_Texture.field_20_5 = 5;
	g_Texture.total_bytes = 0;
	g_Texture.field_39_0 = 0;
	g_Texture.field_36_3 = 3;

	byte *tdata = data + sizeof(TGAX_HEADER);
	int size_divider = 0;
	unsigned tdata_len, tdata_mult = 16, fmt_idx = 6;
	if (h->wType == 24) {
		tdata_mult = 8;
		fmt_idx = 5;
	}
	if (div > 0) do {
		int w = max_width >> size_divider;
		int h = max_height >> size_divider;
		tdata_len = tdata_mult * ((w + 3) >> 2) * ((h + 3) >> 2);
		if (size_divider >= nSkipMipCount) {
			GFXAPI_UpdateTexture(size_divider - nSkipMipCount /*level*/, w, h, fmt_idx, tdata, tdata_len);
			g_Texture.total_bytes += tdata_len;
		}
		++size_divider;
		tdata += tdata_len;
	} while (size_divider < div);

	//save dds:
	CreateOutputPath(".dds");
	ofstream out_dds(g_out_path, ios::binary);
	out_dds.exceptions(ofstream::failbit | ofstream::badbit);
	out_dds.write("DDS ", 4);
	DDS_HEADER dds_header = {};
	dds_header.dwSize = sizeof(dds_header);
	dds_header.dwFlags = 1 /*DDSD_CAPS*/ | 2 /*DDSD_HEIGHT*/ | 4 /*DDSD_WIDTH*/ | 0x1000 /*DDSD_PIXELFORMAT*/ | 0x20000 /*DDSD_MIPMAPCOUNT*/ /*| 0x80000 DDSD_LINEARSIZE*/;
	dds_header.dwHeight = max_height; dds_header.dwWidth = max_width;
	dds_header.dwMipMapCount = div;
	dds_header.dwCaps = 8 /*DDSCAPS_COMPLEX*/ | 0x1000 /*DDSCAPS_TEXTURE*/ | 0x400000 /*DDSCAPS_MIPMAP*/;
	dds_header.ddspf.dwSize = sizeof(DDS_PIXELFORMAT);
	dds_header.ddspf.dwFlags = 1 /*DDPF_ALPHAPIXELS*/ | 4 /*DDPF_FOURCC*/;
	dds_header.ddspf.dwFourCC = fmt_idx == 5 ? 0x31545844 /*DXT1*/ : 0x35545844 /*DXT5*/;
	/*dds_header.ddspf.dwRGBBitCount;
	dds_header.ddspf.dwRBitMask;
	dds_header.ddspf.dwGBitMask;
	dds_header.ddspf.dwBBitMask;
	dds_header.ddspf.dwABitMask;*/
	out_dds.write((const char*)&dds_header, sizeof(dds_header));
	out_dds.write((const char*)data + sizeof(TGAX_HEADER), size - sizeof(TGAX_HEADER));
	cerr << g_out_path << ": written OK\n";
}
size_t ZLIB_Compress(const void *src, size_t src_len, void *dest, size_t dest_len) {
	uLongf ret = (uLongf)dest_len;
	z_stream s = {};
	if (Z_OK == compress((Bytef *)dest, &ret, (const Bytef *)src, (uLongf)src_len)) {
		return ret;
	}
	return 0;
}
int main(int argc, char **argv) {
	if (argc < 2) {
		cerr << "Zwift tgax utility.\nUsage example:\n1. tgax.exe file.ztx\n2. tgax.exe file.tgax\n3. tgax.exe file.dds\n";
		return 1;
	}
	for (int i = 1; i < argc; i++) {
		const char* phase = "ifstream open";
		g_in_path = argv[i];
		g_out_path.clear();
		try {
			ifstream in;
			in.exceptions(ifstream::failbit | ifstream::badbit);
			in.open(g_in_path, ios::binary);

#if 0 // glob for *.tgax -> most of the files are 0 0 2 0 0 0 0 0 0 0 0 0 0 0 0 0 24 0 (as 4iii.tgax) or 0 0 2 0 0 0 0 0 0 0 0 0 0 0 0 0 32 8 (as 100kph.tgax)
//!!! but 0 0 2 0 0 0 0 0 0 0 0 1 0 0 0 0 32 40 = one file: Select_Square.tgax and
//!!!     0 0 2 0 0 0 0 0 0 0 0 0 0 0 0 0 24 32 = ~10 files like Coach_Ride_00.tgax
			union { TGAX_HEADER header18; char h18[18]; } u;
			in.read(u.h18, sizeof(u));
			u.header18.wHeight = 0;
			u.header18.wWidth = 0;
			for (char c: u.h18) {
				cout << int(c) << ' ';
			}
			cout << g_in_path << endl;
			continue;
#endif
			in.seekg(0, ios::end);
			int in_size = int(in.tellg());
			in.seekg(0, ios::beg);

			phase = "ifstream read header";
			char header[4];
			in.read(header, sizeof(header));
			if (LOADER_IsValidCompAssetHeader(header)) { //GFX_CreateTextureFromZTX(data, len, -1)
				phase = "ztx decompr_size";
				size_t decompr_size = -1, act_decompr_size;
				in.read((char*)&decompr_size, sizeof(decompr_size));
				cerr << g_in_path << ": ValidCompAssetHeader(ztx), decompr_size: " << decompr_size << endl;

				phase = "ztx compr_size";
				in.seekg(16, ios::beg);

				phase = "ztx alloc [de]compr_data";
				int compr_size = in_size - 16;
				vector<byte> decompr_data(decompr_size), compr_data(compr_size);

				phase = "ztx read compr_data";
				in.read((char*)&compr_data[0], compr_size);

				phase = "ztx ZLIB_Decompress";
				act_decompr_size = ZLIB_Decompress(&compr_data[0], compr_size, &decompr_data[0]);
				if (act_decompr_size > sizeof(TGAX_HEADER) && act_decompr_size == decompr_size) {
					phase = "ztx save tgax";
					CreateOutputPath(".tgax");
					ofstream out_tgax(g_out_path, ios::binary);
					out_tgax.exceptions(ofstream::failbit | ofstream::badbit);
					out_tgax.write((const char*)&decompr_data[0], decompr_size);
					cerr << g_out_path << ": written OK\n";
#if 0 // glob for *.ztx -> most of the files are 0 0 2 0 0 0 0 0 0 0 0 0 0 0 0 0 24 0 or 0 0 2 0 0 0 0 0 0 0 0 0 0 0 0 0 32 8
//!!! but the only file CeramicSpeedSramXX1.ztx is 0 0 3 0 0 0 0 0 0 0 0 0 0 0 0 0 24 8
//!   and DuraAce9200CompNml.ztx: ValidCompAssetHeader(ztx), decompr_size: 1398098 - maybe the file is broken
//		Zwift tgax utility@GFX_CreateTextureFromZTX() : size mismatch(0)

					union { TGAX_HEADER header18; char h18[18]; } u;
					memcpy(u.h18, &decompr_data[0], 18);
					u.header18.wHeight = 0;
					u.header18.wWidth = 0;
					for (char c : u.h18) {
						cout << int(c) << ' ';
					}
					cout << g_in_path << endl;
					continue;
#endif
					GFX_CreateTextureFromTGAX(&decompr_data[0], decompr_size);
				} else {
					cerr << "Zwift tgax utility@GFX_CreateTextureFromZTX(): size mismatch (" << act_decompr_size << ")\n";
					return 4;
				}
			} else if (header[0] == 'D' && header[1] == 'D' && header[2] == 'S' && header[3] == ' ') {
				phase = "dds read header";
				DDS_HEADER dds_header = {};
				in.read((char*)&dds_header, sizeof(dds_header));
				phase = "dds read data";
				vector<byte> dds_data(in_size - 4 - sizeof(dds_header));
				in.read((char*)&dds_data[0], dds_data.size());
				phase = "dds saveas tgax";
				CreateOutputPath(".tgax");
				ofstream out_tgax(g_out_path, ios::binary);
				out_tgax.exceptions(ofstream::failbit | ofstream::badbit);
				TGAX_HEADER tgax_header = {};
				tgax_header.wType = dds_header.ddspf.dwFourCC == 0x31545844 /*DXT1*/ ? 24 : 0x820 /*what is it?*/;
				tgax_header.wDummy2 = 2;
				tgax_header.wWidth = dds_header.dwWidth;
				tgax_header.wHeight = dds_header.dwHeight;
				out_tgax.write((char*)&tgax_header, sizeof(tgax_header));
				out_tgax.write((char*)&dds_data[0], dds_data.size());
				cerr << g_out_path << ": written OK\n";

				phase = "dds saveas ztx";
				CreateOutputPath(".ztx");
				ofstream out_ztx(g_out_path, ios::binary);
				out_ztx.exceptions(ofstream::failbit | ofstream::badbit);
				out_ztx.write("ZHR\x1", 4);
				size_t decompr_size = dds_data.size() + sizeof(tgax_header);
				out_ztx.write((char*)&decompr_size, sizeof(decompr_size));
				out_ztx.write("\0\0\0\0", 4);
				vector<byte> ztx_buf(decompr_size * 2);
				dds_data.insert(dds_data.begin(), (const byte *)&tgax_header, (const byte *)(& tgax_header + 1));
				size_t compr_size = ZLIB_Compress(&dds_data[0], dds_data.size(), &ztx_buf[0], ztx_buf.size());
				out_ztx.write((char *)& ztx_buf[0], compr_size);
				cerr << g_out_path << " ratio=" << compr_size * 100 / decompr_size << "%, written OK\n";
			} else {
				phase = "tgax alloc decompr_data";
				vector<byte> decompr_data(in_size);
				phase = "tgax read file";
				memcpy(&decompr_data[0], header, sizeof(header));
				in.read((char*)&decompr_data[sizeof(header)], in_size - sizeof(header));
				if (in_size > sizeof(TGAX_HEADER)) {
					GFX_CreateTextureFromTGAX(&decompr_data[0], in_size);
				}
				else {
					cerr << "Zwift tgax utility@GFX_CreateTextureFromTGAX(): size mismatch (" << in_size << ")\n";
					return 3;
				}
			}

		}
		catch (exception e) {
			cerr << "Zwift tgax utility@" << phase << ": in=" << g_in_path << " out=" << g_out_path << ": exception: " << e.what() << endl;
			return 2;
		}
	}
	return 0;
}
