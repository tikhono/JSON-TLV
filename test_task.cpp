#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>

#include "./json/single_include/nlohmann/json.hpp"
#include "./tlv_box.h"

#define INT 0b00
#define BUL 0b01
#define STR 0b10
#define FLT 0b11

int		test()
{
	std::cout << std::endl  << "Testing read from output.bin and dictionary.bin" << std::endl;
	std::cout << std::endl  << "Test hardcoded for check read of last key-value pair" << std::endl;
	std::cout << std::endl  << "Make shure it is int-string" << std::endl;

	std::streampos	size;
	char		*memblock;

	std::ifstream output("output.bin", std::ios::in | std::ios::binary | std::ios::ate);
	if (output.is_open())
	{
		size = output.tellg();
		memblock = new char [size];
		output.seekg (0, std::ios::beg);
		output.read (memblock, size);
		output.close();

		tlv::TlvBox	parsedBoxes;
		if (!parsedBoxes.Parse((const unsigned char *)memblock, size)) {
			std::cout << "output parse Failed !\n";
			return -1;
		}
		std::cout << "output parse Success, " << size/sizeof(char) << " bytes \n";

		int	val1;
		std::string s1;
		if (!parsedBoxes.GetIntValue(INT, val1)) {
			std::cout << "GetIntValue Failed !\n";
			return -1;
		}
		if (!parsedBoxes.GetStringValue(STR, s1)) {
			std::cout << "GetIntValue Failed !\n";
			return -1;
		}
		std::cout << "Last record in output: for new key \"" << val1 << "\" original value is \"" << s1 << "\"" << std::endl;
		
		delete[] memblock;
	}
	else
	{
		std::cout << "Unable to open file";
		return -1;
	}
	std::ifstream dict("dictionary.bin", std::ios::in | std::ios::binary | std::ios::ate);
	if (dict.is_open())
	{
		size = dict.tellg();
		memblock = new char [size];
		dict.seekg (0, std::ios::beg);
		dict.read (memblock, size);
		dict.close();

		tlv::TlvBox	parsedBoxes;
		if (!parsedBoxes.Parse((const unsigned char *)memblock, size)) {
			std::cout << "dict parse Failed !\n";
			return -1;
		}
		std::cout << "dict parse Success, " << size/sizeof(char) << " bytes \n";

		int	val2;
		std::string s2;
		if (!parsedBoxes.GetIntValue(INT, val2)) {
			std::cout << "GetIntValue Failed !\n";
			return -2;
		}
		if (!parsedBoxes.GetStringValue(STR, s2)) {
			std::cout << "GetIntValue Failed !\n";
			return -2;
		}
		std::cout << "Last record in dictionary: new key is \"" << val2 << "\" for original key \"" << s2 << "\"" << std::endl;
		
		delete[] memblock;
	}
	else
	{
		std::cout << "Unable to open file";
		return -1;
	}
		return 0;

}

unsigned int	get_key_id(std::unordered_map<std::string, unsigned int> &dict, auto &j_item)
{
	std::string							key = j_item.key();
	std::unordered_map<std::string, unsigned int>::const_iterator	got = dict.find(key);
	if (got != dict.end())
	{
		return got->second;
	}
	else
	{
		unsigned int new_id = dict.size() + 1;
		dict.insert({key, new_id});
		tlv::TlvBox	box;
		box.PutIntValue(INT, new_id);
		box.PutStringValue(STR, key);
		if (!box.Serialize())
			std::cout << "box Serialize Failed !\n";
		std::ofstream	dictionary;
		dictionary.open("dictionary.bin", std::ios::out | std::ios::app | std::ios::binary);
		unsigned char *bytes = box.GetSerializedBuffer();
		for (int i = 0; i < box.GetSerializedBytes() / sizeof(unsigned char); i++)
			dictionary << bytes[i];
		dictionary.close();
		return new_id;
	}
}

int main ()
{
	std::unordered_map<std::string, unsigned int>	dict;
	std::ifstream					input("../input.json");
	std::string					line;

	while(std::getline(input, line))
	{
		std::cout << line << std::endl;
		nlohmann::json j_line = nlohmann::json::parse(line);
		for (auto& it : j_line.items())
		{
			std::cout << "key: " << it.key() << "\tval: " << it.value();
			unsigned int new_key = get_key_id(dict, it);
			std::cout << "\t\tnew key is " << new_key << std::endl;
			
			tlv::TlvBox	box;
			box.PutIntValue(INT, new_key);
			nlohmann::json	j_value = it.value();

			if (j_value.is_number_integer())
			{
				int	int_val = j_value.get<int>();
				box.PutIntValue(INT, int_val);
			}
			else if (j_value.is_number_float())
			{
				float	float_val = j_value.get<float>();
				box.PutFloatValue(FLT, float_val);
			}
			else if (j_value.is_string())
			{
				std::string	str_val = j_value.get<std::string>();
				box.PutStringValue(STR, str_val);
			}
			else if (j_value.is_boolean())
			{
				bool	bool_val = j_value.get<bool>();
				box.PutBoolValue(BUL, bool_val);
			}
			
			if (!box.Serialize())
				std::cout << "box Serialize Failed !\n";
			std::ofstream	output;
			output.open("output.bin", std::ios::out | std::ios::app | std::ios::binary);
			unsigned char *bytes = box.GetSerializedBuffer();
			for (int i = 0; i < box.GetSerializedBytes() / sizeof(unsigned char); i++)
				output << bytes[i];
			output.close();
			}
			std::cout << std::endl;
		}

	return test();;
}
