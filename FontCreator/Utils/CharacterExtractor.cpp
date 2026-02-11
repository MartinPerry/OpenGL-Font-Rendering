#include "./CharacterExtraxtor.h"

#ifdef USE_ICU_LIBRARY

#include <fstream>
#include <streambuf>
#include <sstream>
#include <iomanip>
#include <regex>

#include <algorithm>
#include <filesystem>


#include "../Unicode/ICUUtils.h"
#include "../Unicode/BidiHelper.h"


CharacterExtractor::CharacterExtractor(const std::vector<std::string>& fontDir, const std::string& outputTTF) :
	outputDir(""),
	outputTTF(outputTTF),
	library(nullptr)
{
	for (const auto& d : fontDir)
	{
		std::filesystem::path dirPath(d);

		for (const auto& dirEntry : std::filesystem::directory_iterator(dirPath))
		{
			if (dirEntry.is_regular_file() == false)
			{
				continue;
			}

			auto ext = dirEntry.path().extension().string();
			if ((ext == ".ttf") || (ext == ".otf"))
			{
				this->inputTTF.push_back(dirEntry.path().string());
			}			
		}
	}
		

	this->InitFreeType();
}

/// <summary>
/// dtor
/// </summary>
CharacterExtractor::~CharacterExtractor()
{
	this->Release();
}

void CharacterExtractor::Release()
{
	for (auto& c : faces)
	{

		FT_Done_Face(c.second);
		c.second = nullptr;
	}
	this->faces.clear();

	if (this->library != nullptr)
	{
		FT_Done_FreeType(this->library);
		this->library = nullptr;
	}

	this->outputDir.clear();
	this->inputTTF.clear();
	this->outputTTF.clear();
	this->characters.clear();

	this->faces.clear();
	this->facesLineOffset.clear();
}

std::vector<char32_t> CharacterExtractor::GetAllCharacters() const
{
	return std::vector<char32_t>(characters.begin(), characters.end());
}

void CharacterExtractor::SetOutputDir(const std::string& outputDir)
{
	this->outputDir = outputDir;
}

/// <summary>
/// Init Freetype library for font checks
/// </summary>
void CharacterExtractor::InitFreeType()
{
	if (this->library == nullptr)
	{
		if (FT_Init_FreeType(&this->library))
		{
			MY_LOG_ERROR("Failed to initialize FreeType library.");
			return;
		}
	}

	for (auto& s : this->inputTTF)
	{
		FT_Face fontFace;

		FT_Error error = FT_New_Face(this->library, s.c_str(), 0, &fontFace);
		if (error == FT_Err_Unknown_File_Format)
		{
			MY_LOG_ERROR("Failed to initialize Font Face %s. File not supported\n", s.c_str());
			return;
		}
		else if (error)
		{
			MY_LOG_ERROR("Failed to initialize Font Face %s.\n", s.c_str());
			return;
		}

		FT_Select_Charmap(fontFace, FT_ENCODING_UNICODE);


		if (fontFace->num_fixed_sizes != 0)
		{
			int size = fontFace->available_sizes[0].size / 64;
			if (FT_Set_Pixel_Sizes(fontFace, 0, size))
			{
				MY_LOG_ERROR("Failed to set font size in points");
				return;
			}
		}
		else
		{
			int dpi = 260;
			int size = 12;

			if (FT_Set_Char_Size(fontFace, 0, size * 64, dpi, dpi))
			{
				MY_LOG_ERROR("Failed to set font size in points");
				return;
			}
		}

		int newLineOffset = static_cast<int>(fontFace->size->metrics.height / 64);

		this->faces[s] = fontFace;
		this->facesLineOffset[s] = newLineOffset;
	}
}

//=======================================================================================
// Adding data
//=======================================================================================

/// <summary>
/// Remove single character
/// </summary>
/// <param name="c"></param>
void CharacterExtractor::RemoveChar(char32_t c)
{
	this->characters.erase(c);
}

/// <summary>
/// Add single character
/// </summary>
/// <param name="c"></param>
void CharacterExtractor::AddCharacter(char32_t c)
{
	this->characters.insert(c);
}

/// <summary>
/// Add all ASCII letters
/// A-Z
/// a-z
/// </summary>
void CharacterExtractor::AddAllAsciiLetters()
{
	for (char c = 'a'; c <= 'z'; c++)
	{
		this->AddCharacter(c);
	}

	for (char c = 'A'; c <= 'Z'; c++)
	{
		this->AddCharacter(c);
	}
}


