#pragma once
#include "pefile.h"

class PeFile32 : public PeFile
{
public:
	PeFile32(const std::string& _filename);

	// This type cannot be copied, to avoid double-freeing HANDLEs use move operator/ctor
	PeFile32() = default;
	PeFile32(PeFile32&& _other) noexcept = default;
	PeFile32(const PeFile32& _other) noexcept = delete;
	
	virtual std::string bitness() const final;
	IMAGE_OPTIONAL_HEADER32& opt_header() const;

	PeFile32& operator=(const PeFile32& _other) = delete;
	PeFile32& operator=(PeFile32&& _other) noexcept = default;

private:
	virtual std::size_t headers_size() const noexcept final;
};