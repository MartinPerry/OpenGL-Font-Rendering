#ifndef CHARACTER_EXTRACTOR_H
#define CHARACTER_EXTRACTOR_H

#include <set>
#include <cstdint>
#include <vector>
#include <unordered_map>

#include <string>
#include <functional>

#include <ft2build.h>
#include FT_FREETYPE_H


#include "../Externalncludes.h"


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

	void Release();

	std::vector<int32_t> GetAllCharacters() const;

	void SetOutputDir(const std::string & outputDir);

	void AddAllAsciiLetters();
	void AddCharacter(uint32_t c);
	void AddText(const UnicodeString & strU);
	void AddTextFromFile(const std::string & filePath);
	void AddTextFromFile(const std::string & filePath, std::function<void(const char* str, CharacterExtractor * ce)> parseCallback);
	void AddDirectory(const std::string & dirPath);

	void RemoveChar(int32_t c);

	void GenerateScript(const std::string & scriptFileName = "run.sh");

protected:

	typedef std::string FaceName;

	struct GlyphsInfo 
	{
		std::unordered_map<FaceName, std::string> glyphsCodes;
		std::unordered_map<FaceName, UnicodeString> glyphsUnicode;		
	};

	std::string outputDir;
	std::vector<std::string> inputTTF;
	std::string outputTTF;
	std::set<int32_t> characters;

	std::unordered_map<FaceName, FT_Face> faces;
	std::unordered_map<FaceName, int> facesLineOffset;
	
	FT_Library library;
	

	void InitFreeType();
	std::string BaseName(std::string const & path);
	bool HasEnding(std::string const &fullString, std::string const &ending);

	GlyphsInfo BuildGlyphs();
	void AddSpaceToAllGlyphs(GlyphsInfo& gi);

	int GetFontsCount(const GlyphsInfo& gi);
};

#endif
