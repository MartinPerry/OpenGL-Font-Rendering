#ifndef _DEBUG_UTILS_H_
#define _DEBUG_UTILS_H_

#include "../Externalncludes.h"

#include <chrono>
#include <random>


struct RandomString
{
	std::default_random_engine rng;
	std::uniform_int_distribution<> dist;

	std::vector<int32_t> allChars;
	size_t length;
};

int32_t UniqueChar(RandomString & rs)
{
	int32_t i = rs.dist(rs.rng);
	if (rs.allChars.size() == 0)
	{
		return i;
	}
	return rs.allChars[i];	
}

std::vector<int32_t> generateRandomCharList(RandomString & rs)
{		
	std::vector<int32_t> str;
	for (int i = 0; i < rs.length; i++)
	{
		str.push_back(UniqueChar(rs));
	}
	//std::generate_n(str.begin(), length - 1, UniqueChar);

	return str;
}

UnicodeString generateRandoString(RandomString & rs)
{
	auto tmp = generateRandomCharList(rs);
	UnicodeString str = tmp[0];
	for (int j = 1; j < tmp.size(); j++)
	{
		str += tmp[j];
	}

	return str;
}

RandomString initRandomGenerator(const std::vector<int32_t> & allChars, int max)
{
	RandomString r;

	r.allChars = allChars;

	//1) create a non-deterministic random number generator      
	r.rng = std::default_random_engine(std::random_device{}());

	//2) create a random number "shaper" that will give
	//   us uniformly distributed indices into the character set
	r.dist = std::uniform_int_distribution<>(0, max);	

	r.length = 10;

	return r;
}

RandomString initRandomGenerator(const std::vector<int32_t> & allChars)
{
	return initRandomGenerator(allChars, allChars.size() - 1);
}

RandomString initRandomGenerator(int max)
{
	return initRandomGenerator({}, max);
}

#endif
