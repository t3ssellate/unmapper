#include "pefile32.h"
#include "pefile64.h"

#include <iostream>

void fix_alignment(PeFile* _pe)
{
	if (auto pe32 = dynamic_cast<PeFile32*>(_pe))
	{
		auto& header = pe32->opt_header();
		std::cout << "[+] Modifying file alignment from 0x" << header.FileAlignment
			<< " to 0x" << header.SectionAlignment << std::endl;
		header.FileAlignment = header.SectionAlignment;
	}
	else if (auto pe64 = dynamic_cast<PeFile64*>(_pe))
	{
		auto& header = pe64->opt_header();
		std::cout << "[+] Modifying file alignment from 0x" << header.FileAlignment
			<< " to 0x" << header.SectionAlignment << std::endl;
		header.FileAlignment = header.SectionAlignment;
	}
}

void fix_image_base(PeFile* _pe, unsigned long long _new_base)
{
	if (auto pe32 = dynamic_cast<PeFile32*>(_pe))
	{
		auto& header = pe32->opt_header();
		std::cout << "[+] Modifying base address from 0x" << std::hex
			<< header.ImageBase << " to 0x" << static_cast<DWORD>(_new_base) << std::endl;
		header.ImageBase = static_cast<DWORD>(_new_base);
	}
	else if (auto pe64 = dynamic_cast<PeFile64*>(_pe))
	{
		auto& header = pe64->opt_header();
		std::cout << "[+] Modifying base address from 0x" << std::hex
			<< header.ImageBase << " to 0x" << _new_base << std::endl;
		header.ImageBase = _new_base;
	}
}

void fix_sections(PeFile* _pe)
{
	auto headers = _pe->section_headers();
	for (const auto header : headers)
	{
		std::cout << "[+] Section " << header->Name << std::endl;
		std::cout << "\t Modifying section file offset from 0x" << header->PointerToRawData << " to 0x" << header->VirtualAddress << std::endl;
		header->PointerToRawData = header->VirtualAddress;

		DWORD new_size = header->SizeOfRawData;
		DWORD alignment = dynamic_cast<PeFile32*>(_pe) ? dynamic_cast<PeFile32*>(_pe)->opt_header().SectionAlignment
			: dynamic_cast<PeFile64*>(_pe)->opt_header().SectionAlignment;
		if (new_size % alignment != 0)
		{
			// Not page aligned? Round up to next page-aligned size
			new_size = alignment * (1 + new_size / alignment);
		}
		std::cout << "\t Modifying section raw size from 0x" << header->SizeOfRawData << " to 0x" << new_size << std::endl;
		header->SizeOfRawData = new_size;
	}
}

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		std::cout << "Usage: Unmapper.exe <dump path>" << std::endl;
		return 0;
	}

	const std::string filename{ argv[1] };
	std::cin.exceptions(std::ios_base::failbit);
	std::unique_ptr<PeFile> pe;
	unsigned long long new_base{ 0 };

	// Open file with correct bitness, create mapping + view...
	try
	{
		pe = std::make_unique<PeFile64>(filename);
	}
	catch (const bitness_error& ex)
	{
		pe = std::make_unique<PeFile32>(filename);
	}
	catch (const std::runtime_error& ex)
	{
		std::cout << "[!] " << ex.what() << std::endl;
		return 1;
	}
	std::cout << "[+] Created mapping for " << pe->bitness() << " bit file " << filename
		<< ", size: " << pe->size() << std::endl;

	std::cout << "[?] Enter the address (in hexadecimal) at which the PE was loaded:\n[>] ";
	try
	{
		std::cin >> std::hex >> new_base;
	}
	catch (const std::ios_base::failure& ex)
	{
		std::cout << "[!] Invalid number format" << std::endl;
		return 1;
	}

	// Do the actual fixing
	fix_image_base(pe.get(), new_base);
	fix_alignment(pe.get());
	fix_sections(pe.get());

	// Write to file <filename>_unmapped.<extension>
	std::string unmapped_filename;
	std::size_t idx = filename.find_first_of('.');
	if (idx != std::string::npos)
	{
		unmapped_filename = filename.substr(0, idx) + "_unmapped" + filename.substr(idx);
	}
	else
	{
		unmapped_filename = filename + "_unmapped";
	}

	std::cout << "[+] Writing to file " << unmapped_filename << std::endl;
	try
	{
		pe->write_to_file(unmapped_filename);
	}
	catch (const std::runtime_error& ex)
	{
		std::cout << "[!] " << ex.what() << std::endl;
		return 1;
	}
	std::cout << "[+] Done!" << std::endl;
	return 0;
}