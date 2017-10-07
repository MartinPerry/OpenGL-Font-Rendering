#ifndef CHARACTER_EXTRACTOR_H
#define CHARACTER_EXTRACTOR_H


#ifdef _WIN32
#include "./win_dirent.h"
#else 
#include <dirent.h>
#endif

#include <set>
#include <cstdint>
#include <vector>
#include <unordered_map>

#include <string>
#include <fstream>
#include <streambuf>
#include <sstream>
#include <iomanip>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "./Externalncludes.h"

//#define USE_TEXT_FILE

/// <summary>
/// Helper class for character extraction and merging from different
/// font files
/// This will generate script for "pyftsubset" and "pyftmerge" from "fonttools"
/// </summary>
class CharacterExtractor
{
public:
	//CharacterExtractor(const std::vector<std::string> & inputTTF, const std::string & outputTTF);
	CharacterExtractor(const std::vector<std::string> & fontWorkingDir, const std::string & outputTTF);
	~CharacterExtractor();

	void SetOutputDir(const std::string & outputDir);
	void AddText(const utf8_string & strUTF8);
	void AddTextFromFile(const std::string & filePath);
	void AddDirectory(const std::string & dirPath);

	void RemoveChar(char32_t c);

	void GenerateScript(const std::string & scriptFileName = "run.sh");

protected:
	
	std::string outputDir;
	std::vector<std::string> inputTTF;
	std::string outputTTF;
	std::set<char32_t> characters;

	std::unordered_map<std::string, FT_Face> faces;
	std::unordered_map<std::string, int> facesLineOffset;
	
	FT_Library library;
	

	void InitFreeType();
	std::string BaseName(std::string const & path);
	bool HasEnding(std::string const &fullString, std::string const &ending);
};

/*
CharacterExtractor::CharacterExtractor(const std::vector<std::string> & inputTTF, const std::string & outputTTF)
	: outputDir(""), inputTTF(inputTTF), outputTTF(outputTTF), library(nullptr)
{
	this->InitFreeType();
};
*/

CharacterExtractor::CharacterExtractor(const std::vector<std::string> & fontDir, const std::string & outputTTF)
	: outputDir(""), outputTTF(outputTTF), library(nullptr)
{
	
	for (auto d : fontDir)
	{
		DIR * dir = opendir(d.c_str());

		if (dir == nullptr)
		{
			printf("Failed to open dir %s\n", d.c_str());
			throw std::invalid_argument("Unknown dir");;
		}

		struct dirent * ent;
		std::string fullPath;

		/* print all the files and directories within directory */
		while ((ent = readdir(dir)) != nullptr)
		{
			if ((strcmp(ent->d_name, ".") == 0) || (strcmp(ent->d_name, "..") == 0))
			{
				continue;
			}
			if (ent->d_type == DT_REG)
			{

				//printf ("%s (file)\n", ent->d_name);
				fullPath = d;
#ifdef _WIN32
				fullPath = dir->patt; //full path using Windows dirent
				fullPath = fullPath.substr(0, fullPath.length() - 1);
#else
				if (fullPath[fullPath.length() - 1] != '/')
				{
					fullPath += "/";
				}
#endif				
				fullPath += ent->d_name;


				if ((this->HasEnding(ent->d_name, ".ttf")) || (this->HasEnding(ent->d_name, ".otf")))
				{
					this->inputTTF.push_back(fullPath);
				}
			}
		}


		closedir(dir);
	}

	this->InitFreeType();
};

/// <summary>
/// dtor
/// </summary>
CharacterExtractor::~CharacterExtractor()
{
	for (auto & c : faces)
	{
		
		FT_Done_Face(c.second);
		c.second = nullptr;
	}
	FT_Done_FreeType(this->library);
	this->library = nullptr;
	
};

void CharacterExtractor::SetOutputDir(const std::string & outputDir)
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

	for (auto & s : this->inputTTF)
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
		
		int dpi = 260;
		int size = 12;
		if (FT_Set_Char_Size(fontFace, 0, size * 64, dpi, dpi))
		{
			MY_LOG_ERROR("Failed to set font size in points");
			return;
		}

		int newLineOffset = static_cast<int>(fontFace->size->metrics.height / 64);

		this->faces[s] = fontFace;
		this->facesLineOffset[s] = newLineOffset;
	}
};

void CharacterExtractor::RemoveChar(char32_t c)
{
	this->characters.erase(c);
};

/// <summary>
/// Add text - extract all UNICODE letters from it
/// </summary>
/// <param name="strUTF8"></param>
void CharacterExtractor::AddText(const utf8_string & strUTF8)
{
	for (auto c : strUTF8)
	{
		this->characters.insert(c);		
	}
};

/// <summary>
/// Add content of file
/// </summary>
/// <param name="filePath"></param>
void CharacterExtractor::AddTextFromFile(const std::string & filePath)
{
	std::ifstream t(filePath);
	std::string str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());

	

	this->AddText(utf8_string::build_from_escaped(str.c_str()));
};

