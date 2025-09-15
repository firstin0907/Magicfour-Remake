#pragma once
#include <unordered_map>
#include <string>
#include <fstream>

#include "core/GameException.hh"
#include "../third-party/rapidxml-1.13/rapidxml.hpp"

struct xml_node_wrapper
{
	rapidxml::xml_node<>* node;
	xml_node_wrapper(rapidxml::xml_node<>* n) : node(n) {}

	std::string get_required_attr(const char* attr_name) const
	{
		auto attr = node->first_attribute(attr_name);
		if (!attr) throw GAME_EXCEPTION(L"Missing required attribute: " + std::wstring(attr_name, attr_name + strlen(attr_name)));
		return std::string(attr->value());
	}

	std::string get_attr(const char* attr_name, const char* default_value = "") const
	{
		auto attr = node->first_attribute(attr_name);
		if (!attr) return std::string(default_value);
		return std::string(attr->value());
	}
};

template <typename T>
class ResourceMap
{
public:
	std::unordered_map<std::string, std::unique_ptr<T> > resources;

	inline T* get(const char* resource_name)
	{
		auto item = resources.find(resource_name);
		if (item == resources.end())
		{
			std::wstring err_msg = L"Resource not found: ";
			err_msg += std::wstring(resource_name, resource_name + strlen(resource_name));
			throw GAME_EXCEPTION(err_msg);
		}
		else return item->second.get();
	}

	inline T* get(const std::string resource_name)
	{
		auto item = resources.find(resource_name);
		if (item == resources.end())
		{
			std::wstring err_msg = L"Resource not found: ";
			err_msg += std::wstring(resource_name.begin(), resource_name.end());
			throw GAME_EXCEPTION(err_msg);
		}
		else return item->second.get();
	}

	inline void insert(const char* resource_name, std::unique_ptr<T> resource)
	{
		auto item = resources.find(resource_name);
		resources[resource_name] = std::move(resource);
	}

	

	void loadFromXML(const char* xml_file_path, const char* resource_type, std::function<std::unique_ptr<T> (xml_node_wrapper)> loader)
	{
		std::ifstream file(xml_file_path);
		if (!file) throw GAME_EXCEPTION(L"Failed to open resource map XML file.");

		std::string xml_content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		file.close();

		static rapidxml::xml_document<> doc;
		doc.clear();
		try
		{
			doc.parse<0>(&xml_content[0]);
		}
		catch (const rapidxml::parse_error& e)
		{
			throw GAME_EXCEPTION(L"Failed to parse resource map XML file.");
		}

		auto root = doc.first_node("Resources");
		if (!root) throw GAME_EXCEPTION(L"Invalid resource map XML format: Missing <Resources> root element.");

		for (auto node = root->first_node(resource_type); node; node = node->next_sibling(resource_type))
		{
			auto name_attr = node->first_attribute("name");
			if (!name_attr) throw GAME_EXCEPTION(L"Invalid resource map XML format: Missing 'name' attribute.");

			std::string resource_name = name_attr->value();
			if (resources.find(resource_name) != resources.end())
			{
				// Resource already exists, skip loading
				continue;
			}

			std::unique_ptr<T> resource;
			try
			{
				resource = loader(node);
			}
			catch (const std::exception& e)
			{
				throw GAME_EXCEPTION(L"Failed to load resource: " + std::wstring(resource_name.begin(), resource_name.end()));
			}

			if (!resource) throw GAME_EXCEPTION(L"Loader function returned null for resource: " + std::wstring(resource_name.begin(), resource_name.end()));

			resources[resource_name] = std::move(resource);
		}
	}
	
	ResourceMap() = default;
};