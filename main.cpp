#include <iostream>
#include <fstream>
#include "pefile.h"

/*
Next steps:
- Write to disk from CoW mapping [x]
- Modify ImageBase to actual address [x]
- Parse the sections + their addresses [x]
- Modify sizes to page-alignment and file-aligment
*/

void fix_alignment(PeFile& _pe)
{
	auto& header = _pe.opt_header();
	std::cout << "[+] Modifying file alignment from 0x" << header.FileAlignment
		<< " to 0x" << header.SectionAlignment << std::endl;
	header.FileAlignment = header.SectionAlignment;
}

void fix_image_base(PeFile& _pe, unsigned long long _new_base)
{
	auto& header = _pe.opt_header();
	std::cout << "[+] Modifying base address from 0x" << std::hex
		<< header.ImageBase << " to 0x" << _new_base << std::endl;
	header.ImageBase = _new_base;
}

void fix_sections(PeFile& _pe)
{
	auto headers = _pe.section_headers();
	for (const auto header : headers)
	{
		std::cout << "[+] Section " << header->Name << std::endl;
		std::cout << "\t Modifying section file offset from 0x" << header->PointerToRawData << " to 0x" << header->VirtualAddress << std::endl;
		header->PointerToRawData = header->VirtualAddress;

		DWORD new_size = header->SizeOfRawData;
		DWORD alignment = _pe.opt_header().SectionAlignment;
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
		std::cout << "Usage: unmapper.exe <dump path>";
		return 0;
	}

	const std::string filename{ argv[1] };
	std::cin.exceptions(std::ios_base::failbit);
	PeFile pe;
	unsigned long long new_base{ 0 };
	try
	{
		pe = std::move(PeFile{ filename });
		std::cout << "[+] Created mapping for file " << filename
			<< ", size: " << pe.size() << std::endl;
		std::cout << "[?] Enter the address (in hexadecimal) at which the PE was loaded:\n[>] ";
		std::cin >> std::hex >> new_base;
	}
	catch (const std::ios_base::failure& ex)
	{
		std::cout << "[!] Invalid number format" << std::endl;
		return 1;
	}
	catch (const std::runtime_error& ex)
	{
		std::cout << "[!] " << ex.what() << std::endl;
		return 1;
	}

	fix_image_base(pe, new_base);
	fix_alignment(pe);
	fix_sections(pe);
	
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

	try 
	{
		std::cout << "[+] Writing to file " << unmapped_filename << std::endl;
		pe.write_to_file(unmapped_filename);
	}
	catch (const std::runtime_error& ex)
	{
		std::cout << "[!] " << ex.what() << std::endl;
		return 1;
	}
	std::cout << "[+] Done!" << std::endl;
	return 0;
}