/// <summary>
/// Add text - extract all UNICODE letters from it
/// </summary>
/// <param name="str"></param>
void CharacterExtractor::AddText(const StringUtf8& str)
{
	char32_t c;

	//Add original text
	//in case that Bidi replace some Arabic chars with some others
	//in arabic shaping
	{
		auto it = CustomIteratorCreator::Create(str);
		while ((c = it.GetCurrentAndAdvance()) != it.DONE)
		{
			this->AddCharacter(c);
		}
	}

	//now add the same text with Bidi	
	//that will use arabic shaping
	StringUtf8 bidiStr = BidiHelper::ConvertOneLine(str);

	auto it = CustomIteratorCreator::Create(bidiStr);

	while ((c = it.GetCurrentAndAdvance()) != it.DONE)
	{
		this->AddCharacter(c);
	}
}

/// <summary>
/// Add content of file
/// </summary>
/// <param name="filePath"></param>
void CharacterExtractor::AddTextFromFile(const std::string& filePath)
{
	std::ifstream t(filePath);
	std::string str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());

	//UnicodeString::fromUTF8(u8"\\u00E9").unescape()
	//results in a UnicodeString containing é, not the literal characters \, u, 0, 0, E, 9.

	auto uniStr = icu::UnicodeString::fromUTF8(str).unescape();
	auto str8 = IcuUtils::to_u8string(uniStr);

	this->AddText(str8);
}

/// <summary>
/// Add content of JSON file
/// Content is parsed via parseCallback
/// </summary>
/// <param name="filePath"></param>
/// <param name="parseCallback"></param>
void CharacterExtractor::AddTextFromFile(const std::string& filePath,
	std::function<void(const char * str, CharacterExtractor* ce)> parseCallback)
{
	std::ifstream t(filePath);
	std::string str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
		
	parseCallback(str.c_str(), this);
	
}

/// <summary>
/// Add all files in directory and its subdirectories
/// </summary>
/// <param name="dirPath"></param>
void CharacterExtractor::AddDirectory(const std::string& dirPath)
{
	
		
	for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(dirPath))
	{
		if (dirEntry.is_regular_file() == false)
		{
			continue;
		}

		this->AddTextFromFile(dirEntry.path().string());
	}	
}

//=======================================================================================
//=======================================================================================
//=======================================================================================

std::string CharacterExtractor::BaseName(std::string const& path)
{
	return path.substr(path.find_last_of("/\\") + 1);
}

bool CharacterExtractor::HasEnding(std::string const& fullString, std::string const& ending)
{
	if (fullString.length() >= ending.length())
	{
		return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
	}
	else
	{
		return false;
	}
}

void CharacterExtractor::AppendCodePoint(StringUtf8& utf8_str, char32_t cp) const
{
	if (cp <= 0x7F) {
		utf8_str.push_back(static_cast<char8_t>(cp));
	}
	else if (cp <= 0x7FF) {
		utf8_str.push_back(static_cast<char8_t>(0xC0 | ((cp >> 6) & 0x1F)));
		utf8_str.push_back(static_cast<char8_t>(0x80 | (cp & 0x3F)));
	}
	else if (cp <= 0xFFFF) {
		utf8_str.push_back(static_cast<char8_t>(0xE0 | ((cp >> 12) & 0x0F)));
		utf8_str.push_back(static_cast<char8_t>(0x80 | ((cp >> 6) & 0x3F)));
		utf8_str.push_back(static_cast<char8_t>(0x80 | (cp & 0x3F)));
	}
	else if (cp <= 0x10FFFF) {
		utf8_str.push_back(static_cast<char8_t>(0xF0 | ((cp >> 18) & 0x07)));
		utf8_str.push_back(static_cast<char8_t>(0x80 | ((cp >> 12) & 0x3F)));
		utf8_str.push_back(static_cast<char8_t>(0x80 | ((cp >> 6) & 0x3F)));
		utf8_str.push_back(static_cast<char8_t>(0x80 | (cp & 0x3F)));
	}
}

