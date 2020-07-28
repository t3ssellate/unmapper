#pragma once
#include "pefile.h"

class PeFile64 : public PeFile
{
public:
	PeFile64(const std::string& _filename);

	// This type cannot be copied, to avoid double-freeing HANDLEs use move operator/ctor
	PeFile64() = default;
	PeFile64(PeFile64&& _other) noexcept = default;
	PeFile64(const PeFile64& _other) noexcept = delete;

	virtual std::string bitness() const final;
	IMAGE_OPTIONAL_HEADER64& opt_header() const;

	PeFile64& operator=(const PeFile64& _other) = delete;
	PeFile64& operator=(PeFile64&& _other) noexcept = default;

private:
	virtual std::size_t headers_size() const noexcept final;
};