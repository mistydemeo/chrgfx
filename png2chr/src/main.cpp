
#include "chrgfx.hpp"
#include "import_defs.hpp"
#include "shared.hpp"

#include <cerrno>
#include <getopt.h>
#include <iostream>
#include <stdio.h>

#ifdef DEBUG
#include <chrono>
#endif

using std::string;
using std::vector;
using namespace chrgfx;

void process_args(int argc, char **argv);
void print_help();

// application globals
static unsigned int const APP_VERSION_MAJOR{1};
static unsigned int const APP_VERSION_MINOR{0};
static unsigned int const APP_VERSION_FIX{0};
static std::string const APP_VERSION{std::to_string(APP_VERSION_MAJOR) + "." +
																		 std::to_string(APP_VERSION_MINOR) + "." +
																		 std::to_string(APP_VERSION_FIX)};
static std::string const APP_NAME{"png2chr"};

struct runtime_config_png2chr : runtime_config {
	string pngdata_name{""};
	string chr_outfile{""};
	string pal_outfile{""};
};

// option settings
runtime_config_png2chr cfg;

int main(int argc, char **argv)
{
	try {
		try {
			// Runtime State Setup
			process_args(argc, argv);
		} catch(std::exception const &e) {
			std::cerr << e.what() << std::endl;
			return -1;
		}

		// see if we even have good input before moving on
		std::ifstream pngdata{cfg.pngdata_name};
		if(!pngdata.good()) {
			throw std::ios_base::failure(std::strerror(errno));
		}

		// converter function pointers
		conv_chr::chrconv_to_t chr_to_converter;
		conv_color::colconv_to_t col_to_converter;
		conv_palette::palconv_to_t pal_to_converter;

		// load definitions
		auto defs = load_gfxdefs(cfg.gfxdef);

		map<string const, chrdef const> chrdefs{std::get<0>(defs)};
		map<string const, coldef const> coldefs{std::get<1>(defs)};
		map<string const, paldef const> paldefs{std::get<2>(defs)};
		map<string const, gfxprofile const> profiles{std::get<3>(defs)};

		if(cfg.chr_outfile == "" && cfg.pal_outfile == "") {
			throw std::invalid_argument("No tile or palette output; nothing to do!");
		}

		auto profile_iter{profiles.find(cfg.profile)};
		if(profile_iter == profiles.end()) {
			throw "Could not find specified gfxprofile";
		}

		gfxprofile profile{profile_iter->second};

		auto chrdef_iter{chrdefs.find(cfg.chrdef.empty() ? profile.get_chrdef_id()
																										 : cfg.chrdef)};
		if(chrdef_iter == chrdefs.end()) {
			throw "Could not find specified chrdef";
		}

		chrdef chrdef{chrdef_iter->second};

		auto coldef_iter{coldefs.find(cfg.coldef.empty() ? profile.get_coldef_id()
																										 : cfg.coldef)};
		if(coldef_iter == coldefs.end()) {
			throw "Could not find specified coldef";
		}

		coldef coldef{coldef_iter->second};

		auto paldef_iter{paldefs.find(cfg.paldef.empty() ? profile.get_paldef_id()
																										 : cfg.paldef)};
		if(paldef_iter == paldefs.end()) {
			throw "Could not find specified paldef";
		}

		paldef paldef{paldef_iter->second};

		try {
			png::image<png::index_pixel> in_img(
					cfg.pngdata_name, png::require_color_space<png::index_pixel>());

			if(cfg.chr_outfile != "") {
#ifdef DEBUG
				std::chrono::high_resolution_clock::time_point t1 =
						std::chrono::high_resolution_clock::now();
#endif

				// TODO - temporary, need to load specified conveter
				chr_to_converter = conv_chr::converters_to.at("default");
				pal_to_converter = conv_palette::converters_to.at("default");
				col_to_converter = conv_color::converters_to.at("default");

				// deal with tiles first
				chrbank png_chrbank{chrgfx::png_chunk(chrdef, in_img.get_pixbuf())};

#ifdef DEBUG
				std::chrono::high_resolution_clock::time_point t2 =
						std::chrono::high_resolution_clock::now();
				auto duration =
						std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1)
								.count();

				std::cerr << "PNG chunk/tile conversion: " << duration << "ms"
									<< std::endl;

#endif

				std::ofstream chr_outfile{cfg.chr_outfile};
				if(!chr_outfile.good()) {
					throw std::ios_base::failure(std::strerror(errno));
				}

				size_t chunksize{chrdef.get_datasize() / 8};

				for(const auto &chr : png_chrbank) {
					u8 *temp_chr{chr_to_converter(chrdef, chr.get())};
					std::copy(temp_chr, temp_chr + chunksize,
										std::ostream_iterator<u8>(chr_outfile));
				}
			}

			// deal with the palette next
			if(cfg.pal_outfile != "") {
				u8 *t = pal_to_converter(paldef, coldef, in_img.get_palette(),
																 cfg.subpalette, col_to_converter);
				std::ofstream pal_outfile{cfg.pal_outfile};
				if(!pal_outfile.good()) {
					throw std::ios_base::failure(std::strerror(errno));
				}
				size_t filesize{
						(cfg.subpalette
								 ? paldef.get_subpal_datasize_bytes()
								 : (paldef.get_palette_length() > 256
												? (size_t)(256 * (paldef.get_entry_datasize() / 8))
												: paldef.get_palette_datasize_bytes()))};

				pal_outfile.write((char *)t, filesize);
			}

		} catch(const png::error &e) {
			std::cerr << "PNG error: " << e.what() << std::endl;
			return -10;
		}

		return 0;
	} catch(std::exception const &e) {
		std::cerr << "FATAL EXCEPTION: " << e.what() << std::endl;
		return -1;
	}
}