CharacterExtractor::GlyphsInfo CharacterExtractor::BuildGlyphs()
{
	CharacterExtractor::GlyphsInfo gi;
	
	for (auto& s : this->faces)
	{
		gi.glyphsCodes[s.first] = "";
		gi.glyphsUnicode[s.first] = u8"";
	}

	for (auto c : this->characters)
	{
		if (c == 32)
		{
			//ignore spaces - will  be added separatelly
			continue;
		}
		if (c < 32)
		{
			//non printable			
			continue;
		}

		//found faceName that has minimal offset
		FaceName faceName = "";
		bool found = false;
		int minOffset = std::numeric_limits<int>::max();

		for (const auto& s : this->faces)
		{

			FT_UInt ci = FT_Get_Char_Index(s.second, c);

			if (ci == 0)
			{
				continue;
			}

			found = true;

			int offset = this->facesLineOffset[s.first];
			if (offset < minOffset)
			{
				minOffset = offset;

				faceName = s.first;
			}
		}

		std::stringstream s;
		s << std::hex << static_cast<uint32_t>(c);
		std::string result(s.str());

		if (found == false)
		{
			printf("Character %zu (%s) not found (FT_Get_Char_Index)\n", static_cast<uint32_t>(c), result.c_str());
			continue;
		}


		if (result.length() > 4)
		{
			//32bit Unicode letters

			std::string g = "U+";
			g += std::string(8 - result.length(), '0') + result;
			g += "\n";
			gi.glyphsCodes[faceName] += g;

			printf("32bit unicode (%s)\n", result.c_str());
		}
		else
		{
			//16bit Unicode letters

			std::string g = "U+";
			g += std::string(4 - result.length(), '0') + result;
			g += "\n";
			gi.glyphsCodes[faceName] += g;
		}

		//todo - will this work?
		AppendCodePoint(gi.glyphsUnicode[faceName], c);
	}

	return gi;
}

/// <summary>
/// Add space " " from every font face
/// </summary>
/// <param name="gi"></param>
void CharacterExtractor::AddSpaceToAllGlyphs(GlyphsInfo& gi)
{
	int32_t c = 32;
	
	std::stringstream s;
	s << std::hex << c;
	std::string result(s.str());
			
	std::set<int> addedOffsets;

	for (auto& s : this->faces)
	{
		FT_UInt ci = FT_Get_Char_Index(s.second, c);

		if (ci == 0)
		{
			continue;
		}

		auto & tmp = gi.glyphsCodes[s.first];

		if (tmp.length() <= 1)
		{
			continue;
		}

		int offset = this->facesLineOffset[s.first];
		
		if (addedOffsets.find(offset) != addedOffsets.end())
		{
			//prevent to put space in the font faces that will be merged
			//font faces are merged by lineOffsets
			continue;
		}

		std::string g = "U+";
		g += std::string(4 - result.length(), '0') + result;
		g += "\n";

		if (tmp.find(g) != std::string::npos)
		{
			continue;
		}

		addedOffsets.insert(offset);
		gi.glyphsCodes[s.first] += g;

		gi.glyphsUnicode[s.first] += c;
	}
}

/// <summary>
/// Calculate number of unique fonts
/// </summary>
/// <param name="gi"></param>
/// <returns></returns>
int CharacterExtractor::GetFontsCount(const GlyphsInfo& gi)
{
	int usedFontsCount = 0;
	for (auto& s : gi.glyphsCodes)
	{
		if (s.second.length() > 1)
		{
			usedFontsCount++;
		}
	}

	return usedFontsCount;
}

