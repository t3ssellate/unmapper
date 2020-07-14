#include "pefile.h"

PeFile::PeFile(std::string _filename)
{
	HANDLE hfile = ::CreateFileA(_filename.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hfile == INVALID_HANDLE_VALUE)
	{
		std::string msg = "Could not open file " + _filename;
		throw std::runtime_error(msg);
	}
	std::size_t last_slash_idx = _filename.find_last_of("\\");;
	std::string name;
	if (last_slash_idx != std::string::npos)
	{
		name = _filename.substr(last_slash_idx);
	}
	else
	{
		name = _filename;
	}
	m_size = ::GetFileSize(hfile, NULL);
	if (m_size == INVALID_FILE_SIZE)
	{
		::CloseHandle(hfile);
		throw std::runtime_error("Could not get size of file, is it over 4 GiB?");
	}
	m_mapping = ::CreateFileMappingA(hfile, NULL, PAGE_READONLY, 0, 0, name.c_str());
	::CloseHandle(hfile);
	if (!m_mapping)
	{
		throw std::runtime_error("Could not map file to memory");
	}
	// CoW mapping view, so that we don't have to manually copy the memory to modify it
	void* view = ::MapViewOfFile(m_mapping, FILE_MAP_COPY, 0, 0, 0);
	if (!view)
	{
		throw std::runtime_error("Could not create file mapping view");
	}
	m_view = static_cast<unsigned char*>(view);
	if (!is_valid_file())
	{
		throw std::runtime_error("Invalid PE header");
	}
}

PeFile::PeFile(PeFile&& _other) noexcept
{
	m_mapping = _other.m_mapping;
	m_size = _other.m_size;
	m_view = _other.m_view;
	_other.m_size = 0;
	_other.m_mapping = nullptr;
	_other.m_view = nullptr;
}

PeFile::~PeFile()
{
	if (m_view != nullptr)
	{
		::UnmapViewOfFile(m_view);
	}
	if (m_mapping != nullptr)
	{
		::CloseHandle(m_mapping);
	}
}

bool PeFile::is_valid_file() const noexcept
{
	// File should be at least contain these headers
	std::size_t headers_size = sizeof(IMAGE_DOS_HEADER) + sizeof(IMAGE_FILE_HEADER) + sizeof(IMAGE_OPTIONAL_HEADER);
	if (m_size < headers_size)
	{
		return false;
	}

	// Section header count validation
	unsigned section_count = pe_header().NumberOfSections;
	if (m_size < headers_size + section_count * sizeof(IMAGE_SECTION_HEADER))
	{
		return false;
	}

	// Check MZ + PE headers
	auto dos_header = this->dos_header();
	if (dos_header.e_magic != IMAGE_DOS_SIGNATURE || *(DWORD*)(m_view + dos_header.e_lfanew) != IMAGE_NT_SIGNATURE)
	{
		return false;
	}

	return true;
}

IMAGE_FILE_HEADER& PeFile::pe_header() const noexcept
{
	auto dos_header = this->dos_header();
	auto pe_header = reinterpret_cast<PIMAGE_FILE_HEADER>(m_view + dos_header.e_lfanew + NT_SIGNATURE_SIZE);
	return *pe_header;
}

IMAGE_DOS_HEADER& PeFile::dos_header() const noexcept
{
	PIMAGE_DOS_HEADER dos_header = reinterpret_cast<PIMAGE_DOS_HEADER>(m_view);
	return *dos_header;
}

IMAGE_OPTIONAL_HEADER& PeFile::opt_header() const noexcept
{
	auto opt_header = reinterpret_cast<PIMAGE_OPTIONAL_HEADER>(m_view
		+ dos_header().e_lfanew + NT_SIGNATURE_SIZE + sizeof(IMAGE_FILE_HEADER));
	return *opt_header;
}

DWORD PeFile::size() const noexcept
{
	return m_size;
}

std::vector<PIMAGE_SECTION_HEADER> PeFile::section_headers() const
{
	unsigned section_count = pe_header().NumberOfSections;
	std::vector<PIMAGE_SECTION_HEADER> res;
	res.reserve(section_count);

	// Disgusting, I know.
	auto section_headers_base = reinterpret_cast<PIMAGE_SECTION_HEADER>(m_view
			+ dos_header().e_lfanew + NT_SIGNATURE_SIZE + sizeof(IMAGE_FILE_HEADER)
			+ sizeof(IMAGE_OPTIONAL_HEADER));
	for (unsigned i = 0; i < section_count; i++)
	{
		auto header = section_headers_base + i;
		res.push_back(header);
	}
	return res;
}

void PeFile::write_to_file(const std::string& _filename) const
{
	HANDLE hfile = ::CreateFileA(_filename.c_str(), GENERIC_WRITE, NULL, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hfile == INVALID_HANDLE_VALUE)
	{
		std::string error = "Could not create file " + _filename + ", does it already exist?";
		throw std::runtime_error(error);
	}
	DWORD bytes_written{ 0 };
	//LARGE_INTEGER 
	BOOL res = WriteFile(hfile, m_view, m_size, &bytes_written, NULL);
	::CloseHandle(hfile);
	if (bytes_written != m_size)
	{
		std::string error = "Could not write to file " + _filename;
		throw std::runtime_error(error);
	}
}

PeFile& PeFile::operator=(PeFile&& _other) noexcept
{
	m_mapping = _other.m_mapping;
	m_size = _other.m_size;
	m_view = _other.m_view;
	_other.m_size = 0;
	_other.m_mapping = nullptr;
	_other.m_view = nullptr;
	return *this;
}