void process_args(int argc, char **argv)
{
	default_long_opts.push_back({"chr-output", required_argument, nullptr, 'c'});
	default_long_opts.push_back({"pal-output", required_argument, nullptr, 'p'});
	default_long_opts.push_back({"png-data", required_argument, nullptr, 'b'});
	default_short_opts.append("c:p:b:");

	bool default_processed = process_default_args(cfg, argc, argv);

	if(!default_processed) {
		print_help();
		exit(1);
	}

	while(true) {
		const auto this_opt = getopt_long(argc, argv, default_short_opts.data(),
																			default_long_opts.data(), nullptr);
		if(this_opt == -1)
			break;

		switch(this_opt) {
			// chr-output
			case 'c':
				cfg.chr_outfile = optarg;
				break;

			// pal-output
			case 'p':
				cfg.pal_outfile = optarg;
				break;

			// png-data
			case 'b':
				cfg.pngdata_name = optarg;
				break;
		}
	}
}

void print_help()
{
	std::cerr << APP_NAME << " - ver " << APP_VERSION << std::endl << std::endl;
	std::cerr << "Valid options:" << std::endl;
	std::cerr << "  --gfx-def, -G   Specify graphics data format" << std::endl;
	std::cerr
			<< "  --chr-def, -C   Specify tile data format (overrides tile format "
				 "in gfx-def)"
			<< std::endl;
	std::cerr << "  --chr-data, -c     Filename to input tile data" << std::endl;
	std::cerr
			<< "  --pal-def, -P   Specify palette data format (overrides palette "
				 "format in gfx-def)"
			<< std::endl;
	std::cerr << "  --pal-data, -p     Filename to input palette data"
						<< std::endl;
	std::cerr << "  --output, -o       Specify output PNG image filename"
						<< std::endl;
	std::cerr << "  --trns, -t         Use image transparency" << std::endl;
	std::cerr
			<< "  --trns-index, -i   Specify palette entry to use as transparency "
				 "(default is 0)"
			<< std::endl;
	std::cerr
			<< "  --columns, -c      Specify number of columns per row of tiles in "
				 "output image"
			<< std::endl;
	std::cerr << "  --subpalette, -s   Specify subpalette (default is 0)"
						<< std::endl;
	std::cerr << "  --help, -h         Display this text" << std::endl;
}