/// <summary>
/// This will generate script for "pyftsubset" and "pyftmerge" from "fonttools"
/// https://github.com/fonttools/fonttools
/// </summary>
void CharacterExtractor::GenerateScript(const std::string& scriptFileName, bool useLinuxPath)
{
	GlyphsInfo gi = this->BuildGlyphs();
	this->AddSpaceToAllGlyphs(gi);
	

	//==============================================

	int usedFontsCount = this->GetFontsCount(gi);
	

	std::string remove = "";

	std::string e = "#!/bin/bash";
	e += '\n';

	std::string lastFileName = "";
	for (auto& s : gi.glyphsCodes)
	{
		if (s.second.length() <= 1)
		{
			continue;
		}

		std::string glyphsFileName = "glyphs_";
		glyphsFileName += this->BaseName(s.first);
		glyphsFileName += ".txt";



		std::ofstream file(outputDir + glyphsFileName);
		file << s.second;


		std::string subsetFileName = "subset_";
		subsetFileName += this->BaseName(s.first);

		lastFileName = subsetFileName;

		std::string inputFileName = s.first;
		if (useLinuxPath)
		{
			inputFileName = std::regex_replace(inputFileName, std::regex("\\\\"), "/");
			inputFileName = std::regex_replace(inputFileName, std::regex("D:"), "/mnt/d");
		}



		//https://github.com/fonttools/fonttools/issues/336

		e += "pyftsubset \"";
		e += inputFileName;
		e += "\"";
		e += " --output-file=\"";
		e += subsetFileName;
		e += "\"";
		e += " --unicodes-file=\"";
		e += glyphsFileName;
		e += "\"";
		e += " --layout-features='*' --glyph-names --symbol-cmap --legacy-cmap --notdef-glyph --notdef-outline --recommended-glyphs ";
		e += " --name-IDs='*' --name-legacy ";
		e += "  --no-recalc-bounds ";
		//e += "  --recalc-bounds ";
		e += '\n';

		remove += "rm ";
		remove += subsetFileName;
		remove += '\n';

		remove += "rm ";
		remove += glyphsFileName;
		remove += '\n';
	}

	if (usedFontsCount > 1)
	{
		std::set<int> emSizes;
		for (auto& s : gi.glyphsCodes)
		{
			emSizes.insert(this->faces[s.first]->units_per_EM);
		}

		std::set<std::string> types;
		for (auto& s : gi.glyphsCodes)
		{
			if (HasEnding(s.first, "ttf"))
			{
				types.insert("ttf");
			}
			else if (HasEnding(s.first, "otf"))
			{
				types.insert("otf");
			}
		}

		const int LINE_OFFSET_THRESSHOLD = 1;
		std::set<int> tmpLineOffsets;
		for (auto& s : gi.glyphsCodes)
		{
			tmpLineOffsets.insert(this->facesLineOffset[s.first]);
		}

		std::set<int> lineOffsets;

		std::vector<int> v(tmpLineOffsets.begin(), tmpLineOffsets.end());
		std::sort(v.begin(), v.end());

		int lastOffset = v[0];
		lineOffsets.insert(v[0]);

		for (size_t i = 1; i < v.size(); i++)
		{
			if (std::abs(lastOffset - v[i]) < LINE_OFFSET_THRESSHOLD)
			{
				continue;
			}

			lastOffset = v[i];
			lineOffsets.insert(v[i]);
		}


		std::set<std::string> used;

		for (auto type : types)
		{
			//printf("Type: %s\n", type.c_str());
			for (auto emSize : emSizes)
			{
				//printf("EmSize: %i\n", emSize);
				for (auto lineOffset : lineOffsets)
				{
					//printf("LineOffset: %i\n", lineOffset);

					int count = 0;
					std::string merge = "pyftmerge ";
					std::string lastFileName = "";
					for (auto& s : gi.glyphsCodes)
					{
						if (s.second.length() <= 1)
						{
							continue;
						}

						if (used.find(s.first) != used.end())
						{
							continue;
						}

						if (this->faces[s.first]->units_per_EM != emSize)
						{
							continue;
						}
						if (HasEnding(s.first, type) == false)
						{
							continue;
						}
						if (std::abs(this->facesLineOffset[s.first] - lineOffset) >= LINE_OFFSET_THRESSHOLD)
						{
							continue;
						}

						used.insert(s.first);

						std::string subsetFileName = "subset_";
						subsetFileName += this->BaseName(s.first);

						lastFileName = subsetFileName;

						merge += subsetFileName;
						merge += " ";
						count++;
					}

					//printf("FilesCount: %i\n", count);

					if (count == 0)
					{
						continue;
					}
					if (count == 1)
					{
						e += "mv ";
						e += lastFileName;
						e += " ";
					}
					else
					{
						merge += " ";
						merge += '\n';

						e += merge;
						e += "mv ";
						e += "merged.";
						e += type;
						e += " ";
					}


					e += this->outputTTF;
					e += "_";
					e += std::to_string(emSize);
					e += "_";
					e += std::to_string(lineOffset);
					e += ".";
					e += type;

					e += '\n';
				}
			}
		}

	}
	else
	{
		e += "mv ";
		e += lastFileName;
		e += " ";
		e += this->outputTTF;
	}
	e += '\n';

	e += remove;

	std::ofstream file = std::ofstream(outputDir + scriptFileName, std::ios_base::binary | std::ios_base::out);

	file << e;

	//pyftsubset arial_unicode.ttf --unicodes-file = "chars.txt" 
	//--layout-features = '*' 
	//--glyph-names --symbol-cmap --legacy-cmap --notdef-glyph --notdef-outline --recommended-glyphs 
	//--name-IDs = '*' --name-legacy --output-file = "ii/ll.ttf"

};

#endif