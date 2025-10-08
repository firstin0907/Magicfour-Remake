#pragma once
#include <unordered_map>
#include <string>
#include <fstream>
#include <functional>
#include <memory>

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

	xml_node_wrapper first_node(const char* name) const
	{
		auto child = node->first_node(name);
		if (!child) return xml_node_wrapper(nullptr);
		return xml_node_wrapper(child);
	}


	xml_node_wrapper next_sibling(const char* name) const
	{
		auto sibling = node->next_sibling(name);
		if (!sibling) return xml_node_wrapper(nullptr);
		return xml_node_wrapper(sibling);
	}

	bool operator!() const { return node == nullptr; }
	operator bool() const { return node != nullptr; }
};

template <typename T>
class ResourceMap
{
public:
	// Key: resource name, Value: resource
	std::unordered_map<std::string, std::shared_ptr<T> > resources;

	// Key: resource file path, Value: resource
	std::unordered_map<std::string, std::shared_ptr<T> > path_resources;

	inline std::shared_ptr<T> get(const char* resource_name) const
	{
		auto item = resources.find(resource_name);
		if (item == resources.end())
		{
			std::wstring err_msg = L"Resource not found: ";
			err_msg += std::wstring(resource_name, resource_name + strlen(resource_name));
			throw GAME_EXCEPTION(err_msg);
		}
		else return item->second;
	}

	inline std::shared_ptr<T> get(const std::string& resource_name) const
	{
		auto item = resources.find(resource_name);
		if (item == resources.end())
		{
			std::wstring err_msg = L"Resource not found: ";
			err_msg += std::wstring(resource_name.begin(), resource_name.end());
			throw GAME_EXCEPTION(err_msg);
		}
		else return item->second;
	}

	inline std::shared_ptr<T> get_by_path(const char* resource_path) const
	{
		auto item = path_resources.find(resource_path);
		if (item == path_resources.end())
		{
			std::wstring err_msg = L"Resource not found by path: ";
			err_msg += std::wstring(resource_path, resource_path + strlen(resource_path));
			throw GAME_EXCEPTION(err_msg);
		}
		else return item->second;
	}

	inline std::shared_ptr<T> get_by_path(const std::string& resource_path) const
	{
		auto item = path_resources.find(resource_path);
		if (item == path_resources.end())
		{
			std::wstring err_msg = L"Resource not found by path: ";
			err_msg += std::wstring(resource_path.begin(), resource_path.end());
			throw GAME_EXCEPTION(err_msg);
		}
		else return item->second;
	}

	inline void insert(std::string resource_name, std::shared_ptr<T> resource)
	{
		if(resources.find(resource_name) != resources.end())
		{
			std::wstring err_msg = L"Resource already exists: ";
			err_msg += std::wstring(resource_name.begin(), resource_name.end());
			throw GAME_EXCEPTION(err_msg);
		}
		else
		{
			resources[resource_name] = resource;
		}
		
	}

	inline void insert_by_path(std::string resource_path, std::shared_ptr<T> resource)
	{
		if (path_resources.find(resource_path) != path_resources.end())
		{
			std::wstring err_msg = L"Resource already exists by path: ";
			err_msg += std::wstring(resource_path.begin(), resource_path.end());
			throw GAME_EXCEPTION(err_msg);
		}
		else
		{
			path_resources[resource_path] = resource;
		}
	}

	void findAllNodes(rapidxml::xml_node<>* parent, const char* name, std::vector<rapidxml::xml_node<>*>& out_nodes)
	{
		for (auto node = parent->first_node(); node; node = node->next_sibling())
		{
			if (strcmp(node->name(), name) == 0)
			{
				out_nodes.push_back(node);
			}
			findAllNodes(node, name, out_nodes);
		}
	}


	void loadFromXML(const char* xml_file_path, const char* resource_type, std::function<std::shared_ptr<T> (xml_node_wrapper)> loader)
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

		// Get all resources of given type although they may be children of different nodes
		std::vector<rapidxml::xml_node<>*> resource_nodes;
		findAllNodes(root, resource_type, resource_nodes);

		for (auto node : resource_nodes)
		{
			auto nodew = xml_node_wrapper(node);
			std::string resource_name = nodew.get_attr("name");
			std::string resource_path = nodew.get_attr("src");
			
			std::shared_ptr<T> resource;

			bool is_path_duplicated = !resource_path.empty() && path_resources.find(resource_path) != path_resources.end();

			// Resource with same path already loaded, reuse it
			if (is_path_duplicated)
			{
				resource = path_resources[resource_path];
			}
			else
			{
				try
				{
					resource = loader(node);
				}
				catch (const std::exception& e)
				{
					throw GAME_EXCEPTION(L"Failed to load resource: " + std::wstring(resource_name.begin(), resource_name.end()));
				}

				if (!resource)
					throw GAME_EXCEPTION(L"Loader function returned null for resource: " + std::wstring(resource_name.begin(), resource_name.end()));
			}

			// if resource_name is duplicate, exception will be thrown
			if(!resource_name.empty()) insert(resource_name, resource);

			// if resource_path is duplicate, it has been handled above
			if(!resource_path.empty() && !is_path_duplicated) insert_by_path(resource_path, resource);
		}
		
	}
	
	ResourceMap() = default;
};