/// <summary>
/// Add all files in directory and its subdirectories
/// </summary>
/// <param name="dirPath"></param>
void CharacterExtractor::AddDirectory(const std::string & dirPath)
{
	DIR * dir = opendir(dirPath.c_str());

	if (dir == nullptr)
	{
		printf("Failed to open dir %s\n", dirPath.c_str());
		return;
	}

	struct dirent * ent;
	std::string newDirName;
	std::string fullPath;


	/* print all the files and directories within directory */
	while ((ent = readdir(dir)) != nullptr)
	{
		if ((strcmp(ent->d_name, ".") == 0) || (strcmp(ent->d_name, "..") == 0))
		{
			continue;
		}
		switch (ent->d_type)
		{
		case DT_REG:

			//printf ("%s (file)\n", ent->d_name);
			fullPath = dirPath;
#ifdef _WIN32
			fullPath = dir->patt; //full path using Windows dirent
			fullPath = fullPath.substr(0, fullPath.length() - 1);
#else
			if (fullPath[fullPath.length() - 1] != '/')
			{
				fullPath += "/";
			}
#endif				
			fullPath += ent->d_name;

			
			//printf("Full file path: %s\n", fullPath.c_str());
			
			this->AddTextFromFile(fullPath);
			break;

		case DT_DIR:			
			newDirName = dirPath;
			if (newDirName[newDirName.length() - 1] != '/')
			{
				newDirName += "/";
			}
			newDirName += ent->d_name;
			this->AddDirectory(newDirName);

			break;

		default:
			//printf ("%s:\n", ent->d_name);
			break;
		}
	}


	closedir(dir);
};

std::string CharacterExtractor::BaseName(std::string const & path)
{
	return path.substr(path.find_last_of("/\\") + 1);
};

bool CharacterExtractor::HasEnding(std::string const &fullString, std::string const &ending)
{
	if (fullString.length() >= ending.length()) 
	{
		return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
	}
	else 
	{
		return false;
	}
};

/// <summary>
/// This will generate script for "pyftsubset" and "pyftmerge" from "fonttools"
/// https://github.com/fonttools/fonttools
/// </summary>
void CharacterExtractor::GenerateScript(const std::string & scriptFileName)
{
	bool useTextFile = true;

	std::unordered_map<std::string, std::string> glyphsCodes;
	std::unordered_map<std::string, utf8_string> glyphsUTF8;
	for (auto & s : this->faces)
	{
		glyphsCodes[s.first] = "";
		glyphsUTF8[s.first] = " ";
	}


	int usedFontsCount = 0;

	
	for (auto c : this->characters)
	{
		if (c <= 32)
		{
			//non printable
			//space = 32
			continue;
		}
		
		std::string faceName = "";
		bool found = false;
		int minOffset = std::numeric_limits<int>::max();

		for (auto & s : this->faces)
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

		if (found == false)
		{
			printf("Character %zu not found (FT_Get_Char_Index)\n", static_cast<uint32_t>(c));
			continue;
		}
		
		std::stringstream s;
		s << std::hex << c;
		std::string result(s.str());

		std::string g = "U+";
		g += std::string(4 - result.length(), '0') + result;
		g += "\n";		
		glyphsCodes[faceName] += g;


		glyphsUTF8[faceName].push_back(c);
	}

	//==============================================
	
#ifdef USE_TEXT_FILE
	auto & glyphs = glyphsUTF8;
#else
	auto & glyphs = glyphsCodes;
#endif

	for (auto & s : glyphs)
	{
		if (s.second.length() > 1)
		{
			usedFontsCount++;
		}
	}

	std::string remove = "";

	std::string e = "#!/bin/sh";
	e += '\n';

	std::string lastFileName = "";
	for (auto & s : glyphs)
	{
		if (s.second.length() <= 1)
		{
			continue;
		}

		std::string glyphsFileName = "glyphs_";
		glyphsFileName += this->BaseName(s.first);
		glyphsFileName += ".txt";


#ifdef USE_TEXT_FILE
		FILE * f = fopen((outputDir + glyphsFileName).c_str(), "wb");
		uint8_t smarker[3];
		smarker[0] = 0xEF;
		smarker[1] = 0xBB;
		smarker[2] = 0xBF;
		fwrite(smarker, sizeof(uint8_t), 3, f);
		fwrite(s.second.c_str() + 1, sizeof(char), s.second.size(), f); //+ 1 skip first " "
		fclose(f);
#else
		std::ofstream file(outputDir + glyphsFileName);
		file << s.second;
#endif

		std::string subsetFileName = "subset_";
		subsetFileName += this->BaseName(s.first);
		
		lastFileName = subsetFileName;

		//https://github.com/fonttools/fonttools/issues/336
		
		e += "pyftsubset \"";
		e += s.first;
		e += "\"";
		e += " --output-file=\"";
		e += subsetFileName;
		e += "\"";		
#ifdef USE_TEXT_FILE
		e += " --text-file=\"";
		e += glyphsFileName;
#else
		e += " --unicodes-file=\"";
		e += glyphsFileName;
#endif
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
		for (auto & s : glyphs)
		{
			emSizes.insert(this->faces[s.first]->units_per_EM);
		}

		std::set<std::string> types;
		for (auto & s : glyphs)
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
		for (auto & s : glyphs)
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
			printf("Type: %s\n", type.c_str());
			for (auto emSize : emSizes)
			{
				printf("EmSize: %i\n", emSize);
				for (auto lineOffset : lineOffsets)
				{
					printf("LineOffset: %i\n", lineOffset);

					int count = 0;					
					std::string merge = "pyftmerge ";
					std::string lastFileName = "";
					for (auto & s : glyphs)
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

					printf("FilesCount: %i\n", count);

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
