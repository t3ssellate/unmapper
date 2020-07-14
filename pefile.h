#pragma once
#include <fstream>
#include <string>
#include <vector>
#include <windows.h>
#include <winnt.h>
#include <functional>

constexpr std::size_t NT_SIGNATURE_SIZE = 4;

class PeFile
{
	HANDLE m_mapping{ nullptr };
	DWORD m_size{ 0 };
	unsigned char* m_view{ nullptr };
	bool is_valid_file() const noexcept;
public:
	PeFile(std::string _filename);

	// This type cannot be copied, to avoid double-freeing HANDLEs
	PeFile() = default;
	PeFile(PeFile&& _other) noexcept;
	PeFile(const PeFile& _other) = delete;
	~PeFile();

	IMAGE_FILE_HEADER& pe_header() const noexcept;
	IMAGE_DOS_HEADER& dos_header() const noexcept;
	IMAGE_OPTIONAL_HEADER& opt_header() const noexcept;
	DWORD size() const noexcept;
	std::vector<PIMAGE_SECTION_HEADER> section_headers() const;
	void write_to_file(const std::string& _filename) const;

	PeFile& operator=(const PeFile& _other) = delete;
	PeFile& operator=(PeFile&& _other) noexcept;
};