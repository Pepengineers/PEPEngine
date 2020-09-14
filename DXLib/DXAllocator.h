#pragma once

#include <d3d12.h>
#include "STLCustomAllocator.h"

class GAllocator;
class GMemory;

class DXAllocator
{
	static LinearAllocationStrategy<> allocatorStrategy;
	
	static custom_vector<std::unique_ptr<GAllocator>> graphicAllocator;

	
public:

	static void ResetAllocator();

	static GMemory AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t descriptorCount = 1 );
	
	template<typename T>
	CustomAllocator<T> static GetAllocator()
	{
		CustomAllocator <T> allocator(allocatorStrategy );

		return allocator;
	}

	template<typename T>
	custom_vector<T> static CreateVector()
	{
		return custom_vector<T>(GetAllocator<T>());
	}

	template<typename T>
	custom_list<T> static CreateList()
	{
		return custom_list<T>{GetAllocator<T>()};
	}

	template<typename T>
	custom_deque<T> static CreateDeque()
	{
		return custom_deque<T>{GetAllocator<T>()};
	}

	template<typename T>
	custom_queue<T> static CreateQueue()
	{
		return custom_queue<T>{GetAllocator<T>()};
	}
		
	template<typename K, typename V>
	custom_map<K, V> static CreateMap()
	{
		return custom_map<K, V>(GetAllocator<std::pair<K, V>>());
	}
	
	template<typename K, typename V>
	custom_multimap<K, V> static CreateMultimap()
	{
		return custom_multimap<K, V>(GetAllocator<std::pair<K, V>>());
	}

	template<typename K, typename V>
	custom_unordered_map<K, V> static CreateUnorderedMap()
	{
		return custom_unordered_map<K, V>(GetAllocator<std::pair<K, V>>());
	}

	template<typename T>
	custom_set<T> static CreateSet()
	{
		return custom_set<T>(GetAllocator<T>());
	}

	template<typename T>
	custom_unordered_set<T> static CreateUnorderedSet()
	{
		return custom_unordered_set<T>(GetAllocator<T>());
	}

	custom_string static CreateString()
	{
		return custom_string( GetAllocator<char>() );
	}

	custom_wstring static CreateWString()
	{
		return custom_wstring( GetAllocator<wchar_t>() );
	}
		
	template<typename T, typename... Args>
	custom_unique_ptr<T> make_custom_unique(Args&&... args)
	{
		return make_custom_unique<T>(GetAllocator<T>(), args);
	}
};

