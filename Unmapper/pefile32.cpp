#include "pefile32.h"

PeFile32::PeFile32(const std::string& _filename)
{
	init_mapping_view(_filename);
	if (!is_file_valid())
	{
		throw std::runtime_error("Invalid PE header");
	}
	auto pe_header = this->pe_header();
	if ((pe_header.Machine & IMAGE_FILE_32BIT_MACHINE) != IMAGE_FILE_32BIT_MACHINE)
	{
		throw bitness_error();
	}
}

std::string PeFile32::bitness() const
{
	return "32";
}

IMAGE_OPTIONAL_HEADER32& PeFile32::opt_header() const
{
	auto opt_header_ptr = reinterpret_cast<PIMAGE_OPTIONAL_HEADER32>(m_view
		+ dos_header().e_lfanew + NT_SIGNATURE_SIZE + sizeof(IMAGE_FILE_HEADER));
	return *opt_header_ptr;
}

std::size_t PeFile32::headers_size() const noexcept
{
	return sizeof(IMAGE_DOS_HEADER) + NT_SIGNATURE_SIZE + sizeof(IMAGE_FILE_HEADER) + sizeof(IMAGE_OPTIONAL_HEADER32);
}