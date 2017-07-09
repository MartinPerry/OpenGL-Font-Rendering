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

	void GenerateScript(const std::string & scriptFileName = "run.sh");

protected:
	
	std::string outputDir;
	std::vector<std::string> inputTTF;
	std::string outputTTF;
	std::set<uint32_t> characters;

	std::unordered_map<std::string, FT_Face> faces;
	
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
			printf("Failed to initialize FreeType library.");
			return;
		}
	}	

	for (auto & s : this->inputTTF)
	{
		FT_Face fontFace;

		FT_Error error = FT_New_Face(this->library, s.c_str(), 0, &fontFace);
		if (error == FT_Err_Unknown_File_Format)
		{
			printf("Failed to initialize Font Face %s. File not supported\n", s.c_str());
			return;
		}
		else if (error)
		{
			printf("Failed to initialize Font Face %s.\n", s.c_str());
			return;
		}

		FT_Select_Charmap(fontFace, FT_ENCODING_UNICODE);
		
		

		this->faces[s] = fontFace;

	}
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
	std::unordered_map<std::string, std::string> glyphs;
	for (auto & s : this->faces)
	{
		glyphs[s.first] = "";
	}


	int usedFontsCount = 0;

	for (auto c : this->characters)
	{
		std::stringstream s;
		s << std::hex << c;
		std::string result(s.str());

		std::string found = "";
		for (auto & s : this->faces)
		{
			FT_UInt ci = FT_Get_Char_Index(s.second, c);

			if (ci == 0)
			{								
				continue;
			}

			found = s.first;
			break;
		}

		if (found == "")
		{
			printf("Character %s not found (FT_Get_Char_Index)\n", result.c_str());
			continue;
		}

		std::string g = "U+";
		g += std::string(4 - result.length(), '0') + result;
		g += "\n";

		glyphs[found] += g;

	}

	for (auto & s : glyphs)
	{
		if (s.second.size() > 0)
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

		std::ofstream file(outputDir + glyphsFileName);
		file << s.second;



		std::string subsetFileName = "subset_";
		subsetFileName += this->BaseName(s.first);
		
		lastFileName = subsetFileName;

		
		e += "pyftsubset \"";
		e += s.first;
		e += "\"";
		e += " --output-file=\"";
		e += subsetFileName;
		e += "\"";
		e += " --unicodes-file=\"";
		e += glyphsFileName;
		e += "\"";
		e += " --layout-features='*' --glyph-names --symbol-cmap --legacy-cmap --notdef-glyph --notdef-outline --recommended-glyphs ";
		e += " --name-IDs='*' --name-legacy ";
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

		for (auto type : types)
		{
			printf("%s\n", type.c_str());
			for (auto emSize : emSizes)
			{
				int count = 0;
				printf("%i\n", emSize);
				std::string merge = "pyftmerge ";
				std::string lastFileName = "";
				for (auto & s : glyphs)
				{
					if (s.second.length() <= 1)
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

					std::string subsetFileName = "subset_";
					subsetFileName += this->BaseName(s.first);

					lastFileName = subsetFileName;

					merge += subsetFileName;
					merge += " ";
					count++;
				}

				printf("%i\n", count);

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
				if (emSizes.size() > 1)
				{
					e += this->outputTTF;
					e += "_";
					e += std::to_string(emSize);
					e += ".";
					e += type;
				}
				else
				{
					e += this->outputTTF;
					e += ".";
					e += type;
				}

				e += '\n';
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

	//e += remove;

	std::ofstream file = std::ofstream(outputDir + scriptFileName, std::ios_base::binary | std::ios_base::out);
	
	file << e;

	//pyftsubset arial_unicode.ttf --unicodes-file = "chars.txt" 
	//--layout-features = '*' 
	//--glyph-names --symbol-cmap --legacy-cmap --notdef-glyph --notdef-outline --recommended-glyphs 
	//--name-IDs = '*' --name-legacy --output-file = "ii/ll.ttf"

};

#endif
