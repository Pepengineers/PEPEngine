#pragma once

#include "pch.h"
#include "Allocator.h"

class CAllocator : public Allocator
{
public:
	CAllocator();

	virtual ~CAllocator();

	void* Allocate(std::size_t size, std::size_t alignment = 0) override;

	void Free(void* ptr) override;

	void Init() override;
};
