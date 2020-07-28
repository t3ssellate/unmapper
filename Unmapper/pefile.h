#pragma once
#include <windows.h>
#include <winnt.h>

#include <stdexcept>
#include <string>
#include <vector>

constexpr std::size_t NT_SIGNATURE_SIZE = 4;

class bitness_error : public std::runtime_error
{
public:
	bitness_error() : std::runtime_error("Incompatible bitness") { }
};

class PeFile
{
public:
	virtual ~PeFile();

	IMAGE_DOS_HEADER& dos_header() const;
	IMAGE_FILE_HEADER& pe_header() const;
	std::vector<PIMAGE_SECTION_HEADER> section_headers() const;

	DWORD size() const noexcept;
	virtual std::string bitness() const = 0;
	void write_to_file(const std::string& _filename) const;

protected:
	HANDLE m_mapping{ nullptr };
	unsigned char* m_view{ nullptr };
	DWORD m_size{ 0 };

	void init_mapping_view(const std::string& _filename);
	virtual std::size_t headers_size() const noexcept = 0;
	virtual bool is_file_valid() const noexcept;
};

